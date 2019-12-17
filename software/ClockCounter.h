#include <stdint.h>

#pragma once

namespace ClockCounter
{
    struct __attribute__((packed)) Dev
    {
    private:
        volatile uint32_t low;
        volatile uint32_t high;

    public:
        void Reset() volatile
        {
            low = 0;
        }
        
        uint64_t GetTime() volatile
        {
            return low | ((uint64_t)(high) << 32);
        }
    };
}