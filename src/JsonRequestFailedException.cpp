#include "JsonRequestFailedException.h"

using namespace std;

JsonRequestFailedException::JsonRequestFailedException(const char *id)
        : runtime_error(string("json request failed with error : ") + id), m_id(id)
{

}

JsonRequestFailedException::JsonRequestFailedException(const char *id, const char * what)
        : runtime_error(what), m_id(id)
{

}

JsonRequestFailedException::JsonRequestFailedException(const char *id, const string & what)
        : runtime_error(what), m_id(id)
{

}

const string &JsonRequestFailedException::getId() const
{
    return m_id;
}
