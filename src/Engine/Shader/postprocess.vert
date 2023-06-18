#version 450

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding =0) uniform UniformBufferOjbect{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 origin_position;
    vec3 light_color;
    int has_tangent;
    int has_envmap;
}ubo;

layout(location=0) in vec3 inVertex;

void main(){
    //保持坐标组件维持一定的屏幕空间大小，当camera远离object时需要方法操作
    float scale = ubo.origin_position.z;

    vec4 position = ubo.proj*ubo.view* ubo.model* vec4(inVertex,1.0);
    float w = position.w;

    position = position/w;
    //先平移到原点放大
    position.xy = (position.xy-ubo.origin_position.xy)*scale;
    //放大后移动到之前的位置
    position.xy = position.xy+ubo.origin_position.xy;

    gl_Position = position*w;

}