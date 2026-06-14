using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

public static class PreCompute
{
    private const ulong KernelGranuleSize = 64 * 1024;
    private const ulong L2SpanSize = 512UL * 1024 * 1024;
    private const int L3Entries = 8192;
    private const string KernelSymbolsSectionName = ".kernel_symbols";
    private const string NumKernelSymbolsName = "NUM_KERNEL_SYMBOLS";

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    private readonly record struct KernelSymbolEntry(ulong Start, ulong Size, ulong NamePointer);

    private sealed record SymbolSection(ulong VirtualAddress, ulong FileOffset, ulong Size);
    private sealed record ElfSymbol(ulong Address, ulong Size, string Name);
    private sealed record LoadSegment(ulong VirtualAddress, ulong FileOffset, ulong FileSize, ulong MemorySize);

    public static int Main(string[] args)
    {
        try
        {
            if (args.Length != 2)
            {
                Console.Error.WriteLine("usage: Jc.OS.RaspBootPreCompute <kernel8.elf> <board>");
                return 1;
            }

            string elf = args[0];
            string board = args[1];

            if (!File.Exists(elf))
            {
                Console.Error.WriteLine($"ELF not found: {elf}");
                return 1;
            }

            ulong physMmioStart = board switch
            {
                "bsp_rpi3" => 0x3F000000UL,
                "bsp_rpi4" => 0xFE000000UL,
                _ => throw new InvalidOperationException($"Unsupported board: {board}")
            };

            ulong kernelVirtAddrSpaceSize = SymbolAddr(elf, "__kernel_virt_addr_space_size");
            ulong kernelVirtStart = SymbolAddr(elf, "__kernel_virt_start_addr");
            ulong codeStart = SymbolAddr(elf, "__code_start");
            ulong codeEndExclusive = SymbolAddr(elf, "__code_end_exclusive");
            ulong dataStart = SymbolAddr(elf, "__data_start");
            ulong dataEndExclusive = SymbolAddr(elf, "__data_end_exclusive");
            ulong heapStart = SymbolAddr(elf, "__heap_start");
            ulong heapEndExclusive = SymbolAddr(elf, "__heap_end_exclusive");
            ulong mmioRemapStart = SymbolAddr(elf, "__mmio_remap_start");
            ulong mmioRemapEndExclusive = SymbolAddr(elf, "__mmio_remap_end_exclusive");
            ulong bootCoreStackStart = SymbolAddr(elf, "__boot_core_stack_start");
            ulong bootCoreStackEndExclusive = SymbolAddr(elf, "__boot_core_stack_end_exclusive");
            ulong physCodeStart = SymbolAddr(elf, "__phys_code_start");
            ulong physDataStart = SymbolAddr(elf, "__phys_data_start");
            ulong physHeapStart = SymbolAddr(elf, "__phys_heap_start");
            ulong physBootCoreStackStart = SymbolAddr(elf, "__phys_boot_core_stack_start");
            ulong numKernelSymbolsVirt = SymbolAddr(elf, NumKernelSymbolsName);

            int numL2Tables = checked((int)(kernelVirtAddrSpaceSize / L2SpanSize));
            if ((kernelVirtAddrSpaceSize % L2SpanSize) != 0 || numL2Tables <= 0)
                throw new InvalidOperationException("Unsupported kernel virtual address space size");

            ulong kernelTablesVirt = SymbolAddr(elf, "KERNEL_TABLES");
            ulong physArgAddr = SymbolAddr(elf, "PHYS_KERNEL_TABLES_BASE_ADDR");
            ulong kernelTablesPhys = physDataStart + (kernelTablesVirt - dataStart);
            SymbolSection kernelSymbolsSection = ReadSection(elf, KernelSymbolsSectionName);
            ulong numKernelSymbolsOffset = VirtualAddressToFileOffset(elf, numKernelSymbolsVirt);

            string tempDir = Path.Combine(Path.GetTempPath(), "jcos-precompute-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(tempDir);

            try
            {
                string dataBin = Path.Combine(tempDir, "data.bin");
                string textBin = Path.Combine(tempDir, "text.bin");
                string kernelSymbolsBin = Path.Combine(tempDir, "kernel_symbols.bin");

                Run("aarch64-elf-objcopy", $"--dump-section .data={Quote(dataBin)} --dump-section .text={Quote(textBin)} --dump-section {KernelSymbolsSectionName}={Quote(kernelSymbolsBin)} {Quote(elf)}");

                byte[] dataBytes = File.ReadAllBytes(dataBin);
                byte[] textBytes = File.ReadAllBytes(textBin);
                byte[] kernelSymbolsBytes = File.ReadAllBytes(kernelSymbolsBin);

                byte[] tableBytes = BuildTranslationTables(
                    kernelVirtStart,
                    kernelVirtAddrSpaceSize,
                    numL2Tables,
                    kernelTablesPhys,
                    codeStart,
                    codeEndExclusive,
                    physCodeStart,
                    dataStart,
                    dataEndExclusive,
                    physDataStart,
                    heapStart,
                    heapEndExclusive,
                    physHeapStart,
                    mmioRemapStart,
                    mmioRemapEndExclusive,
                    physMmioStart,
                    bootCoreStackStart,
                    bootCoreStackEndExclusive,
                    physBootCoreStackStart);

                long dataOffset = checked((long)(kernelTablesVirt - dataStart));
                if (dataOffset < 0 || dataOffset + tableBytes.Length > dataBytes.Length)
                    throw new InvalidOperationException("table patch out of range");

                Buffer.BlockCopy(tableBytes, 0, dataBytes, (int)dataOffset, tableBytes.Length);

                ulong l3Bytes = (ulong)(numL2Tables * L3Entries * sizeof(ulong));
                ulong physTablesBaseAddr = kernelTablesPhys + l3Bytes;

                long textOffset = checked((long)(physArgAddr - codeStart));
                if (textOffset < 0 || textOffset + 8 > textBytes.Length)
                    throw new InvalidOperationException("arg patch out of range");

                byte[] physTablesBaseBytes = BitConverter.GetBytes(physTablesBaseAddr);
                if (!BitConverter.IsLittleEndian)
                    Array.Reverse(physTablesBaseBytes);

                Buffer.BlockCopy(physTablesBaseBytes, 0, textBytes, (int)textOffset, 8);

                IReadOnlyList<ElfSymbol> symbols = ReadKernelSymbols(elf);
                byte[] symbolBlob = BuildKernelSymbolBlob(symbols, kernelSymbolsSection);
                if ((ulong)symbolBlob.Length > kernelSymbolsSection.Size)
                    throw new InvalidOperationException(
                        $"kernel symbols section too small: need {symbolBlob.Length} bytes, have {kernelSymbolsSection.Size}");

                Array.Clear(kernelSymbolsBytes, 0, kernelSymbolsBytes.Length);
                Buffer.BlockCopy(symbolBlob, 0, kernelSymbolsBytes, 0, symbolBlob.Length);

                File.WriteAllBytes(dataBin, dataBytes);
                File.WriteAllBytes(textBin, textBytes);
                File.WriteAllBytes(kernelSymbolsBin, kernelSymbolsBytes);

                Run("aarch64-elf-objcopy", $"--update-section .data={Quote(dataBin)} --update-section .text={Quote(textBin)} --update-section {KernelSymbolsSectionName}={Quote(kernelSymbolsBin)} {Quote(elf)}");
                PatchU64(elf, numKernelSymbolsOffset, checked((ulong)symbols.Count));
            }
            finally
            {
                if (Directory.Exists(tempDir))
                    Directory.Delete(tempDir, true);
            }

            Console.WriteLine($"Patched precomputed translation tables and kernel symbols into {elf}");
            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex.Message);
            return 1;
        }
    }

