#version 450

#extension GL_GOOGLE_include_directive : enable

#include "ubo.glsl"
layout(location=0) in vec3 inPosition; //location
layout(location=1) in vec3 inNormal; //normal
layout(location=2) in vec2 inTexCoord;
layout(location=3) in vec3 inTangent;

layout(location=0) out vec3 fragPosition; //location
layout(location=1) out vec3 fragNormal; //normal
layout(location=2) out vec2 fragTexCoord;
layout(location=3) out vec3 fragTangent;
layout(location=4) out vec3 projPosition;

void main(){
    vec4 proj_position =  ubo.proj*ubo.view*ubo.model*vec4(inPosition,1.0);
    gl_Position = proj_position;
    projPosition = vec3(proj_position)/proj_position.w;
    projPosition.xy = projPosition.xy/2.0 + vec2(0.5);

    fragPosition = vec3(ubo.model* vec4( inPosition,1.0));
    fragNormal = normalize(mat3(transpose(inverse(ubo.model)))*inNormal);;
    fragTexCoord = inTexCoord;
    if(ubo.has_tangent == 1){
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