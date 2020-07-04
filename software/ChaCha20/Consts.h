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

namespace ChaCha20
{
    // Constants of ChaCha20
    typedef std::array<uint32_t, 4> Consts;
    
    // Default values of constants for ChaCha20
    const Consts DEFAULT_CONSTS = 
    {
        0x61707865, 0x3320646e,
        0x79622d32, 0x6b206574
    };
}