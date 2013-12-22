#include "Crypto.h"

namespace Crypto
{
    std::vector<byte> SHA256(std::vector<byte> data)
    {
        std::vector<byte> hash;
        hash.resize(SHA256_DIGEST_LENGTH);
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, &data[0], data.size());
        SHA256_Final(&hash[0], &sha256);
        return std::vector<byte>(hash.begin(), hash.end());
    }
    
    std::vector<byte> SHA256(std::string data)
    {
        return SHA256(std::vector<byte>(data.begin(), data.end()));
    }
    
    std::vector<byte> SHA256D(std::vector<byte> data)
    {
        return SHA256(SHA256(data));
    }
}
