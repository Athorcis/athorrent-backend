#ifndef UTILS_H
#define UTILS_H

#include <string>

class Utils {
    public:
        static std::string from_hex(std::string const & hex);

        static std::string toUtf8(const std::string & string);
        static std::string fromUtf8(const std::string & string);
};

#endif /* UTILS_H */
