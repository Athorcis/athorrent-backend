#include "JsonRequest.h"
#include "Utils.h"

#include <iostream>

using namespace rapidjson;
using namespace std;

JsonRequest::JsonRequest(const string & buffer)
{
    Parse(buffer.c_str());

    if (!IsObject()) {
        throw "JsonRequest::JsonRequest: root is not an object";
    }

    Value & action = operator[]("action");

    if (!action.IsString()) {
        throw "JsonRequest::JsonRequest: action is not a string";
    }

    Value & parameters = operator[]("parameters");

    if (!parameters.IsObject()) {
        throw "JsonRequest::JsonRequest: parameters is not an object";
    }

    for (Value::ConstMemberIterator it = parameters.MemberBegin(), end = parameters.MemberEnd(); it < end; ++it) {
        if (!it->value.IsString()) {
            throw string("JsonRequest::JsonRequest: parameter ") + it->name.GetString() + " is not a string";
        }
    }
}

string JsonRequest::getAction() const
{
    return operator[]("action").GetString();
}

bool JsonRequest::hasParameter(const string & key) const
{
    return operator[]("parameters").HasMember(key);
}

string JsonRequest::getParameter(const string & key) const
{
    return operator[]("parameters")[key].GetString();
}
