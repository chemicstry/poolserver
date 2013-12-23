#include "Crypto.h"

namespace Crypto
{
    BinaryData SHA256(BinaryData data)
    {
        std::vector<byte> hash;
        hash.resize(SHA256_DIGEST_LENGTH);
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, &data[0], data.size());
        SHA256_Final(&hash[0], &sha256);
        return std::vector<byte>(hash.begin(), hash.end());
    }
    
    BinaryData SHA256(std::string data)
    {
        return SHA256(BinaryData(data.begin(), data.end()));
    }
    
    BinaryData SHA256D(BinaryData data)
    {
        return SHA256(SHA256(data));
    }
}
