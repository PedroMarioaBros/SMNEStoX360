#include "smb360/sha1.hpp"
#include <array>
#include <iomanip>
#include <sstream>
#include <vector>

namespace smb360 {
namespace {
static std::uint32_t rol(std::uint32_t x, unsigned n) { return (x << n) | (x >> (32u - n)); }
static std::uint32_t be32(const std::uint8_t* p) {
    return (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16) |
           (std::uint32_t(p[2]) << 8) | std::uint32_t(p[3]);
}
}

Sha1Digest sha1(const void* ptr, std::size_t size) {
    const auto* input = static_cast<const std::uint8_t*>(ptr);
    const std::uint64_t bit_len = static_cast<std::uint64_t>(size) * 8u;
    std::size_t padded = size + 1u;
    while ((padded % 64u) != 56u) ++padded;
    std::vector<std::uint8_t> msg(padded + 8u, 0);
    for (std::size_t i=0;i<size;++i) msg[i]=input[i];
    msg[size]=0x80u;
    for (unsigned i=0;i<8;++i) msg[padded+i]=static_cast<std::uint8_t>(bit_len >> (56u-8u*i));

    std::uint32_t h0=0x67452301u,h1=0xefcdab89u,h2=0x98badcfeu,h3=0x10325476u,h4=0xc3d2e1f0u;
    for (std::size_t off=0;off<msg.size();off+=64u) {
        std::array<std::uint32_t,80> w{};
        for (unsigned i=0;i<16;++i) w[i]=be32(&msg[off+i*4u]);
        for (unsigned i=16;i<80;++i) w[i]=rol(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
        std::uint32_t a=h0,b=h1,c=h2,d=h3,e=h4;
        for (unsigned i=0;i<80;++i) {
            std::uint32_t f=0,k=0;
            if (i<20) { f=(b&c)|((~b)&d); k=0x5a827999u; }
            else if (i<40) { f=b^c^d; k=0x6ed9eba1u; }
            else if (i<60) { f=(b&c)|(b&d)|(c&d); k=0x8f1bbcdcu; }
            else { f=b^c^d; k=0xca62c1d6u; }
            const std::uint32_t temp=rol(a,5)+f+e+k+w[i];
            e=d; d=c; c=rol(b,30); b=a; a=temp;
        }
        h0+=a;h1+=b;h2+=c;h3+=d;h4+=e;
    }
    Sha1Digest out{};
    const std::uint32_t h[5]={h0,h1,h2,h3,h4};
    for (unsigned j=0;j<5;++j) for (unsigned i=0;i<4;++i)
        out[j*4+i]=static_cast<std::uint8_t>(h[j] >> (24u-8u*i));
    return out;
}

std::string sha1_hex(const void* data, std::size_t size) {
    const auto d=sha1(data,size);
    std::ostringstream os; os << std::hex << std::setfill('0');
    for (auto b:d) os << std::setw(2) << unsigned(b);
    return os.str();
}
}
