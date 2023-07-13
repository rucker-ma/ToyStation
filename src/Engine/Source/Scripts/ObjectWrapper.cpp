//
// Created by ma on 2023/7/13.
//
#include "ObjectWrapper.h"


namespace toystation{


void ObjectWrapper::WrapV8Global(v8::Isolate* isolate,v8::Local<v8::Context> context){
    WrapConsole(isolate,context);
}

void ObjectWrapper::WrapConsole(v8::Isolate* isolate,v8::Local<v8::Context> context){

}

}