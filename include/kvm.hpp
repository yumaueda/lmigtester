#ifndef KVM_HPP
#define KVM_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/kvm.h>
#include <baseclass.hpp>
#include <vm.hpp>


struct vm_config;
class VM;


constexpr const char* DEV_KVM = "/dev/kvm";

constexpr const int KVM_CAP_CHECK[] = {
    KVM_CAP_IMMEDIATE_EXIT,
    KVM_CAP_NR_MEMSLOTS,
    KVM_CAP_MULTI_ADDRESS_SPACE,
    KVM_CAP_NR_VCPUS,
    KVM_CAP_MAX_VCPUS,
    KVM_CAP_COALESCED_MMIO,
};

struct kvm_cap {
    int immediate_exit;
    int nr_slots;
    int nr_as;
    int soft_vcpus_limit;
    int hard_vcpus_limit;
    int coalesced_mmio;
};

class KVM : public BaseClass {
    public:
        explicit KVM(int fd);
        ~KVM();

        int mmap_size;

        static int getKVMFD(){
            int r;
            if ((r = open(DEV_KVM, O_RDWR)) < 0) {
                std::cout<< (std::string(__func__) + ": Could not open " + DEV_KVM + ".") << std::endl;
                return -errno;
            }
            return r;
        };

        int kvmCreateVM(VM** ptr_vm, vm_config vm_conf);

    private:
        int api_ver;
        kvm_cap cap;

        int kvmCapCheck();
};


#endif  // KVM_HPP
