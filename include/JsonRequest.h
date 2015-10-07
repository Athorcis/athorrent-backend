#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>

class JsonRequest {
    public:
        JsonRequest(std::string action, std::map<std::string, std::string> parameters);
    
        const std::string & getAction() const;
        
        bool hasParameter(const std::string & key) const;
        const std::string & getParameter(const std::string & key) const;
        
        static const JsonRequest * parse(const std::string & buffer);
        
    private:
        std::string m_action;
        std::map<std::string, std::string> m_parameters;
};

#endif // REQUEST_H
