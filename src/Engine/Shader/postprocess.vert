#version 450

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding =0) uniform UniformBufferOjbect{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;

layout(location=0) in vec3 inVertex;

void main(){
    vec3 vertex = inVertex + vec3(800,600,0);
    gl_Position = ubo.proj*ubo.view* vec4(vertex,1.0);
}