    private static byte[] BuildTranslationTables(
        ulong kernelVirtStart,
        ulong kernelVirtAddrSpaceSize,
        int numL2Tables,
        ulong kernelTablesPhys,
        ulong codeStart,
        ulong codeEndExclusive,
        ulong physCodeStart,
        ulong dataStart,
        ulong dataEndExclusive,
        ulong physDataStart,
        ulong heapStart,
        ulong heapEndExclusive,
        ulong physHeapStart,
        ulong mmioRemapStart,
        ulong mmioRemapEndExclusive,
        ulong physMmioStart,
        ulong bootCoreStackStart,
        ulong bootCoreStackEndExclusive,
        ulong physBootCoreStackStart)
    {
        using var ms = new MemoryStream();
        using var bw = new BinaryWriter(ms);

        ulong attrCode = AttrBits(isCache: true, readOnly: true, pxn: false);
        ulong attrDram = AttrBits(isCache: true, readOnly: false, pxn: true);
        ulong attrDev = AttrBits(isCache: false, readOnly: false, pxn: true);

        for (ulong l2 = 0; l2 < (ulong)numL2Tables; l2++)
        {
            for (ulong l3 = 0; l3 < L3Entries; l3++)
            {
                ulong virt = kernelVirtStart + (l2 << 29) + (l3 << 16);

                ulong pageDesc = 0;
                if (virt >= codeStart && virt < codeEndExclusive)
                {
                    ulong phys = physCodeStart + (virt - codeStart);
                    pageDesc = BuildPageDescriptor(phys, attrCode);
                }
                else if (virt >= dataStart && virt < dataEndExclusive)
                {
                    ulong phys = physDataStart + (virt - dataStart);
                    pageDesc = BuildPageDescriptor(phys, attrDram);
                }
                else if (virt >= heapStart && virt < heapEndExclusive)
                {
                    ulong phys = physHeapStart + (virt - heapStart);
                    pageDesc = BuildPageDescriptor(phys, attrDram);
                }
                else if (virt >= mmioRemapStart && virt < mmioRemapEndExclusive)
                {
                    ulong phys = physMmioStart + (virt - mmioRemapStart);
                    pageDesc = BuildPageDescriptor(phys, attrDev);
                }
                else if (virt >= bootCoreStackStart && virt < bootCoreStackEndExclusive)
                {
                    ulong phys = physBootCoreStackStart + (virt - bootCoreStackStart);
                    pageDesc = BuildPageDescriptor(phys, attrDram);
                }
                bw.Write(pageDesc);
            }
        }

        ulong coveredVirtSpace = (ulong)numL2Tables * L2SpanSize;
        if (coveredVirtSpace != kernelVirtAddrSpaceSize)
            throw new InvalidOperationException("Translation table coverage does not match kernel VA size");

        for (ulong l2 = 0; l2 < (ulong)numL2Tables; l2++)
        {
            ulong lvl3Phys = kernelTablesPhys + (l2 * (ulong)L3Entries * sizeof(ulong));
            ulong shifted = (lvl3Phys >> 16) & 0xFFFFFFFFFFFUL;
            ulong tableDesc = (shifted << 16) | 0b11UL;
            bw.Write(tableDesc);
        }

        return ms.ToArray();
    }

