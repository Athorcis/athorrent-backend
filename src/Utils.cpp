#include "Utils.h"

#include <libtorrent/hex.hpp>

using namespace std;

string Utils::from_hex(string const & hex) {
    string ret;

    for (string::const_iterator it = hex.begin(); it != hex.end(); ++it) {
        int n = libtorrent::detail::hex_to_int(*it);

        if (n == -1) {
            return std::string();
        }

        ret += static_cast<char>(n << 4);

        n = libtorrent::detail::hex_to_int(*++it);

        if (n == -1) {
            return std::string();
        }

        ret[ret.size() - 1] |= n & 15;
    }

    return ret;
}
