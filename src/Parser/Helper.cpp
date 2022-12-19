#include "Helper.h"

PathEnv& PathEnv::Get()
{
    static PathEnv Env;
    return Env;
}
