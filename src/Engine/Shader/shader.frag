#version 450

#extension GL_GOOGLE_include_directive : enable

layout(location =0) in vec3 fragNormal;
layout(location =1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set=0,binding =0) uniform UniformBufferOjbect{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;

layout(set=1,binding =0) uniform sampler2D albedo;
layout(set=1,binding =1) uniform sampler2D occlusion;
layout(set=1,binding =2) uniform sampler2D metallic_roughness;
layout(set=1,binding =3) uniform sampler2D normal;

void main(){
    outColor = texture(albedo,fragTexCoord);
}
