#ifndef JSON_RESPONSE_H
#define JSON_RESPONSE_H

#include <rapidjson/document.h>

class JsonResponse
{
    public:
        JsonResponse();

        rapidjson::Document::AllocatorType & getAllocator();

        std::string getStatus() const;
        void setStatus(const std::string & status);

        void setSuccess(const std::string & message);
        void setError(const std::string & message);
        void setError(const std::string & message, const std::string & id);

        rapidjson::Value & getData();

        std::string toRawResponse() const;

    private:
        rapidjson::Document m_doc;
};

#endif // JSON_RESPONSE_H
