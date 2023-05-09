#version 450

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding = 1) uniform samplerCube sepcular_sampler;
layout(location = 0) in vec3 uvw;

layout(location = 0) out vec4 outColor;
void main(){
    vec3 color = textureLod(sepcular_sampler,uvw,0.0).rgb;
    outColor = vec4(color,1.0);
}