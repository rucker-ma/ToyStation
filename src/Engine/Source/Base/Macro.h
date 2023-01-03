#pragma once

#define TS_API __declspec(dllexport)

#ifdef TS_CPP_EXPORT
#define TS_CPP_API __declspec(dllexport)
#else
#define TS_CPP_API
#endif

#ifdef __cplusplus
#define BEGIN_EXPORT extern "C" {
#define END_EXPORT }
#else
#define BEGIN_EXPORT
#define END_EXPORT
#endif

#define PROPERTY(...)
#define CLASS(...)
#define FUNCTION(...)
#define REFLECTION_BODY(...)

#define STR(x) #x

#define TYPE_PAIR(x) \
    { x, STR(x) }



#include "Logger.h"

// modify for CLASS Macro
#define SINGLETON
// export for csharp use,
#define CSHARP
