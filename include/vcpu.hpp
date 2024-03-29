/*
 *  include/vcpu.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */

#ifndef INCLUDE_VCPU_HPP_
#define INCLUDE_VCPU_HPP_


#include <linux/kvm.h>

#include <baseclass.hpp>
#include <kvm.hpp>


class KVM;

constexpr uint32_t KVM_CPUID_ENTRIES_NUM = 100;
constexpr uint32_t KVM_CPUID_SIGNATURE   = 0x40000000;
constexpr uint32_t KVM_CPUID_FEATURES    = 0x40000001;
constexpr uint32_t KVM_CPUID_EBX         = 0x4b4d564b;
constexpr uint32_t KVM_CPUID_ECX         = 0x564b4d56;
constexpr uint32_t KVM_CPUID_EDX         = 0x0000004d;

constexpr uint64_t CR0_PE = 1;
constexpr uint64_t CR0_MP = 1 << 1;
constexpr uint64_t CR0_EM = 1 << 2;
constexpr uint64_t CR0_TS = 1 << 3;
constexpr uint64_t CR0_ET = 1 << 4;
constexpr uint64_t CR0_NE = 1 << 5;
constexpr uint64_t CR0_WP = 1 << 16;
constexpr uint64_t CR0_AM = 1 << 18;
constexpr uint64_t CR0_NW = 1 << 29;
constexpr uint64_t CR0_CD = 1 << 30;
constexpr uint64_t CR0_PG = 1 << 31;

constexpr uint64_t CR3_PWT       = 1 << 3;
constexpr uint64_t CR3_PCD       = 1 << 4;
constexpr uint64_t CR3_PDBR_MASK = (((1UL << 52) - 1) << 12);

constexpr uint64_t CR4_VME        = 1;
constexpr uint64_t CR4_PVI        = 1 << 1;
constexpr uint64_t CR4_TSD        = 1 << 2;
constexpr uint64_t CR4_DE         = 1 << 3;
constexpr uint64_t CR4_PSE        = 1 << 4;
constexpr uint64_t CR4_PAE        = 1 << 5;
constexpr uint64_t CR4_MCE        = 1 << 6;
constexpr uint64_t CR4_PGE        = 1 << 7;
constexpr uint64_t CR4_PCE        = 1 << 8;
constexpr uint64_t CR4_OSFXSR     = 1 << 9;
constexpr uint64_t CR4_OSXMMEXCPT = 1 << 10;
constexpr uint64_t CR4_UMIP       = 1 << 11;
constexpr uint64_t CR4_VMXE       = 1 << 13;
constexpr uint64_t CR4_SMXE       = 1 << 14;
constexpr uint64_t CR4_FSGSBASE   = 1 << 16;
constexpr uint64_t CR4_PCIDE      = 1 << 17;
constexpr uint64_t CR4_OSXSAVE    = 1 << 18;
constexpr uint64_t CR4_SMEP       = 1 << 20;
constexpr uint64_t CR4_SMAP       = 1 << 21;
constexpr uint64_t CR4_PKE        = 1 << 22;
constexpr uint64_t CR4_CET        = 1 << 23;
constexpr uint64_t CR4_PKS        = 1 << 24;

constexpr uint64_t RF_CF   = 1 << 0;
constexpr uint64_t RF_INIT = 1 << 1;
constexpr uint64_t RF_PF   = 1 << 2;
constexpr uint64_t RF_AF   = 1 << 4;
constexpr uint64_t RF_ZF   = 1 << 6;
constexpr uint64_t RF_SF   = 1 << 7;
constexpr uint64_t RF_TF   = 1 << 8;
constexpr uint64_t RF_IF   = 1 << 9;
constexpr uint64_t RF_DF   = 1 << 10;
constexpr uint64_t RF_OF   = 1 << 11;
constexpr uint64_t RF_IOPL = 1 << 12 & 1 << 13;
constexpr uint64_t RF_NT   = 1 << 14;
constexpr uint64_t RF_MD   = 1 << 15;
constexpr uint64_t RF_RF   = 1 << 16;
constexpr uint64_t RF_VM   = 1 << 17;
constexpr uint64_t RF_AC   = 1 << 18;
constexpr uint64_t RF_VIF  = 1 << 19;
constexpr uint64_t RF_VIP  = 1 << 20;
constexpr uint64_t RF_ID   = 1 << 21;
constexpr uint64_t RF_AES  = 1 << 30;
constexpr uint64_t RF_AI   = 1 << 31;

