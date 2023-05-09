#version 450

#extension GL_GOOGLE_include_directive : enable

#include "metallic_roughness_pbr.glsl"

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 gBuffer0;
layout(location = 2) out vec4 gBuffer1;
void main(){
    outColor = get_render_color();
//    outColor = vec4(fragNormal,1.0);
    gBuffer0 =vec4(vec3(get_metallic()),1.0);
    gBuffer1 = vec4(vec3(get_roughness()),1.0);
}
