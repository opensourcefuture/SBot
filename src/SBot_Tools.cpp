#include <SBot_Private/SBot_Tools.h>

#include<random>
#include<sstream>
#include<iomanip>
#include<regex>

#ifdef _WIN32
    #include <objbase.h>
#endif


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

std::string to_ansi(const std::string & utf8_str)
{
    #ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), utf8_str.size(), NULL, 0);
        std::wstring unicode_buf(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), utf8_str.size(), unicode_buf.data(), len);
        len = WideCharToMultiByte(CP_ACP, 0, unicode_buf.data(), unicode_buf.size(), NULL, 0, NULL, NULL);
        std::string ansi_buf(len, '\0');
        WideCharToMultiByte(CP_ACP, 0, unicode_buf.data(), unicode_buf.size(), ansi_buf.data(), len, NULL, NULL);
        return ansi_buf;
    #else
        return utf8_str;
    #endif
}

std::string to_utf8(const std::string & ansi_str)
{
    #ifdef _WIN32
        int len = MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), ansi_str.size(), NULL, 0);
        std::wstring unicode_buf(len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), ansi_str.size(), unicode_buf.data(), len);
        len = WideCharToMultiByte(CP_UTF8, 0, unicode_buf.data(), unicode_buf.size(), NULL, 0, NULL, NULL);
        std::string utf8_buf(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, unicode_buf.data(), unicode_buf.size(), utf8_buf.data(), len, NULL, NULL);
        return utf8_buf;
    #else
        return ansi_str;
    #endif
}

std::string url_encode(const std::string& url)
{
    //TODO...
    return url;
}

std::vector<std::vector<std::string>> match_all(const std::string& content, const std::string& pattern)
{
    using namespace std;
    std::vector<std::vector<std::string>> ret_vec;
    regex r(pattern);
    smatch sm;
    string::const_iterator search_start(content.cbegin());
    std::vector<std::string> dat_vec;
    while(regex_search(search_start, content.cend(), sm, r))
	{
        vector<string> temp_vec;
        for(size_t i = 0;i < sm.size();++i)
        {
            temp_vec.push_back(sm[i].str());
        }
		ret_vec.push_back(temp_vec);
		search_start = sm.suffix().first;
	}
    return ret_vec;
}