#include "dtb.hpp"

namespace CPU {
namespace Boot {
namespace DTB {
namespace {

constexpr uint32_t kFdtMagic = 0xD00DFEED;
constexpr uint32_t kFdtBeginNode = 0x1;
constexpr uint32_t kFdtEndNode = 0x2;
constexpr uint32_t kFdtProp = 0x3;
constexpr uint32_t kFdtNop = 0x4;
constexpr uint32_t kFdtEnd = 0x9;
constexpr size_t kMaxNodeDepth = 16;

struct FdtHeader {
  uint32_t magic;
  uint32_t totalSize;
  uint32_t offDtStruct;
  uint32_t offDtStrings;
  uint32_t offMemRsvmap;
  uint32_t version;
  uint32_t lastCompVersion;
  uint32_t bootCpuidPhys;
  uint32_t sizeDtStrings;
  uint32_t sizeDtStruct;
};

struct ParserState {
  const uint8_t *structBlock;
  size_t structSize;
  const char *stringsBlock;
  size_t stringsSize;
};

enum class NodeKind : uint8_t {
  Other = 0,
  Cpus = 1,
  Cpu = 2,
};

uintptr_t gDeviceTreeAddress = 0;
size_t gCpuCoreCount = 0;

uint32_t readBe32(const void *ptr) {
  const uint8_t *bytes = static_cast<const uint8_t *>(ptr);
  return (static_cast<uint32_t>(bytes[0]) << 24) |
         (static_cast<uint32_t>(bytes[1]) << 16) |
         (static_cast<uint32_t>(bytes[2]) << 8) |
         static_cast<uint32_t>(bytes[3]);
}

const uint8_t *alignPtr(const uint8_t *ptr, const uint8_t *limit) {
  uintptr_t value = reinterpret_cast<uintptr_t>(ptr);
  value = (value + 3u) & ~static_cast<uintptr_t>(3u);
  const uint8_t *aligned = reinterpret_cast<const uint8_t *>(value);
  return aligned <= limit ? aligned : nullptr;
}

size_t stringLengthBounded(const char *str, const uint8_t *limit) {
  const uint8_t *cursor = reinterpret_cast<const uint8_t *>(str);
  size_t length = 0;

  while (cursor < limit) {
    if (*cursor == '\0') {
      return length;
    }
    ++cursor;
    ++length;
  }

  return static_cast<size_t>(-1);
}

bool stringsEqual(const char *lhs, const char *rhs) {
  while (*lhs != '\0' && *rhs != '\0') {
    if (*lhs != *rhs) {
      return false;
    }
    ++lhs;
    ++rhs;
  }

  return *lhs == *rhs;
}

bool loadParserState(uintptr_t deviceTreePhysAddr, ParserState &state) {
  if (deviceTreePhysAddr == 0) {
    return false;
  }

  const uint8_t *blob = reinterpret_cast<const uint8_t *>(deviceTreePhysAddr);
  const FdtHeader *header = reinterpret_cast<const FdtHeader *>(blob);

  if (readBe32(&header->magic) != kFdtMagic) {
    return false;
  }

  const size_t totalSize = readBe32(&header->totalSize);
  const size_t offDtStruct = readBe32(&header->offDtStruct);
  const size_t offDtStrings = readBe32(&header->offDtStrings);
  size_t sizeDtStruct = readBe32(&header->sizeDtStruct);
  size_t sizeDtStrings = readBe32(&header->sizeDtStrings);

  if (totalSize < sizeof(FdtHeader) || offDtStruct >= totalSize ||
      offDtStrings >= totalSize) {
    return false;
  }

  if (sizeDtStruct == 0 && offDtStrings > offDtStruct) {
    sizeDtStruct = offDtStrings - offDtStruct;
  }
  if (sizeDtStrings == 0 && totalSize > offDtStrings) {
    sizeDtStrings = totalSize - offDtStrings;
  }

  if (sizeDtStruct == 0 || sizeDtStrings == 0 ||
      offDtStruct + sizeDtStruct > totalSize ||
      offDtStrings + sizeDtStrings > totalSize) {
    return false;
  }

  state.structBlock = blob + offDtStruct;
  state.structSize = sizeDtStruct;
  state.stringsBlock = reinterpret_cast<const char *>(blob + offDtStrings);
  state.stringsSize = sizeDtStrings;
  return true;
}

bool isCpuNode(const char *name, size_t length) {
  if (length < 3) {
    return false;
  }

  return name[0] == 'c' && name[1] == 'p' && name[2] == 'u' &&
         (length == 3 || name[3] == '@');
}

size_t countCpuNodes(const ParserState &state) {
  const uint8_t *cursor = state.structBlock;
  const uint8_t *limit = state.structBlock + state.structSize;
  NodeKind nodeStack[kMaxNodeDepth] = {};
  bool countedCpuNode[kMaxNodeDepth] = {};
  bool inCpusNode = false;
  size_t depth = 0;
  size_t cpuCount = 0;

  while (cursor != nullptr && cursor + 4 <= limit) {
    const uint32_t token = readBe32(cursor);
    cursor += 4;

    switch (token) {
    case kFdtBeginNode: {
      if (depth >= kMaxNodeDepth) {
        return 0;
      }

      const char *name = reinterpret_cast<const char *>(cursor);
      const size_t nameLength = stringLengthBounded(name, limit);
      if (nameLength == static_cast<size_t>(-1)) {
        return 0;
      }

      NodeKind nodeKind = NodeKind::Other;
      if (!inCpusNode && stringsEqual(name, "cpus")) {
        inCpusNode = true;
        nodeKind = NodeKind::Cpus;
      } else if (inCpusNode && isCpuNode(name, nameLength)) {
        nodeKind = NodeKind::Cpu;
      }

      nodeStack[depth] = nodeKind;
      countedCpuNode[depth] = false;

      cursor += nameLength + 1;
      cursor = alignPtr(cursor, limit);
      if (cursor == nullptr) {
        return 0;
      }

      ++depth;
      break;
    }
    case kFdtEndNode: {
      if (depth == 0) {
        return 0;
      }

      const NodeKind finishedNode = nodeStack[depth - 1];
      --depth;
      if (finishedNode == NodeKind::Cpus) {
        inCpusNode = false;
      }
      break;
    }
    case kFdtProp: {
      if (cursor + 8 > limit || depth == 0) {
        return 0;
      }

      const uint32_t propertyLength = readBe32(cursor);
      cursor += 4;
      const uint32_t nameOffset = readBe32(cursor);
      cursor += 4;

      if (nameOffset >= state.stringsSize || cursor + propertyLength > limit) {
        return 0;
      }

      const char *propertyName = state.stringsBlock + nameOffset;
      if (nodeStack[depth - 1] == NodeKind::Cpu && !countedCpuNode[depth - 1] &&
          stringsEqual(propertyName, "reg") && propertyLength >= 4) {
        ++cpuCount;
        countedCpuNode[depth - 1] = true;
      }

      cursor += propertyLength;
      cursor = alignPtr(cursor, limit);
      if (cursor == nullptr) {
        return 0;
      }
      break;
    }
    case kFdtNop:
      break;
    case kFdtEnd:
      return cpuCount;
    default:
      return 0;
    }
  }

  return 0;
}

} // namespace

uintptr_t deviceTreeAddress() { return gDeviceTreeAddress; }

size_t cpuCoreCount() { return gCpuCoreCount; }

bool processDeviceTree(uintptr_t deviceTreePhysAddr) {
  gDeviceTreeAddress = deviceTreePhysAddr;
  gCpuCoreCount = 0;

  ParserState state = {};
  if (!loadParserState(deviceTreePhysAddr, state)) {
    return false;
  }

  gCpuCoreCount = countCpuNodes(state);
  return gCpuCoreCount > 0;
}

} // namespace DTB
} // namespace Boot
} // namespace CPU
