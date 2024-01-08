/*
 *  src/main.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <cstdint>
#include <iostream>

#include <boot.hpp>
#include <kvm.hpp>
#include <vcpu.hpp>
#include <vm.hpp>


int main() {
    int  r;
    KVM* kvm;
    VM*  vm;
    vm_config vm_conf {
        .vcpu_num = 2,
        .ram_size = static_cast<uint64_t>(1) << 30,
        .kernel_path = "bzImage",
        .initramfs_path = "initramfs",
    };

    r = KVM::getKVMFD();
    if (r < 0)
        return -1;

    kvm = new KVM(r);
    r = kvm->kvmCreateVM(&vm, vm_conf);
    if (r < 0) {
        std::cerr << "kvm->kvmCreateVM() failed" << std::endl;
        return -1;
    }

    r = vm->initMachine();
    if (r) {
        std::cerr << "vm->initMachine() failed" << std::endl;
        return -1;
    }

    r = vm->initRAM("dummy");
    if (r) {
        std::cerr << "vm->initRAM() failed" << std::endl;
        return -1;
    }

    r = vm->initVcpuRegs();
    if (r) {
        std::cerr << "vm->initVcpuRegs() failed" << std::endl;
        return -1;
    }

    r = vm->createPageTable(BOOT_PAGETABLE_BASE, false);
    if (r) {
        std::cerr << "vm->createPageTable() failed" << std::endl;
        return -1;
    }

    r = vm->initVcpuSregs(false);
    if (r) {
        std::cerr << "vm->initVcpuSregs() failed" << std::endl;
        return -1;
    }

    return 0;
}
