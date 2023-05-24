
layout(set=0,binding =0) uniform UniformBufferOjbect{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camera_position;
    vec3 light_color;
    int has_tangent;
    int has_envmap;
}ubo;