#include "Transaction.h"
#include "Util.h"

namespace Bitcoin
{
    BinaryData Transaction::GetHash()
    {
        ByteBuffer buf;
        buf << *this;
        return Crypto::SHA256D(buf.Binary());
    }
}
