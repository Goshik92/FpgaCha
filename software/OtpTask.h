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

#include "ChaCha20/State.h"
#include "ChaCha20/BCount.h"

// Describes the need for a block of OTP 
// or a complete block of OTP 
struct OtpTask
{
    // Sequence number of OTP block
    size_t id;
    
    // Buffer to store OTP block
    uint32_t* buffer;
    
    // Size of the buffer in words
    size_t length;
    
    // Returns the number of OTP blocks that can fit the buffer
    uint32_t getOtpCount()
    {
        return length / ChaCha20::State::WORD_SIZE;
    }
    
    // Get block count (see ChaCha20 docs) based on the initial
    // block count and the ID
    void getBCount(const ChaCha20::BCount& base, ChaCha20::BCount& result)
    {
        result[0] = base[0] + getOtpCount() * id;
    }
};