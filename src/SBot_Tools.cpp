#include <SBot_PriVate/SBot_Tools.h>

#include<random>
#include<sstream>
#include<iomanip>
std::string gen_echo()
{
    static std::random_device rd;
    static std::uniform_int_distribution<uint64_t> dist(0ULL, 0xFFFFFFFFFFFFFFFFULL);
    uint64_t ab = dist(rd);
     uint64_t cd = dist(rd);
     uint32_t a, b, c, d;
     std::stringstream ss;
     ab = ( ab & 0xFFFFFFFFFFFF0FFFULL ) | 0x0000000000004000ULL;
     cd = ( cd & 0x3FFFFFFFFFFFFFFFULL ) | 0x8000000000000000ULL;
     a  = ( ab >> 32U );
     b  = ( ab & 0xFFFFFFFFU);
     c  = ( cd >> 32U );
     d  = ( cd & 0xFFFFFFFFU);
     ss << std::hex << std::nouppercase << std::setfill('0');
     ss << std::setw(8) << (a) << '-';
     ss << std::setw(4) << (b >> 16U) << '-';
     ss << std::setw(4) << (b & 0xFFFFU) << '-';
     ss << std::setw(4) << (c >> 16U) << '-';
     ss << std::setw(4) << (c & 0xFFFFU);
     ss << std::setw(8) << d;
    return ss.str();
}