#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <exception>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/kvm.h>
#include <boot.hpp>
#include <util.hpp>
#include <vm.hpp>


int VM::allocGuestRAM() {
    // not caring about hugetlbpage
    ram_start = mmap(NULL, vm_conf.ram_size, (PROT_READ|PROT_WRITE),
                        (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE), -1, 0);

    if (ram_start == MAP_FAILED) {
        perror(("VM::" + std::string(__func__) + ": mmap").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__ << ": VM.ram_start mmaped: "
        << ram_start << std::endl;

    if (madvise(ram_start, vm_conf.ram_size, MADV_MERGEABLE) < 0) {
        perror(("VM::" + std::string(__func__) + ": madvise").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__
        << ": VM.ram_start madvised as MERGEABLE: "
        << ram_start << std::endl;

    return 0;
}

int VM::setUserMemRegion() {
    // TMP implementation
    // We don't implement membank yet. So there's a limitation of ram_size!
    // RECOMMENDED: userspace_addr[0:20] == guest_phys_addr[0:20]
    int r;
    user_memory_region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = vm_conf.ram_size,
        .userspace_addr = reinterpret_cast<uint64_t>(ram_start),
    };

    r = kvmIoctl(KVM_SET_USER_MEMORY_REGION, &user_memory_region);

    if (r < 0)
        perror(("VM::" + std::string(__func__) + ": kmvIoctl").c_str());
    std::cout << "VM::" << __func__ << ": registered" << std::endl;

    return r;
}

int VM::createVcpu() {
    int r;
    vcpus = (Vcpu*)operator new[](vm_conf.vcpu_num*sizeof(Vcpu));
    std::cout << "VM::vcpus: " << vcpus << std::endl;

    for (int i = 0; i < vm_conf.vcpu_num; i++) {
        r = kvmIoctl(KVM_CREATE_VCPU, i);
        if (r < 0) {
            perror(("VM::" + std::string(__func__) + ": kvmIoctl: ").c_str());
            return -errno;
        }
        std::cout << "VM::" << __func__ << ": "
            << "fd=" << r << " cpu_id= " << i << std::endl;
        new(&vcpus[i]) Vcpu(r, kvm, i);
        std::cout << "&VM.vcpus[" << i << "]: " << &vcpus[i] << std::endl;
    }

    return 0;
}

int VM::initMachine() {
    int r;
    for (const InitMachineFunc e : initmachine_func) {
        r = (this->*e)();
        if (r < 0)
            return r;
    }
    std::cout << "VM::" << __func__ << ": success" << std::endl;
    return 0;
}

int VM::initRAM(std::string cmdline) {
    ebda* ebda_start = reinterpret_cast<ebda*>((
                reinterpret_cast<uint8_t*>(ram_start)+EBDA_START));
    ebda* ebda_end;
    char* ramdisk_image = reinterpret_cast<char*>(ram_start)
                                + INITRAMFS_ADDR;
    uint32_t ramdisk_size;
    char* cmdline_start = reinterpret_cast<char*>(ram_start)
                                + COMMANDLINE_ADDR;
    char* cmdline_end;
    boot_params* boot_params_start = reinterpret_cast<boot_params*>((
                reinterpret_cast<uint8_t*>(ram_start)+BOOT_PARAMS_ADDR));
    boot_params* boot_params_end;

    ebda ebda_data = gen_ebda(vm_conf.vcpu_num);

    std::cout << "ebda generated" << '\n';
    std::cout << "ebda_data.fps.checksum: "
        << ebda_data.fps.checksum+0 << '\n';
    std::cout << "ebda_data.ctable.checksum: "
        << ebda_data.ctable.checksum+0 << std::endl;

    if (is_mp_checksum_valid(&ebda_data.fps)) {
        std::cout << "ebda_data.fps.checksum is valid." << std::endl;
    } else {
        std::cerr << "ebda_data.fps.checksum corrupted" << std::endl;
    }

    if (is_mp_checksum_valid(&ebda_data.ctable)) {
        std::cout << "ebda_data.ctable.checksum is valid." << std::endl;
    } else {
        std::cerr << "ebda_data.ctable.checksum corrupted" << std::endl;
    }

    ebda_end = std::copy_n(&ebda_data, 1, ebda_start);
    std::cout << "ebda_data copied to guest RAM: " << ebda_start << std::endl;
    std::cout << "ebda_end: " << ebda_end << std::endl;

    // TODO:
    // REF L159: we should load setup header first cuz we don't know
    // max command line size till we do it.
    // - [ ]  0. set virtio-net up (we won't do it for now)
    //
    // <--- in another func
    //
    // ---> in this func
    // - [ ]  1. poisoning the guest RAM for debugging here (optional)
    // - [x]  2. copy command line to guest RAM. make sure it is null-terminated!
    // - [x]  3. get the size of initramfs and copy it to guest RAM
    // - [x]  4. read bootparam setup header data from kernel
    // - [x]  5. write into bootparam e820 entries / setup header
    // - [x]  6. bootparam into guest RAM
    // - [ ]  7. copy protected-mode kernel into guest RAM
    // <--- in this func
    //
    // in another func --->
    // - [ ]  8. init vCPUs regs
    // - [ ]  9. add devs cmos noop
    // - [ ] 10. init ioporthandler

    // initramfs
    ramdisk_size = get_ifs_size(initramfs);
    std::cout << "initramfs size: " << ramdisk_size << std::endl;
    if (!initramfs.read(ramdisk_image, ramdisk_size)) {
        std::cerr << "cannnot read from initramfs" << std::endl;
        return 1;
    }
    std::cout << "initramfs copied to guest RAM: "
        << static_cast<void*>(ramdisk_image) << std::endl;

    // cmdline
    // FIXME: should check commandline size before do this
    std::cout << "cmdline size: " << cmdline.size() << std::endl;
    cmdline_end = std::copy_n(cmdline.begin(), cmdline.size(), cmdline_start);
    *cmdline_end = '\0';  // null-terminate
    std::cout << "cmdline copied to guest RAM: "
        << static_cast<void*>(cmdline_start) << std::endl;
    std::cout << "cmdline_end: "
        << static_cast<void*>(cmdline_end) << std::endl;

    // bootparam
    boot_params bp;

    kernel.seekg(SETUP_HEADER_ADDR, std::ios::beg);
    kernel.read(reinterpret_cast<char*>(&bp.header), sizeof(bp.header));
    if (!kernel) {
        kernel.seekg(0, std::ios::beg);
        std::cerr << "Couldn't read a setup header from the kernel image" << std::endl;
        return 1;
    }
    kernel.seekg(0, std::ios::beg);
    std::cout << "bootparam setup header has been loaded from the kernel image" << std::endl;

    if (bp.header.is_valid()) {
        std::cout << "bootparam setup header is valid" << std::endl;
    } else {
        std::cerr << "bootparam setup header is invalid or the boot protocol version is old" << std::endl;
        return 1;
    }

    if (bp.header.check_setup_sects())
        std::cout << "The value of setup_sects has been modified to 4" << std::endl;

    std::cout << "Writing to bootparam..." << std::endl;

    bp.add_e820_entry(REALMODE_IVT_START, EBDA_START-REALMODE_IVT_START, BOOT_E820_TYPE_RAM);
    bp.add_e820_entry(EBDA_START, VGARAM_START-EBDA_START, BOOT_E820_TYPE_RESERVED);
    bp.add_e820_entry(MBBIOS_START, MBBIOS_SIZE, BOOT_E820_TYPE_RESERVED);
    bp.add_e820_entry(HIGHMEM_BASE, vm_conf.ram_size, BOOT_E820_TYPE_RAM);

    // change later to print these writing processs
    // some kind of setter?
    bp.header.vid_mode       = BOOT_HDR_VID_MODE_NML;
    bp.header.type_of_loader = BOOT_HDR_BLT_UNDEFINED;
    bp.header.loadflags      = BOOT_HDR_LF_HIGH
                             | BOOT_HDR_LF_KEEP_SGMT
                             | BOOT_HDR_LF_HEAP;
    bp.header.ramdisk_image  = INITRAMFS_ADDR;
    bp.header.ramdisk_size   = ramdisk_size;
    bp.header.heap_end_ptr   = BOOT_PARAMS_ADDR - BOOT_HDR_HEAPEND_OFFSET;
    bp.header.cmd_line_ptr   = COMMANDLINE_ADDR;

    boot_params_end = std::copy_n(&bp, 1, boot_params_start);
    std::cout << "boot_params copied to guest RAM: "
        << static_cast<void*>(boot_params_start) << std::endl;
    std::cout << "boot_params_end: "
        << static_cast<void*>(boot_params_end) << std::endl;

    std::cout << "VM::" << __func__ << ": success" << std::endl;

    return 0;
}

VM::VM(int vm_fd, KVM& kvm, vm_config vm_conf)\
        : BaseClass(vm_fd), kvm(kvm), vm_conf(vm_conf) {
    std::cout << "Constructing VM..." << std::endl;

    // Currently we only assumes the case where unrestricted_guest == 1.
    // So we won't call following APIs:
    // - KVM_SET_TSS
    // - KVM_SET_IDENTITY_MAP

    // Create VIOAPIC, VPIC. Set up future vCPUs to use them as a local APIC.
    // - GSI00-15 -> IOAPIC/PIC
    // - GIS16-23 -> IOAPIC
    kvmIoctlCtor(KVM_CREATE_IRQCHIP);
    std::cout << "IRQCHIP created" << std::endl;
    kvmIoctlCtor(KVM_CREATE_PIT2, &pit_config);
    std::cout << "PIT2 created" << std::endl;

    kernel.open(vm_conf.kernel_path, std::ios::in | std::ios::binary);
    initramfs.open(vm_conf.initramfs_path, std::ios::in | std::ios::binary);

    if (!kernel)
        throw std::runtime_error("Cloud not open the kernel: "
                + std::string(vm_conf.kernel_path));
    if (!initramfs)
        throw std::runtime_error("Cloud not open the initramfs: "
                + std::string(vm_conf.initramfs_path));

    if (is_kernel_elf(kernel))
        throw std::runtime_error("VM::"+std::string(__func__)+
                ": the kernel is elf file");
    std::cout << "Verified that the kernel is not an elf file" << std::endl;

    std::cout << "Constructed VM." << std::endl;
}

VM::~VM() {}
