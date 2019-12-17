/***********************************************
* Author: Igor Semenov (is0031@uah.edu)
* Platform: Terasic DE2-115
* Date: Dec 2019
* Description: Tests file encryption
***********************************************/

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <string.h>
#include "DevMem.h"
#include "PhyMap.h"
#include "FpgaCha.h"

void FpgaChaEncrypt(std::ifstream& inFile, std::ofstream& outFile)
{
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
    
    DevMem devMem = DevMem();
    
    auto fpgaChaMap = PhyMap<FpgaCha::Dev>(devMem, 0xFF200000);
    auto fpgaChaDev = fpgaChaMap();
    
    // Start block count for ChaCha with 0
    uint32_t blockCount = 0;
    
    // Configure FpgaCha
    fpgaChaDev->SetConstants();
    fpgaChaDev->SetKey(key);
    fpgaChaDev->SetNonce(nonce);
    fpgaChaDev->SetBlockCount(blockCount++);
    fpgaChaDev->Start();
   
    while(true)
    {
        // Buffers for 512-bit chunks of data
        uint32_t plainText[16], cypherText[16];
        
        // Try to read 512 bits from file at once
        std::streamsize length = inFile.readsome((char*)plainText, sizeof(plainText));
        
        // Keep encrypting till end of file
        if (length == 0) break;
        
        // Wait till one-time pad is ready
        while(fpgaChaDev->Busy());
        
        // Copy one-time pad to local storage,
        // so that we can start generating a new one
        FpgaCha::State oneTimePad;
        fpgaChaDev->CopyState(oneTimePad);
        
        // Start generating a new one-time pad
        fpgaChaDev->SetBlockCount(blockCount++);
        fpgaChaDev->Start();
        
        // Do the XOR encryption for all words read from the file
        // The number of words can be less than 16 if it's the end
        // of the file. That is why the condition of for loop is complex
        for(int i = 0; i < (length + sizeof(uint32_t) - 1) / sizeof(uint32_t); i++)
        {
            cypherText[i] = oneTimePad[i] ^ plainText[i];
        }
        
        // Write valid (depending on the number read words) words
        // to the output file
        outFile.write((char*)cypherText, length);
    }
}

void NoEncrypt(std::ifstream& inFile, std::ofstream& outFile)
{
    while(true)
    {
        // Buffers for 512-bit chunks of data
        uint32_t plainText[16], cypherText[16];
        
        // Try to read 512 bits from file at once
        std::streamsize length = inFile.readsome((char*)plainText, sizeof(plainText));
        
        // Keep "encrypting till end of file"
        if (length == 0) break;
        
        // Write valid (depending on the number read words) words
        // to the output file
        outFile.write((char*)cypherText, length);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Validate input arguments
        if (argc < 4) throw std::runtime_error("too few arguments passed");
        
        // Open input and output files
        std::ifstream inFile(argv[1], std::ios::binary);
        std::ofstream outFile(argv[2], std::ios::binary);
        
        // Choose encryption method
        if (strcmp(argv[3], "-fpga") == 0) FpgaChaEncrypt(inFile, outFile);
        else NoEncrypt(inFile, outFile);
    }
    
    catch (std::runtime_error e)
    {
        std::cout << e.what() << std::endl;
    }
}