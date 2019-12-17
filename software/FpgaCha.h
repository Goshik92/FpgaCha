/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
***********************************************/

#include <stdbool.h>
#include <stdint.h>
#include <algorithm>
#include <iterator>

#pragma once

namespace FpgaCha
{
    typedef uint32_t State[16];
    typedef uint32_t Key[8];
    typedef uint32_t Nonce[3];

    struct __attribute__((packed)) Dev
    {
    private:
        volatile State InitState;
        volatile State FinalState;
        volatile uint32_t Control;
    
    public:
        void SetConstants() volatile
        {
            InitState[0] = 0x61707865;
            InitState[1] = 0x3320646e;
            InitState[2] = 0x79622d32;
            InitState[3] = 0x6b206574;
        }
        
        void SetKey(Key key) volatile
        {
            for(int i = 0; i < 8; i++) 
                InitState[i + 4] = key[i];
        }
        
        void SetBlockCount(uint32_t blockCount) volatile
        {
            InitState[12] = blockCount;
        }
        
        void SetNonce(Nonce nonce) volatile
        {
            InitState[13] = nonce[0];
            InitState[14] = nonce[1];
            InitState[15] = nonce[2];
        }
        
        void Start() volatile
        {
            Control = 10;
        }
        
        bool Busy() volatile
        {
            return Control != 0;
        }
        
        void CopyState(State& state) volatile
        {
            // memcpy discards volatile quantifier,
            // so I had to use a loop;
            for(int i = 0; i < 16; i++) state[i] = FinalState[i];
        }
    };
}