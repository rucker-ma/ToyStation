#version 450

#extension GL_GOOGLE_include_directive : enable

#include "ubo.glsl"

layout(location=0) in vec3 inPosition;

void main(){
    gl_Position = ubo.proj*ubo.view*ubo.model*vec4(inPosition,1.0);
}