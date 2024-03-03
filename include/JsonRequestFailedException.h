#ifndef JSON_REQUEST_FAILED_EXCEPTION_H
#define JSON_REQUEST_FAILED_EXCEPTION_H

#include <stdexcept>

class JsonRequestFailedException : public std::runtime_error {

    std::string m_id;

public:
    explicit JsonRequestFailedException(const char *id);
    explicit JsonRequestFailedException(const char *id, const char *what);
    explicit JsonRequestFailedException(const char *id, const std::string &what);

    const std::string & getId() const;
};

#endif //JSON_REQUEST_FAILED_EXCEPTION_H
