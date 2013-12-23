#ifndef CRYPTO_H_
#define CRYPTO_H_

#include "Common.h"
#include "Util.h"
#include <string>
#include <vector>
#include <openssl/sha.h>

namespace Crypto
{
    BinaryData SHA256(BinaryData data);
    BinaryData SHA256(std::string data);
    BinaryData SHA256D(BinaryData data);
}

#endif