    private static ulong BuildPageDescriptor(ulong phys, ulong attr)
    {
        ulong shifted = (phys >> 16) & 0xFFFFFFFFFFFUL;
        return (shifted << 16) | (1UL << 10) | 0b11UL | attr;
    }

    private static byte[] BuildKernelSymbolBlob(IReadOnlyList<ElfSymbol> symbols, SymbolSection section)
    {
        int entrySize = Marshal.SizeOf<KernelSymbolEntry>();
        ulong descriptorBytes = checked((ulong)entrySize * (ulong)symbols.Count);
        ulong stringTableVirtStart = section.VirtualAddress + descriptorBytes;

        var stringBytes = new List<byte>();
        var entries = new List<KernelSymbolEntry>(symbols.Count);

        foreach (ElfSymbol symbol in symbols)
        {
            ulong namePointer = stringTableVirtStart + checked((ulong)stringBytes.Count);
            byte[] encodedName = Encoding.ASCII.GetBytes(symbol.Name);
            stringBytes.AddRange(encodedName);
            stringBytes.Add(0);

            entries.Add(new KernelSymbolEntry(symbol.Address, symbol.Size, namePointer));
        }

        ulong totalSize = descriptorBytes + checked((ulong)stringBytes.Count);
        if (totalSize > section.Size)
            throw new InvalidOperationException(
                $"kernel symbols section too small: need {totalSize} bytes, have {section.Size}");

        using var ms = new MemoryStream(checked((int)totalSize));
        using var bw = new BinaryWriter(ms);

        foreach (KernelSymbolEntry entry in entries)
        {
            bw.Write(entry.Start);
            bw.Write(entry.Size);
            bw.Write(entry.NamePointer);
        }

        bw.Write(stringBytes.ToArray());
        return ms.ToArray();
    }

    private static ulong AttrBits(bool isCache, bool readOnly, bool pxn)
    {
        ulong desc = 0;

        if (isCache)
            desc |= (0b11UL << 8) | (0b001UL << 2);
        else
            desc |= (0b010UL << 8) | (0b000UL << 2);

        if (readOnly)
            desc |= (0b10UL << 6);

        if (pxn)
            desc |= (1UL << 53);

        desc |= (1UL << 54);
        return desc;
    }

    private static IReadOnlyList<ElfSymbol> ReadKernelSymbols(string elf)
    {
        string output = Run("aarch64-elf-nm", $"-n -S --defined-only {Quote(elf)}");
        var symbols = new List<ElfSymbol>();

        foreach (string rawLine in output.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            string line = rawLine.Trim();
            Match match = Regex.Match(line, @"^(?<addr>[0-9A-Fa-f]+)\s+(?<size>[0-9A-Fa-f]+)\s+\S\s+(?<name>.+)$");
            if (!match.Success)
                continue;

            ulong size = ParseNum(match.Groups["size"].Value);
            if (size == 0)
                continue;

            ulong address = ParseNum(match.Groups["addr"].Value);
            string demangled = Demangle(match.Groups["name"].Value);
            symbols.Add(new ElfSymbol(address, size, demangled));
        }

        return symbols;
    }

    private static string Demangle(string symbolName)
    {
        string output = Run("c++filt", Quote(symbolName));
        return output.TrimEnd('\r', '\n');
    }

