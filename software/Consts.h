#pragma once

#include <array>

namespace FpgaCha
{
    typedef std::array<uint32_t, 4> Consts;
    
    const Consts DEFAULT_CONSTS = 
    {
        0x61707865, 0x3320646e,
        0x79622d32, 0x6b206574
    };
}