#version 450

#extension GL_GOOGLE_include_directive : enable

#include "ubo.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragPosition;

void main(){
    gl_Position = ubo.proj*vec4(inPosition,1.0);
    gl_Position.z = 0;
    fragPosition = inTexCoord;
}