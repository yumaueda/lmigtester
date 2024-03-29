/*
 *  src/post.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <post.hpp>

#include <cstdint>

#include <iodev.hpp>
#include <vm.hpp>


int Post::Read(uint16_t, char*, uint8_t) {
    return 0;
}

int Post::Write(uint16_t, char*, uint8_t) {
    return 0;
}

Post::Post(VM* vm) : IODev(PIO_PORT_POST_START, PIO_PORT_POST_SIZE, vm) {}
