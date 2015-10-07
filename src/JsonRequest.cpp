#include "JsonRequest.h"
#include "Utils.h"

#include <iostream>

#include <libjson/libjson.h>

using namespace std;

JsonRequest::JsonRequest(const string action, map<string, string> parameters) :
    m_action(action), m_parameters(parameters) {
}

const string & JsonRequest::getAction() const {
    return m_action;
}

bool JsonRequest::hasParameter(const string & key) const {
    return m_parameters.count(key) > 0;
}

const string & JsonRequest::getParameter(const string & key) const {
    return m_parameters.at(key);
}

#define JSON_ENSURE_TYPE(node, type) node == NULL || node == JSON_NULL || json_type(node) != type

const JsonRequest * JsonRequest::parse(const string & buffer) {
    std::string action;
    std::map<std::string, std::string> parameters;
    
    JSONNODE * rootNode = json_parse(buffer.c_str());
    
    if (JSON_ENSURE_TYPE(rootNode, JSON_NODE)) {
        cerr << "Request::parse / JSON_ENSURE_TYPE failed for rootNode" << endl;
        return NULL;
    }
    
    JSONNODE * actionNode = json_get(rootNode, "action");
    
    if (JSON_ENSURE_TYPE(actionNode, JSON_STRING)) {
        cerr << "Request::parse / JSON_ENSURE_TYPE failed for actionNode" << endl;
        return NULL;
    }
    
    action = json_as_string(actionNode);
   
    JSONNODE * parametersNode = json_get(rootNode, "parameters");

    if (JSON_ENSURE_TYPE(parametersNode, JSON_NODE)) {
        cerr << "Request::parse / JSON_ENSURE_TYPE failed for parametersNode" << endl;
        return NULL;
    }
    
    for (JSONNODE_ITERATOR it = json_begin(parametersNode), end = json_end(parametersNode); it != end; ++it) {
        if (JSON_ENSURE_TYPE(*it, JSON_STRING)) {
            cerr << "Request::parse / JSON_ENSURE_TYPE failed for *it" << endl;
            return NULL;
        }
        
        parameters[json_name(*it)] = Utils::toUtf8(json_as_string(*it));
    }
    
    return new JsonRequest(action, parameters);
}
