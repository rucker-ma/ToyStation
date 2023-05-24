#define M_PI 3.14159265359

#include "ubo.glsl"

layout(location=0) in vec3 fragPosition;    //location
layout(location=1) in vec3 fragNormal;      //normal
layout(location=2) in vec2 fragTexCoord;    //texcoord
layout(location=3) in vec3 fragTangent;     //tangent

layout(set=1,binding =0) uniform sampler2D albedo_map;
layout(set=1,binding =1) uniform sampler2D occlusion_map;
layout(set=1,binding =2) uniform sampler2D metallic_roughness_map;
layout(set=1,binding =3) uniform sampler2D normal_map;

float get_metallic(){
    return clamp(texture(metallic_roughness_map, fragTexCoord).b*material.metallic_factor,0.0,1.0);
}
float get_roughness(){
    return clamp(texture(metallic_roughness_map, fragTexCoord).g*material.roughness_factor,0.0,1.0);
}
//--------------------------------------
//Cook-Torrance BRDF
//-------------------------------------

//Trowbridge-Reitz GGX 法线分布函数
//粗糙度不同会会影响光线在表面斑点的大小，根据微平面理论，越粗糙的表面法线方向越不一致，
//入射的光线会被反射向四面八方，也就更少的进入人眼
float distrubution_ggx(vec3 n,vec3 h,float roughness){
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return nom / denom;
}
//微平面由于粗糙度不同，会导致自遮蔽的现象，这个函数就是通过粗糙度和入射角度估算被遮挡的概率
float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
//需要考虑入射和出射的遮挡，只有都不遮挡才会进入人眼
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}
//反射光线对比折射光线的比例，F0：基础反射率
//对于平面，根据观察角度的不同，这个比例也会不同，倾角越大反射现象越明显
//另外对于这一属性更依赖于金属和非金属的材质，金属有接近于1的基础反射率，表示对于金属观察角度的影响较小
//对于非金属，基础反射率比较小，观察角度的影响较大，对于微平面而言，如果出现两种材质的混合，单一的这种计算
//就不正确了，因此引入金属度来表示这种混合性
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnel_schlick_roughness(float costheta,vec3 F0,float roughness){
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - costheta, 5.0);
}