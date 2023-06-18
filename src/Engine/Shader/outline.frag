#version 450

#extension GL_GOOGLE_include_directive : enable

layout(push_constant) uniform OutlineParam{
    vec3 color;
}parm;
layout(location = 0) out vec4 outColor;

void main(){
    outColor = vec4(parm.color,1.0);
}