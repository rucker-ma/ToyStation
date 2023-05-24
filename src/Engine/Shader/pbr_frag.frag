#version 450

#extension GL_GOOGLE_include_directive : enable

#include "metallic_roughness_pbr.glsl"

layout(location = 0) out vec4 outColor;

void main(){
    outColor = shading_color();
    gBuffer0 = vec4(outColor.rgb,gl_FragCoord.z);
    gBuffer1 = vec4(gl_FragCoord.xy,0.0,1.0);
}
