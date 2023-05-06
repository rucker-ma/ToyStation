#define M_PI 3.14159265359

layout(set=0,binding =0) uniform UniformBufferOjbect{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;

layout(location=0) in vec3 fragPosition;    //location
layout(location=1) in vec3 fragNormal;      //normal
layout(location=2) in vec2 fragTexCoord;    //texcoord
layout(location=3) in vec3 fragTangent;     //tangent

layout(set=1,binding =0) uniform sampler2D albedo_map;
layout(set=1,binding =1) uniform sampler2D occlusion_map;
layout(set=1,binding =2) uniform sampler2D metallic_roughness_map;
layout(set=1,binding =3) uniform sampler2D normal_map;