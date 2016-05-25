#ifndef JSON_REQUEST_H
#define JSON_REQUEST_H

#include <rapidjson/document.h>
#include <string>

class JsonRequest : protected rapidjson::Document
{
    public:
        JsonRequest(const std::string & buffer);

        std::string getAction() const;

        bool hasParameter(const std::string & key) const;
        std::string getParameter(const std::string & key) const;
};

#endif // JSON_REQUEST_H
