#version 450

#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding =0) uniform UniformBufferOjbect{
    mat4 model;
    mat4 view;
    mat4 proj;
    bool has_tangent;
    vec3 light_position;
    vec3 light_color;
}ubo;

layout(location=0) in vec3 inPosition; //location
layout(location=1) in vec3 inNormal; //normal
layout(location=2) in vec2 inTexCoord;
layout(location=3) in vec3 inTangent;

layout(location=0) out vec3 fragPosition; //location
layout(location=1) out vec3 fragNormal; //normal
layout(location=2) out vec2 fragTexCoord;
layout(location=3) out vec3 fragTangent;

void main(){
    gl_Position = ubo.proj*ubo.view*ubo.model*vec4(inPosition,1.0);
    fragPosition = inPosition;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
    if(ubo.has_tangent){
        fragTangent = inTangent;
    }else{
        vec3 c1 = cross(inNormal,vec3(0.0,0.0,1.0));
        vec3 c2 = cross(inNormal,vec3(0.0,1.0,0.0));
        if(length(c1)>length(c2)){
            fragTangent = c1;
        }else{
            fragTangent = c2;
        }
        fragTangent = normalize(fragTangent);
    }
}