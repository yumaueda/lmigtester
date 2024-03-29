/*
 *  include/baseclass.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_BASECLASS_HPP_
#define INCLUDE_BASECLASS_HPP_


#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>


class BaseClass {
 public:
    explicit BaseClass(int fd);
    ~BaseClass();

    template<typename... kvmIoctlArgs>
    int kvmIoctl(unsigned long request, kvmIoctlArgs... args) {
        return _kvmIoctl(request, false, args...);
    }

    template<typename... kvmIoctlArgs>
    int kvmIoctlCtor(unsigned long request, kvmIoctlArgs... args) {
        return _kvmIoctl(request, true, args...);
    }

 protected:
    int fd = -1;

 private:
    template<typename... kvmIoctlArgs>
    int _kvmIoctl(unsigned long request,
            bool raise_exception, kvmIoctlArgs... args) {
        errno = 0;
#ifdef MONITOR_IOCTL
        constexpr bool monitor_ioctl = true;
#else
        constexpr bool monitor_ioctl = false;
#endif  // MONITOR_IOCTL

        int r = ioctl(fd, request, args...);

        if (monitor_ioctl || raise_exception) {
            std::ostringstream args_oss;
            args_oss << fd << ',' << request;
            ((args_oss << ',' << args), ...);
            std::string args_str = args_oss.str();

            if (monitor_ioctl)
                std::cout << (std::string(__func__)
                        + ": ioctl("
                        + args_str + "): "
                        + strerror(errno))
                    << std::endl;

            if (r < 0 && raise_exception)
                    // The type of exception should be detailed.
                    throw std::runtime_error(std::string(__func__)
                            + ": ioctl(" + args_str + "): "
                            + strerror(errno));
        }

        return r < 0 ? -errno : r;
    }
};


#endif  // INCLUDE_BASECLASS_HPP_
