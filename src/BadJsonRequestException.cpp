//
// Created by athor on 24/10/2022.
//

#include "BadJsonRequestException.h"

BadJsonRequestException::BadJsonRequestException(const char *what) : runtime_error(what)
{

}

BadJsonRequestException::BadJsonRequestException(const std::string &what) : runtime_error(what)
{

}
