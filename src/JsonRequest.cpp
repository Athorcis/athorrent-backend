#include "JsonRequest.h"
#include "Utils.h"
#include "BadJsonRequestException.h"

#include <iostream>

using namespace rapidjson;
using namespace std;

JsonRequest::JsonRequest(const string & buffer)
{
    Parse(buffer.c_str());

    if (!IsObject()) {
        throw BadJsonRequestException("root is not an object");
    }

    const Value & action = operator[]("action");

    if (!action.IsString()) {
        throw BadJsonRequestException("action is not a string");
    }

    Value & parameters = operator[]("parameters");

    if (!parameters.IsObject()) {
        throw BadJsonRequestException("parameters is not an object");
    }

    for (Value::ConstMemberIterator it = parameters.MemberBegin(), end = parameters.MemberEnd(); it < end; ++it) {
        if (!it->value.IsString()) {
            throw BadJsonRequestException(string("parameter ") + it->name.GetString() + " is not a string");
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
