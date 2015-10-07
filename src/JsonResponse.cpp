#include "JsonResponse.h"

JsonResponse::JsonResponse(JSONNODE * node) :
    m_success(true), m_node(node) {
    json_set_name(m_node, "data");
}

JsonResponse::JsonResponse(const char * message, bool success) : m_success(success) {
    m_node = json_new_a("data", message);
}

bool JsonResponse::isSuccess() const {
    return m_success;
}

const JSONNODE * JsonResponse::getNode() const {
    return m_node;
}

std::string JsonResponse::toRawResponse() const {
    JSONNODE * root = json_new(JSON_NODE);
    
    json_push_back(root, json_new_a("status", m_success ? "success" : "error"));
    json_push_back(root, m_node);
    
    std::string rawResponse = json_write(root);
    json_delete(root);
    
    // cout << "Response::toString / result: " << result << endl;
    
    return rawResponse + '\n';
}
