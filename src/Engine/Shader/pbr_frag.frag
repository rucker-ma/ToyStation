#version 450

#extension GL_GOOGLE_include_directive : enable

#include "metallic_roughness_pbr.glsl"

layout(location = 0) out vec4 outColor;

void main(){
    outColor = shading_color();
    gBuffer0 = vec4(gl_FragCoord.x/2400,gl_FragCoord.y/1600,gl_FragCoord.zw);
    gBuffer1 = vec4(projPosition,1.0);
}
