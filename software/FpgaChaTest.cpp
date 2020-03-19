#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdexcept>
#include "Key.h"
#include "Nonce.h"
#include "State.h"
#include "BlockCount.h"
#include "FpgaCha.h"
#include "UDmaBuf.h"

// Key from an example in RFC8439 (see page 10)
FpgaCha::Key key = 
{
    0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
    0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c
};

// Nonce from an example in RFC8439 (see page 10)
FpgaCha::Nonce nonce = {0x09000000, 0x4a000000, 0x00000000};

// Block count from an example in RFC8439 (see page 10)
FpgaCha::BlockCount blockCount = { 0x00000001 };

int main()
{
    try
    {
        uint32_t otpCount = 10;
        
        auto uDmaBuf = FpgaCha::UDmaBuf("udmabuf0");
        std::cout << "UDmaBuf physical: " << uDmaBuf.physical << std::endl;
        std::cout << "UDmaBuf size: " << uDmaBuf.size << std::endl;
        std::cout << "UDmaBuf content: " << std::endl;
        for(int i = 0; i < 16; i++)
        {
            std::cout << std::setfill('0') << std::setw(8);
            std::cout << std::hex << uDmaBuf.content[i];
            if (i % 4 == 3) std::cout << std::endl;
            else std::cout << " ";
        }
        
        auto fpgaCha0 = FpgaCha::FpgaCha("uio0");
        
        FpgaCha::State initialState;
        initialState.consts = FpgaCha::DEFAULT_CONSTS;
        initialState.key = key;
        initialState.blockCount = blockCount;
        initialState.nonce = nonce;
        
        fpgaCha0.chacha20->setState(initialState);
        fpgaCha0.chacha20->start(otpCount);
        fpgaCha0.stToMem->start(otpCount, uDmaBuf.physical);
        fpgaCha0.waitForIrq();
        fpgaCha0.stToMem->clearIrq();
        fpgaCha0.enableIrq();
        
        /*// Print the result
        std::cout << "Hardware one-time pad:" << std::endl;
        for(int i = 0; i < 16; i++)
        {
            std::cout << std::setfill('0') << std::setw(8);
            std::cout << std::hex << hwOneTimePad[i];
            if (i % 4 == 3) std::cout << std::endl;
            else std::cout << " ";
        }
        
        // Print the reference value
        std::cout << std::endl;
        std::cout << "Reference one-time pad (from RFC8439):" << std::endl;
        for(int i = 0; i < 16; i++)
        {
            std::cout << std::setfill('0') << std::setw(8);
            std::cout << std::hex << refOneTimePad[i];
            if (i % 4 == 3) std::cout << std::endl;
            else std::cout << " ";
        }
        
        // Check if values match
        std::cout << std::endl;
        int mismatchCount = 0;
        for(int i = 0; i < 16; i++)
        {
            if (hwOneTimePad[i] != refOneTimePad[i])
            {
                std::cout << "Word " << std::dec << i << " does not match" << std::endl;
                mismatchCount++;
            }
        }
        
        // Print the result of comparison
        std::cout << mismatchCount << " words mismatch" << std::endl;*/
    }
    
    catch (std::runtime_error e)
    {
        std::cout << e.what() << std::endl;
    }
}