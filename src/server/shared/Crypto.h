#ifndef CRYPTO_H_
#define CRYPTO_H_

#include "Common.h"
#include "Util.h"
#include <string>
#include <vector>
#include <openssl/sha.h>

namespace Crypto
{
    std::vector<byte> SHA256(std::vector<byte> data);
    std::vector<byte> SHA256(std::string data);
    std::vector<byte> SHA256D(std::vector<byte> data);
}

#endif
