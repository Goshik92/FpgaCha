#pragma once

#include <array>
#include "Consts.h"
#include "Key.h"
#include "Nonce.h"
#include "BlockCount.h"

namespace FpgaCha
{
    struct State
    {
        Consts consts;
        Key key;
        BlockCount blockCount; 
        Nonce nonce;
    };
}