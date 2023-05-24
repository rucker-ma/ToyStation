#version 450

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding =1) uniform UniformBufferOjbect{
    vec2 size;
    vec3 color;
}frag_uniform;

layout(set=0,binding = 2) uniform sampler2D rendercolor;

layout(location = 0) out vec4 outColor;

void main(){
    vec2 coord = vec2((gl_FragCoord.x-0.5)/frag_uniform.size.x,
    (gl_FragCoord.y-0.5)/frag_uniform.size.y);
    vec4 value = texture(rendercolor,coord);
    vec3 pre_color = value.rgb;
    float pre_depth = value.z;

    if(pre_color == vec3(0)){
        outColor = vec4(1.0);
    }else if(pre_depth<gl_FragCoord.z){
//        outColor = vec4(vec3(1.0),0.4);
        outColor = vec4(pre_color*0.6 +vec3(1.0)*0.4,1.0);
    }else{
        outColor = vec4(0,0,1.0,1);
    }
}
