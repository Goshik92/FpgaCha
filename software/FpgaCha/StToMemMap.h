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
#include <algorithm>
#include <iterator>
#include <atomic>

namespace FpgaCha
{
    // Register map of S2M adapter
    struct StToMemMap
    {
    private:
        volatile uint32_t _length;
        volatile uint32_t _address;
        volatile uint32_t _irq;

    public:
        // Starts moving data to <address>
        // Moves <length> 256-bit items
        void start(uint32_t address, uint32_t length) volatile
        {
            _address = address;
            _length = length;
        }
        
        // Clears pending interrupt
        void clearIrq() volatile
        {
            _irq = 0;
        }
    };
}