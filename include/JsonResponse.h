#ifndef RESPONSE_H
#define RESPONSE_H

#include <libjson/libjson.h>

class JsonResponse {
    public:
        JsonResponse(JSONNODE * node);
        JsonResponse(const char * message, bool success);
        
        bool isSuccess() const;      
        const JSONNODE * getNode() const;
        
        std::string toRawResponse() const;
        
    private:
        bool m_success;  
        JSONNODE * m_node;
};

#endif // RESPONSE_H
