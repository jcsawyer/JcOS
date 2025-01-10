#pragma once

#include <memory/memory_register.hpp>
#include "../../exception.hpp"
#include <panic.hpp>
#include <printf.h>

namespace Exception
{
    namespace CurrentEL
    {
        enum Value
        {
            EL2 = 0b10,
            EL1 = 0b01,
            EL0 = 0b00
        };
        class CurrentExLevel : public Memory::InMemoryRegister<CurrentExLevel>
        {
        public:
            static constexpr Exception::CurrentEL::Value get()
            {
                uint64_t value = 0;
                asm volatile("mrs %0, CurrentEL" : "=r"(value));
                return static_cast<Exception::CurrentEL::Value>((value >> 2) & 0b11); // Bits [3:2] hold the EL value
            }

            static constexpr void
            set(uint64_t value)
            {
                panic("Cannot write to CurrentEL register");
            };
        };

        // Determines the current privilege level
        inline PrivilegeLevel current_privilege_level(const char **el_string)
        {
            CurrentEL::Value el = CurrentExLevel::get();
            switch (el)
            {
            case CurrentEL::EL2:
                *el_string = "EL2";
                return PrivilegeLevel::Hypervisor;
            case CurrentEL::EL1:
                *el_string = "EL1";
                return PrivilegeLevel::Kernel;
            case CurrentEL::EL0:
                *el_string = "EL0";
                return PrivilegeLevel::User;
            default:
                *el_string = "Unknown";
                return PrivilegeLevel::Unknown;
            }
        }
    }

    class SpsrEL1 : public Memory::InMemoryRegister<SpsrEL1>
    {
    public:
        static constexpr uint64_t get()
        {
            uint64_t value = 0;
            asm volatile("mrs %0, spsr_el1" : "=r"(value));
            return value;
        };

        static constexpr void set(uint64_t value)
        {
            asm volatile("msr spsr_el1, %0" : : "r"(value));
        };

        static constexpr void print() {
            printf_("SPSR_EL1: 0x%08x\n", get());
            printf_("      Flags:");
            printf_("            Negative: %s\n", toFlatStr(get() & 0x80000000));
            printf_("            Zero: %s\n", toFlatStr(get() & 0x40000000));
            printf_("            Carry: %s\n", toFlatStr(get() & 0x20000000));
            printf_("            Overflow: %s\n", toFlatStr(get() & 0x10000000));

            printf_("      Exception handling state:\n");
            printf_("            Debug  (D): %s\n", toMaskStr(get() & 0x800000));
            printf_("            SError (A): %s\n", toMaskStr(get() & 0x400000));
            printf_("            IRQ    (I): %s\n", toMaskStr(get() & 0x200000));
            printf_("            FIQ    (F): %s\n", toMaskStr(get() & 0x100000));
        
            printf_("      Illegal Execution State (IL): %s\n", toFlatStr(get() & 0x80000));
        }

        static constexpr char* toFlatStr(uint64_t value) {
            if (value) return "Set";
            return "Unset";
        }
        static constexpr char* toMaskStr(uint64_t value) {
            if (value) return "Masked";
            return "Unmasked";
        }
    };

    // ESR_EL1 Register
    class EsrEL1 : public Memory::InMemoryRegister<EsrEL1>
    {
    public:
        static constexpr uint64_t get()
        {
            uint64_t value = 0;
            asm volatile("mrs %0, esr_el1" : "=r"(value));
            return value;
        };

        static constexpr void set(uint64_t value)
        {
            asm volatile("msr esr_el1, %0" : : "r"(value));
        };
    };

    struct ExceptionContext
    {
        // General Purpose Registers
        uint64_t gpr[30];
        // The link register
        uint64_t lr;
        // Exception Link Register
        uint64_t elr_el1;
        // Saved Program Status Register
        SpsrEL1 spsr_el1;
        // Exception Syndrome Register
        EsrEL1 esr_el1 = EsrEL1();
    };

  //extern "C" uint64_t __exception_vector_start;

    extern "C" void defaultExceptionHandler(ExceptionContext *context);

    void handlingInit();

} // namespace Exception
