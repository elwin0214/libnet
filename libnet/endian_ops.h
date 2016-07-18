#ifndef __LIBNET_ENDIAN_OPS_H__
#define __LIBNET_ENDIAN_OPS_H__

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)
#endif

#ifdef __linux__
#include <endian.h>
#include <stdint.h>
#endif

namespace libnet
{
namespace sockets
{

inline uint16_t hostToNetwork16(uint16_t host_16bits)
{
  return htobe16(host_16bits);
};

inline uint16_t network16ToHost(uint16_t big_endian_16bits)
{
  return be16toh(big_endian_16bits);
};

inline uint32_t hostToNetwork32(uint32_t host_32bits)
{
  return htobe32(host_32bits);
};

inline uint32_t network32ToHost(uint32_t big_endian_32bits)
{
  return be32toh(big_endian_32bits);
};

inline uint64_t hostToNetwork64(uint64_t host_64bits)
{
  return htobe64(host_64bits);
};

inline uint64_t network64ToHost(uint64_t big_endian_64bits)
{
  return be64toh(big_endian_64bits);
};

}
}

#endif
