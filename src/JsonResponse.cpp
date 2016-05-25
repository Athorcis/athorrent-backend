#include "JsonResponse.h"

#include <rapidjson/writer.h>
#include <iostream>

using namespace rapidjson;
using namespace std;

JsonResponse::JsonResponse()
{
    m_doc.SetObject();
    m_doc.AddMember("status", "", m_doc.GetAllocator());
    m_doc.AddMember("data", Value(), m_doc.GetAllocator());
}

Document::AllocatorType & JsonResponse::getAllocator()
{
    return m_doc.GetAllocator();
}

string JsonResponse::getStatus() const
{
    return m_doc["status"].GetString();
}

void JsonResponse::setStatus(const string & status)
{
    m_doc["status"].SetString(status, m_doc.GetAllocator());
}

void JsonResponse::setSuccess(const string & message)
{
    setStatus("success");
    m_doc["data"].SetString(message, m_doc.GetAllocator());
}

void JsonResponse::setError(const string & message)
{
    setStatus("error");
    m_doc["data"].SetString(message, m_doc.GetAllocator());
}

Value & JsonResponse::getData()
{
    return m_doc["data"];
}

string JsonResponse::toRawResponse() const {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    m_doc.Accept(writer);
    
    string message = string(buffer.GetString()) + '\n';
    
    cout << message << endl;
    
    return message;
}
