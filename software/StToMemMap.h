#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <algorithm>
#include <iterator>
#include <atomic>

namespace FpgaCha
{
    struct StToMemMap
    {
    private:
        volatile std::atomic<uint32_t> _length;
        volatile std::atomic<uint32_t> _address;
        volatile std::atomic<uint32_t> _irq;

    public:
        void start(uint32_t address, uint32_t length) volatile
        {
            _address = address;
            _length = length;
        }
        
        void clearIrq() volatile
        {
            _irq = 0;
        }
    };
}