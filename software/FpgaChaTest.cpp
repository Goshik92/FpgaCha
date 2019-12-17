/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
* Description: Does sanity check of FpgaCha 
***********************************************/

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdexcept>
#include "DevMem.h"
#include "PhyMap.h"
#include "FpgaCha.h"

int main()
{
    try
    {
        DevMem devMem = DevMem();
        
        auto fpgaChaMap = PhyMap<FpgaCha::Dev>(devMem, 0xFF200000);
        auto fpgaChaDev = fpgaChaMap();
        
        // Reference one-time pad from page 11 of RFC8439
        FpgaCha::State refOneTimePad = 
        {
            0xe4e7f110, 0x15593bd1, 0x1fdd0f50, 0xc47120a3,
            0xc7f4d1c7, 0x0368c033, 0x9aaa2204, 0x4e6cd4c3,
            0x466482d2, 0x09aa9f07, 0x05d7c214, 0xa2028bd9,
            0xd19c12b5, 0xb94e16de, 0xe883d0cb, 0x4e3c50a2
        };

        // Key from an example in RFC8439 (see page 10)
        FpgaCha::Key key = 
        {
            0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
            0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c
        };

        // Nonce from an example in RFC8439 (see page 10)
        FpgaCha::Nonce nonce = {0x09000000, 0x4a000000, 0x00000000};

        // Block count from an example in RFC8439 (see page 10)
        uint32_t blockCount = 0x00000001;
        
        // Local storage for one-time pad 
        FpgaCha::State hwOneTimePad;
    
        // Fill INIT_STATE of the hw device
        fpgaChaDev->SetConstants();
        fpgaChaDev->SetKey(key);
        fpgaChaDev->SetBlockCount(blockCount);
        fpgaChaDev->SetNonce(nonce);
               
        // Start computation
        fpgaChaDev->Start();
        
        // Wait for computation to finish
        while(fpgaChaDev->Busy());
        
        // Copy one-time pad to a local storage
        fpgaChaDev->CopyState(hwOneTimePad);
        
        // Print the result
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
        std::cout << mismatchCount << " words mismatch" << std::endl;
    }
    
    catch (std::runtime_error e)
    {
        std::cout << e.what() << std::endl;
    }
}