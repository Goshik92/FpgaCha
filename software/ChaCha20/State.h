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

#include <array>
#include "Consts.h"
#include "Key.h"
#include "Nonce.h"
#include "BCount.h"

namespace ChaCha20
{
    // ChaCha20 state
    struct State
    {
        // Sizes of the state
        static const size_t BYTE_SIZE = 512 / 8;
        static const size_t WORD_SIZE = BYTE_SIZE / sizeof(uint32_t);
        
        // Fields of the state
        Consts consts;
        Key key;
        BCount bCount; 
        Nonce nonce;
        
        // Word access to the state
        volatile uint32_t& operator[](int i) volatile
        {
            return (reinterpret_cast<volatile uint32_t*>(this))[i];
        }
        
        // Word access to the state
        const volatile uint32_t& operator[](int i) volatile const
        {
            return (reinterpret_cast<const volatile uint32_t*>(this))[i];
        }
        
        // Prints the state
        void print() volatile const
        {
            for(int i = 0; i < WORD_SIZE; i++)
            {
                std::cout << std::setfill('0') << std::setw(8);
                std::cout << std::hex << (*this)[i];
                if (i % 4 == 3) std::cout << std::endl;
                else std::cout << " ";
            }
        }
    };
}