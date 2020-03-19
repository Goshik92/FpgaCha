/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
***********************************************/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <iterator>
#include <array>
#include "State.h"

namespace FpgaCha
{
    struct __attribute__((packed)) ChaCha20Map
    {
        
    private:
        volatile State _state;
        volatile uint32_t _padCount;
    
    public:        
        void setState(const State& state) volatile
        {
            // Copy state word by word to prevent problems with volatile
            auto src = reinterpret_cast<const uint32_t*>(&state);
            auto dst = reinterpret_cast<volatile uint32_t*>(&_state);
            for(int i = 0; i < sizeof(State)/sizeof(uint32_t); i++)
            {
                dst[i] = src[i];
            }
        }
        
        void start(uint32_t padCount) volatile
        {
            _padCount = padCount;
        }
    };
}