    private static ulong SymbolAddr(string elf, string symbolName)
    {
        string output = Run("aarch64-elf-nm", $"-n {Quote(elf)}");

        foreach (string line in output.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            string[] parts = line.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 3)
                continue;

            if (parts[2] == symbolName)
                return ParseNum(parts[0]);
        }

        throw new InvalidOperationException($"Missing symbol: {symbolName}");
    }

    private static SymbolSection ReadSection(string elf, string sectionName)
    {
        string output = Run("aarch64-elf-readelf", $"-W -S {Quote(elf)}");
        string[] lines = output.Split('\n', StringSplitOptions.RemoveEmptyEntries);

        foreach (string rawLine in lines)
        {
            string line = rawLine.Trim();
            if (!line.Contains($"] {sectionName} "))
                continue;

            Match match = Regex.Match(
                line,
                @"^\[\s*\d+\]\s+\S+\s+\S+\s+(?<addr>[0-9A-Fa-f]+)\s+(?<off>[0-9A-Fa-f]+)\s+(?<size>[0-9A-Fa-f]+)\s+");
            if (!match.Success)
                break;

            return new SymbolSection(
                ParseNum(match.Groups["addr"].Value),
                ParseNum(match.Groups["off"].Value),
                ParseNum(match.Groups["size"].Value));
        }

        throw new InvalidOperationException($"Missing section: {sectionName}");
    }

    private static ulong VirtualAddressToFileOffset(string elf, ulong virtualAddress)
    {
        foreach (LoadSegment segment in ReadLoadSegments(elf))
        {
            ulong segmentEnd = segment.VirtualAddress + segment.MemorySize;
            if (virtualAddress >= segment.VirtualAddress && virtualAddress < segmentEnd)
            {
                ulong relative = virtualAddress - segment.VirtualAddress;
                if (relative >= segment.FileSize)
                    throw new InvalidOperationException($"Virtual address 0x{virtualAddress:X} is not backed by file data");

                return segment.FileOffset + relative;
            }
        }

        throw new InvalidOperationException($"No load segment contains virtual address 0x{virtualAddress:X}");
    }

    private static IReadOnlyList<LoadSegment> ReadLoadSegments(string elf)
    {
        string output = Run("aarch64-elf-readelf", $"-W -l {Quote(elf)}");
        var segments = new List<LoadSegment>();

        foreach (string rawLine in output.Split('\n', StringSplitOptions.RemoveEmptyEntries))
        {
            string line = rawLine.Trim();
            Match match = Regex.Match(
                line,
                @"^LOAD\s+0x(?<off>[0-9A-Fa-f]+)\s+0x(?<virt>[0-9A-Fa-f]+)\s+0x[0-9A-Fa-f]+\s+0x(?<filesz>[0-9A-Fa-f]+)\s+0x(?<memsz>[0-9A-Fa-f]+)\s+");
            if (!match.Success)
                continue;

            segments.Add(new LoadSegment(
                ParseNum(match.Groups["virt"].Value),
                ParseNum(match.Groups["off"].Value),
                ParseNum(match.Groups["filesz"].Value),
                ParseNum(match.Groups["memsz"].Value)));
        }

        return segments;
    }

    private static void PatchU64(string path, ulong fileOffset, ulong value)
    {
        byte[] bytes = BitConverter.GetBytes(value);
        if (!BitConverter.IsLittleEndian)
            Array.Reverse(bytes);

        using var stream = new FileStream(path, FileMode.Open, FileAccess.Write, FileShare.None);
        stream.Position = checked((long)fileOffset);
        stream.Write(bytes, 0, bytes.Length);
    }

    private static ulong ParseNum(string value)
    {
        string trimmed = value.Trim();
        if (trimmed.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
            return ulong.Parse(trimmed[2..], NumberStyles.HexNumber, CultureInfo.InvariantCulture);

        return ulong.Parse(trimmed, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
    }

    private static string Run(string fileName, string arguments)
    {
        var psi = new ProcessStartInfo(fileName, arguments)
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };

        using var process = Process.Start(psi) ?? throw new InvalidOperationException($"Failed to start: {fileName}");

        string stdout = process.StandardOutput.ReadToEnd();
        string stderr = process.StandardError.ReadToEnd();

        process.WaitForExit();

        if (process.ExitCode != 0)
        {
            var sb = new StringBuilder();
            sb.AppendLine($"Command failed: {fileName} {arguments}");
            if (!string.IsNullOrWhiteSpace(stderr))
                sb.AppendLine(stderr.TrimEnd());
            throw new InvalidOperationException(sb.ToString().TrimEnd());
        }

        return stdout;
    }

    private static string Quote(string value)
    {
        return $"\"{value.Replace("\"", "\\\"")}" + "\"";
    }
}
