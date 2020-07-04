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

#include "State.h"

namespace ChaCha20
{
    // Software ChaCha20 implementation
    class ChaCha20
    {
    private:
        inline uint32_t rotl32(uint32_t x, int n) 
        {
            return (x << n) | (x >> (32 - n));
        }
        
        void inline quaterRound(State& s, int a, int b, int c, int d)
        {
            s[a] += s[b]; 
            s[d] = rotl32(s[d] ^ s[a], 16);
            s[c] += s[d]; 
            s[b] = rotl32(s[b] ^ s[c], 12);
            s[a] += s[b]; 
            s[d] = rotl32(s[d] ^ s[a], 8);
            s[c] += s[d];
            s[b] = rotl32(s[b] ^ s[c], 7);
        }

    public:
        State state;
    
        // Computes ChaCha20 with encryption parameters <s>
        // The result can be found in this->state
        void compute(const State& s)
        {
            state = s;
            
            // Do rounds
            for (int i = 0; i < 10; i++) 
            {
                quaterRound(state, 0, 4,  8, 12);
                quaterRound(state, 1, 5,  9, 13);
                quaterRound(state, 2, 6, 10, 14);
                quaterRound(state, 3, 7, 11, 15);
                quaterRound(state, 0, 5, 10, 15);
                quaterRound(state, 1, 6, 11, 12);
                quaterRound(state, 2, 7,  8, 13);
                quaterRound(state, 3, 4,  9, 14);
            }
            
            // Do summation
            for (int i = 0; i < State::WORD_SIZE; i++)
            {
                state[i] += s[i];
            }
        }
    };
}