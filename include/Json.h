#ifndef JSON_H
#define JSON_H

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <string>

#define JSON_OBJECT rapidjson::Document document; \
                    document.SetObject();

#define JSON_ADD_STRING(key, value) document.AddMember(key, rapidjson::Value(value, document.GetAllocator()).Move(), document.GetAllocator());

#define JSON_WRITE(variable)  rapidjson::StringBuffer buffer; \
                              rapidjson::Writer<rapidjson::StringBuffer> writer(buffer); \
                              document.Accept(writer); \
                              std::string variable = std::string(buffer.GetString());

#endif //JSON_H
