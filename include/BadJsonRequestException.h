//
// Created by athor on 24/10/2022.
//

#ifndef ATHORRENT_BACKEND_BADJSONREQUESTEXCEPTION_H
#define ATHORRENT_BACKEND_BADJSONREQUESTEXCEPTION_H

#include <stdexcept>

class BadJsonRequestException : public std::runtime_error {
public:
    explicit BadJsonRequestException(const char * what);
    explicit BadJsonRequestException(const std::string & what);
};


#endif //ATHORRENT_BACKEND_BADJSONREQUESTEXCEPTION_H
