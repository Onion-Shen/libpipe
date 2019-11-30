#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include "UtilDef.h"

namespace Crypto
{
extern std::string b64encode(std::string buffer, bool newline = false);

extern std::string b64decode(std::string buffer, bool newline = false);

extern std::basic_string<Util::byte> sha1encode(Util::byte *src, size_t len);
};

#endif