layout(set=2,bingding =0) uniform MaterialFactor{
    vec4 basecolor_factor;
    float specular_factor;
    float glonessiness_factor;
}material;

#include "meshdata.glsl"
//gltf 使用的是metallic-roughness 工作流，
//对于specular-roughness工作流，需要参考
//https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_specular
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}



float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 getBRDF(vec3 specularColor, float glossiness, vec3 N, vec3 V, vec3 L) {
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    vec3 F = fresnelSchlick(LdotH, specularColor);
    float D = distributionGGX(N, H, glossiness);
    float G = geometrySmith(N, V, L, glossiness);

    return (F * D * G) / (4.0 * NdotL * NdotV);
}


vec4 get_color(){
    
    vec3 albedo = texture(baseColorMap, fragTexCoord).rgb;
    vec4 sg = texture(specularGlossinessMap, fragTexCoord);

    vec3 specularColor = sg.rgb;
    float glossiness = sg.a * sg.a;

    vec3 worldPos = vec3(ubo.model * vec4(fragPosition, 1.0));
    vec3 worldNormal = normalize(mat3(ubo.model) * fragNormal);

    vec3 viewPos = vec3(ubo.view * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 viewDir = normalize(worldPos - viewPos);

    vec3 lightPos = vec3(0.0, 5.0, 0.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 lightDir = normalize(lightPos - worldPos);

    vec3 color = vec3(0.0);
    vec3 L = -lightDir;
    vec3 V = -viewDir;
    vec3 N = worldNormal;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, specularColor, glossiness);

    vec3 brdf = getBRDF(F0, glossiness, N, V, L);
    vec3 diffuse = (1.0 - F0) * albedo;
    vec3 specular = brdf * (lightColor * max(dot(N, L), 0.0));

    color = diffuse + specular;

    return vec4(color, 1.0);
}