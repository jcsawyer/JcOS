#include "mmu.hpp"
#include "../memory.hpp"
#include "../../../memory/mmu.hpp"

namespace Memory
{
    // Function to return the array
    TranslationDescriptor *getDescriptors()
    {
        static TranslationDescriptor descriptors[NUM_MEM_RANGES] = {
            {
                "Kernel code and RO data",
                codeStart(),
                codeEndExclusive() - 1,
                TranslationType::Identity(),
                {MemAttributes::CacheableDRAM, AccessPermissions::ReadOnly, false},
            },
            {
                "Remapped Device MMIO",
                0x1FFF0000,
                0x1FFFFFFF,
                TranslationType::Offset(Memory::Map::getMMIO().START + 0x200000),
                {MemAttributes::Device, AccessPermissions::ReadWrite, true},
            },
            {
                "Device MMIO",
                Memory::Map::getMMIO().START,
                Memory::Map::getMMIO().END_INCLUSIVE,
                TranslationType::Identity(),
                {MemAttributes::Device, AccessPermissions::ReadWrite, true},
            },
        };

        return descriptors;
    }
    
    static KernelVirtualLayout<NUM_MEM_RANGES> *kernelVirtualLayout = nullptr;

    KernelVirtualLayout<NUM_MEM_RANGES> *virtMemLayout()
    {
        if (kernelVirtualLayout == nullptr) {
            static KernelVirtualLayout kvl = KernelVirtualLayout<NUM_MEM_RANGES>(Memory::Map::END_INCLUSIVE, getDescriptors());
            kernelVirtualLayout = &kvl;
        }
         
        return kernelVirtualLayout;
    }
} // namespace Memory
