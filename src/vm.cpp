#include <iostream>
#include <vm.hpp>

VM::VM(KVM& kvm, int vmfd, const uint64_t ram_size, const int vcpu_num) : kvm(kvm), vmfd(vmfd), ram_size(ram_size), vcpu_num(vcpu_num) {
    std::cout << "Constructing VM..." << std::endl;
    vcpus = NULL;
    errno = 0;
    int r;

    /*
        * Create VIOAPIC, VPIC. Set up future vCPUs to use them as a local APIC.
        * GSI00-15 -> IOAPIC/PIC
        * GIS16-23 -> IOAPIC
        */
    r = ioctl(this->vmfd, KVM_CREATE_IRQCHIP);
    if (r < 0)
        std::cerr << "KVM_CREATE_IRQCHIP failed" << std::endl;
        // exception!
    r = ioctl(this->vmfd, KVM_CREATE_PIT2, &this->pit_config);
    if (r < 0)
        std::cerr << "KVM_CREATE_PIT2 failed" << std::endl;
        // exception!

    // RAM
    // not caring about hugetlbpage
    this->ram_start = NULL;
    this->ram_page_size = getpagesize();
    // FIX? 32_BIT_GAP and mprotect(PROT_NONE)?
    this->ram_start = mmap(NULL, this->ram_size, (PROT_READ|PROT_WRITE), (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE), -1, 0);
    if (this->ram_start == MAP_FAILED) {
        std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
        std::cerr << "VM.ram_size: " << this->ram_size << std::endl;
        // exception!
    } else {
        errno = 0;
        if (madvise(this->ram_start, this->ram_size, MADV_MERGEABLE) < 0)
            std::cerr << "madvise failed: " << std::strerror(errno) << std::endl;
    }


    // vCPU(s)
    // some kind of kvm_vcpu struct is needed. will be implemented soon
    this->vcpus = new Vcpu*[vcpu_num];

    for (int i = 0; i < this->vcpu_num; i++) {
        r = ioctl(this->vmfd, KVM_CREATE_VCPU, i);
        if (r < 0) {
            std::cerr << "KVM_CREATE_VCPU failed" << std::endl;
            // exception
        } else {
            std::cout << "KVM_CREATE_VCPU cpuid: " << i << " vcpufd: " << r << std::endl;
            this->vcpus[i] = new Vcpu(*this, i, r);
        }
    }


}

VM::~VM() {
    errno = 0;
    std::cout << "Destructing VM..." << std::endl;
    for (int i = 0; i < this->vcpu_num; i++) {
        std::cout << "deleted VM.vcpus[i]" << std::endl;
        delete vcpus[i];
    }
    if (vcpus != NULL) {
        std::cout << "deleted VM.vcpus" << std::endl;
        delete vcpus;
    }
    if (this->ram_start != NULL) {
        if (munmap(this->ram_start, this->ram_size) < 0)
            std::cerr << "munmap VM.ram_start failed: " << std::strerror(errno) << std::endl;
        else
            std::cout << "munmapped VM.ram_start" << std::endl;
    }
    if (this->vmfd >= 0) {
        close(this->vmfd);
        std::cout << "closed VM.vmfd" << std::endl;
    }

}
