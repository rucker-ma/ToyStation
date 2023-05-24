#version 450

#extension GL_GOOGLE_include_directive : enable

layout(location=0) out vec3 uvw;
#include "ubo.glsl"

void main(){

    //Cubemap faces are stored in the order: +X, -X, +Y, -Y, +Z, -Z in a left-handed coordinate system with +Y up and,
    //with the +Z face forward, +X on the on the right
    const vec3 cube_corner_vertex_offsets[8] = vec3[8](vec3(1.0, 1.0, 1.0),
    vec3(1.0, 1.0, -1.0), vec3(1.0, -1.0, -1.0), vec3(1.0, -1.0, 1.0),
    vec3(-1.0, 1.0, 1.0), vec3(-1.0, 1.0, -1.0), vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, 1.0));

    const int cube_triangle_index[36] = int[36](
    0, 1, 2, 2, 3, 0,
    7, 6, 5, 5, 4, 7,
    4, 5, 1,1, 0, 4,
    3, 2, 6,6, 7, 3,
    4, 0, 3, 3, 7, 4,
    1, 5, 6, 6, 2, 1);// x+, x-,y+, y-, z+, z-

    vec3 local_position = cube_corner_vertex_offsets[cube_triangle_index[gl_VertexIndex]];
    vec3 world_position = ubo.camera_position+local_position;
    vec4 clip_position = ubo.proj*ubo.view*vec4(world_position,1.0);
    clip_position.z = clip_position.w*0.99999;
    gl_Position = clip_position;
    uvw = normalize(world_position-ubo.camera_position);
}