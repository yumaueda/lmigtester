#include <boot.hpp>


// NOTE: we should re-implement some function as the member of an object


static inline
uint8_t _mp_calc_checksum_inline(uint8_t *byteptr_mp, int length) {
    uint8_t checksum = 0;
    for (int i = 0; i < length; ++i)
        checksum += *byteptr_mp++;
    return checksum;
}

template <>
uint8_t mp_calc_checksum<mpfps*>(mpfps* mpptr) {
    uint8_t* byteptr_fps = reinterpret_cast<uint8_t*>(mpptr);
    return _mp_calc_checksum_inline(byteptr_fps, MPFPS_LENGTH*PARAGRAPH_SIZE);
}

template <>
uint8_t mp_calc_checksum<mpctable*>(mpctable* mpptr) {
    uint8_t* byteptr_ctable = reinterpret_cast<uint8_t*>(mpptr);
    return _mp_calc_checksum_inline(byteptr_ctable, MPCTABLE_LENGTH);
}

#ifndef UNITTEST
static
#endif
void processor_entry_init(mpctable_processor_entry* entry_array,
                            const int vcpu_num) {
    for (int i = 0; i < vcpu_num; ++i) {
        entry_array[i].local_apic_id = i;
        entry_array[i].cpu_flags |= MPCTE_PROC_CPUFLAGS_EN;
        entry_array[i].cpu_flags |= i==0 ?
            MPCTE_PROC_CPUFLAGS_BP : 0;
        entry_array[i].cpu_sig = MPCTE_PROC_CPUSIGNATURE;
    }
}

ebda gen_ebda(const int vcpu_num) {
    ebda ebda_ret;

    // checksum is initialized to zero. check boot.hpp
    // see definition of struct ebda
    ebda_ret.fps.phys_addr_ptr = EBDA_START + 0x40;
    ebda_ret.fps.checksum = mp_gen_checksum(&ebda_ret.fps);

    // we don't initialize all memory regions of the array processor_entry
    // since there's no need to do so.
    processor_entry_init(ebda_ret.ctable.processor_entry, vcpu_num);
    ebda_ret.ctable.checksum = mp_gen_checksum(&ebda_ret.ctable);

    return ebda_ret;
}