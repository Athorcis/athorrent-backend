#include "Utils.h"
#include <cstdint>

using namespace std;

inline int8_t hex_to_int(char in)
{
    if (in >= '0' && in <= '9') {
        return (int)in - '0';
    }
    
    if (in >= 'A' && in <= 'F') {
        return (int)in - 'A' + 10;
    }

    if (in >= 'a' && in <= 'f') {
        return (int)in - 'a' + 10;
    }
    
    return -1;
}

string hex2bin(const string & hex) {

    string bin;

    for (string::const_iterator it = hex.begin(); it != hex.end(); ++it) {
        
        int8_t n = hex_to_int(*it);

        if (n == -1) {
            return string();
        }
        
        bin += static_cast<char>(n << 4);
        
        n = hex_to_int(*++it);
        
        if (n == -1) {
            return string();
        }
        
        bin[bin.size() - 1] |= n & 0xf;
    }

    return bin;
}

string bin2hex(const string & bin)
{
    static const char map[] = "0123456789ABCDEF";
    
    std::string hex;

    for (string::const_iterator it = bin.begin(); it != bin.end(); ++it) {

        uint8_t c = *it;

        hex += map[c >> 4];
        hex += map[c & 0xf];
    }

    return hex;
}
