/*******************************************************************************
 * Copyright 2020 Igor Semenov (goshik92@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *******************************************************************************/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <iterator>
#include <array>
#include "../ChaCha20/State.h"

namespace FpgaCha
{
    // Registers of ChaCha20 accelerator core
    struct __attribute__((packed)) ChaCha20Map
    {
    private:
        volatile ChaCha20::State _state;
        volatile uint32_t _padCount;
        volatile uint32_t _roundCount;
        volatile uint32_t _probe;
    
    public:
        // Prints the content of registers
        void test() volatile
        {
            uint32_t p = _probe;
            bool isOk = p == 0xfb7e03d9;
            std::cout << "Probe is " << (isOk ? "OK: " : "WRONG: ") << p << std::endl;
            std::cout << "Pad count: " << _padCount << std::endl;
            std::cout << "Round count: " << _roundCount << std::endl;
            std::cout << "State: " << std::endl;
            _state.print();
        }
    
        // Sets encryption parameters
        void setState(const ChaCha20::State& state) volatile
        {
            // Copy state word by word to prevent problems with volatile
            for(int i = 0; i < ChaCha20::State::WORD_SIZE; i++)
            {
                _state[i] = state[i];
            }
        }
        
        // Starts pad computations
        // padCount - number of OTP blocks to compute
        void start(uint32_t padCount) volatile
        {
            _padCount = padCount;
        }
    };
}