#version 450

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding = 1) uniform sampler2D text_sampler;
layout(location = 0) in vec2 texcoord;

layout(location = 0) out vec4 outColor;
void main(){
    float color = texture(text_sampler,texcoord).r;

    outColor = vec4(color,0.0,0.0,1.0);
}
