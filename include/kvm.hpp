#ifndef KVM_HPP
#define KVM_HPP


#include <sys/ioctl.h>

#include <linux/kvm.h>

#include <baseclass.hpp>
#include <vm.hpp>


constexpr const char* DEV_KVM = "/dev/kvm";


class VM;

class KVM : public BaseClass {
    public:
        explicit KVM(int fd);
        ~KVM();

        int mmap_size;

        int kvmCreateVM(VM** ptr_vm, uint64_t ram_size, int vcpu_num);

    protected:
        int fd;

    private:
        void kvmCapCheck();

        // KVM state
        int api_ver;
        // CAP
        int immediate_exit;
        int nr_slots, nr_as;
        int soft_vcpus_limit, hard_vcpus_limit;
        int coalesced_mmio;
};

#endif  // KVM_HPP