constexpr uint64_t MSR_IA32_EFER_SCE   = 1;
constexpr uint64_t MSR_IA32_EFER_LME   = 1 << 8;
constexpr uint64_t MSR_IA32_EFER_LMA   = 1 << 10;
constexpr uint64_t MSR_IA32_EFER_NXE   = 1 << 11;
constexpr uint64_t MSR_IA32_EFER_SVME  = 1 << 12;
constexpr uint64_t MSR_IA32_EFER_LMSLE = 1 << 13;
constexpr uint64_t MSR_IA32_EFER_FFXSR = 1 << 14;
constexpr uint64_t MSR_IA32_EFER_TCE   = 1 << 15;

constexpr uint16_t SEG_DESC_SELECTOR_TI_GDT     = 0 << 2;
constexpr uint16_t SEG_DESC_SELECTOR_TI_LDT     = 1 << 2;
constexpr uint8_t  SEG_DESC_SELECTOR_IDX_SHIFT  = 3;
constexpr uint16_t SEG_DESC_SELECTOR_RPL_KERNEL = 0;
constexpr uint8_t  SEG_DESC_TYPE_CODE_A         = 1;       // accessed
constexpr uint8_t  SEG_DESC_TYPE_CODE_R         = 1 << 1;  // read
constexpr uint8_t  SEG_DESC_TYPE_CODE_C         = 1 << 2;  // conforming
constexpr uint8_t  SEG_DESC_TYPE_CODE           = 1 << 3;
constexpr uint8_t  SEG_DESC_TYPE_DATA_A         = 1;
constexpr uint8_t  SEG_DESC_TYPE_DATA_W         = 1 << 1;
constexpr uint8_t  SEG_DESC_TYPE_DATA_E         = 1 << 2;
constexpr uint8_t  SEG_DESC_TYPE_DATA           = 0 << 3;
constexpr uint8_t  SEG_DESC_TYPE_FLAG_CD        = 1;
constexpr uint8_t  SEG_DESC_DPL_KERNEL          = 0;
constexpr uint8_t  SEG_DESC_DB_EX_LSET          = 0;
constexpr uint8_t  SEG_DESC_L_64BIT_MODE        = 1;
constexpr uint8_t  SEG_DESC_GRAN_4KB            = 1;

struct vcpu_dregs {
    uint64_t db[4];
    uint64_t dr6;
    uint64_t dr7;
    uint64_t flags;
    uint64_t reserved[9];
};

struct vcpu_regs {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rsp, rbp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
};

struct descriptor_table {
    uint64_t base;
    uint16_t limit;
    uint16_t padding[3];
};

struct vcpu_sregs {
    kvm_segment cs, ds, es, fs, gs, ss;
    kvm_segment tr, ldt;
    descriptor_table gdt, idt;
    uint64_t cr0, cr2, cr3, cr4, cr8;
    uint64_t efer;
    uint64_t apic_base;
    uint64_t interrupt_bitmap[(KVM_NR_INTERRUPTS+63)/64];
};

typedef kvm_segment vcpu_sregs::*SegmentDescriptorPointer;
typedef descriptor_table   vcpu_sregs::*DescriptorTablePointer;


class Vcpu : public BaseClass {
 public:
    explicit Vcpu(int vcpu_fd, KVM* kvm, VM* vm, int cpu_id);
    ~Vcpu();

#ifdef GUEST_DEBUG
    int SetGuestDebug(bool enable, bool singlestep);
#endif  // GUEST_DEBUG

    int InitRegs(uint64_t rip, uint64_t rsi);
    int InitSregs(bool is_elfclass64);
    int RunLoop();

 private:
    KVM* kvm;
    VM*  vm;
    const int cpu_id;

    kvm_cpuid2* kvm_cpuid = static_cast<kvm_cpuid2*>(nullptr);
    kvm_run* run = static_cast<kvm_run*>(nullptr);

    int GetRegs(vcpu_regs *regs);
    int GetSregs(vcpu_sregs *sregs);

    int SetRegs(vcpu_regs *regs);
    int SetSregs(vcpu_sregs *sregs);

    void DumpSegmentDescriptor(kvm_segment& sd);
    void DumpDescriptorTable(descriptor_table& dt);
    int DumpRegs();
    int DumpSregs();

    int Run();
    int RunOnce();
};


#endif  // INCLUDE_VCPU_HPP_
