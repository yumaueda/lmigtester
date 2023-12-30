#ifndef UTIL_HPP
#define UTIL_HPP


#include <fstream>
#include <ios>
#include <iostream>


// These funcs don't change the value of the position indicator of kernel.
static inline std::streamsize get_ifs_size(std::ifstream& file) {
    std::ios::pos_type original_pos = file.tellg();

    if (!file) {
        std::cerr << __func__ << ": invalid ifstream given" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    file.seekg(original_pos, std::ios::beg);

    return size;
}

static inline bool is_kernel_elf(std::ifstream& kernel) {
    char buf[ELF_MAGIC_SIZE];
    std::ios::pos_type original_pos = kernel.tellg();

    if (!kernel) {
        std::cerr << __func__ << ": invalid ifstream given" << std::endl;
        return false;
    }

    kernel.seekg(0, std::ios::beg);
    kernel.read(buf, ELF_MAGIC_SIZE);
    kernel.seekg(original_pos);

    return std::equal(buf, buf+ELF_MAGIC_SIZE, ELF_MAGIC);
};


#endif  // UTIL_HPP