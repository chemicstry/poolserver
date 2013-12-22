#include "Transaction.h"
#include "Util.h"

namespace Bitcoin
{
    std::vector<byte> Transaction::GetHash()
    {
        ByteBuffer buf;
        buf << *this;
        return Crypto::SHA256D(buf.Bytes());
    }
}
