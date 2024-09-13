#ifndef BYTECOV_HPP
#define BYTECOV_HPP

#include <cstdint>

// 检查当前系统是否是小端字节序
bool isLittleEndian() {
    uint16_t test = 0x0001;
    return *(reinterpret_cast<uint8_t*>(&test)) == 1;
}

// ntohs: 网络字节序到主机字节序的16位转换
uint16_t ntohs(uint16_t net) {
    if (isLittleEndian()) {
        return (net >> 8) | (net << 8);  // 交换高低字节
    }
    return net;  // 如果是大端字节序，直接返回
}

// htons: 主机字节序到网络字节序的16位转换
uint16_t htons(uint16_t host) {
    return ntohs(host);  // htons 和 ntohs 实现相同
}

// ntohl: 网络字节序到主机字节序的32位转换
uint32_t ntohl(uint32_t net) {
    if (isLittleEndian()) {
        return ((net >> 24) & 0x000000FF) |
               ((net >> 8)  & 0x0000FF00) |
               ((net << 8)  & 0x00FF0000) |
               ((net << 24) & 0xFF000000);  // 交换每个字节的位置
    }
    return net;
}

// htonl: 主机字节序到网络字节序的32位转换
uint32_t htonl(uint32_t host) {
    return ntohl(host);  // htonl 和 ntohl 实现相同
}

// ntohll: 网络字节序到主机字节序的64位转换
uint64_t ntohll(uint64_t net) {
    if (isLittleEndian()) {
        return ((net >> 56) & 0x00000000000000FFULL) |
               ((net >> 40) & 0x000000000000FF00ULL) |
               ((net >> 24) & 0x0000000000FF0000ULL) |
               ((net >> 8)  & 0x00000000FF000000ULL) |
               ((net << 8)  & 0x000000FF00000000ULL) |
               ((net << 24) & 0x0000FF0000000000ULL) |
               ((net << 40) & 0x00FF000000000000ULL) |
               ((net << 56) & 0xFF00000000000000ULL);  // 交换64位中每个字节的位置
    }
    return net;
}

// htonll: 主机字节序到网络字节序的64位转换
uint64_t htonll(uint64_t host) {
    return ntohll(host);  // htonll 和 ntohll 实现相同
}

#endif