#extension GL_GOOGLE_include_directive : enable

layout(set=2, binding =0) uniform MaterialFactor{
    vec4 basecolor_factor;
    float metallic_factor;
    float roughness_factor;
    bool has_normal_map;
}material;

#include "meshdata.glsl"
//对于gltf 2.0模型使用的metallic-roughness工作流
//需要查看gltf 文档进行核对https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation
float get_metallic(){
    return texture(metallic_roughness_map, fragTexCoord).b*material.metallic_factor;
}
float get_roughness(){
    return texture(metallic_roughness_map, fragTexCoord).g*material.roughness_factor;
}
vec4 get_color_value1(){
    //投影空间中的位置
    vec4 position = ubo.proj*ubo.view*ubo.model*vec4(fragPosition, 1.0);
    //对象空间转换世界空间
    vec3 normal = normalize(mat3(transpose(inverse(ubo.model)))*fragNormal);
    //转置：glsl中矩阵是按列主序存储，需要转置？
    //vec3 tangent = normalize(mat3(transpose(ubo.model))* fragTangent);
    vec3 tangent = normalize(mat3(ubo.model)* fragTangent);
    vec3 bitangent = normalize(cross(normal, tangent));

    // 材质属性
    vec4 albedo = texture(albedo_map, fragTexCoord)*material.basecolor_factor;
    float metallic = get_metallic();
    float roughness = get_roughness();
    //？？TODO:此处不正确，需要修改确认
    vec3 normal_map_value =vec3(1.0);
    if(material.has_normal_map){
        //如果存在法线贴图，此片段的法线向量可以从法线贴图上获取
        normal_map_value = texture(normal_map, fragTexCoord).rgb;
        //将值从[0,1]映射到[-1,1]
        normal_map_value = normal_map_value*2.0-1.0;
    }else{
        //如果没有法线贴图，此着色点的法线需要通过顶点法线自动插值获得
        normal_map_value = normal;

    }
    //法线向量
//    vec3 tangent_world[3];
//    tangent_world[0] = normalize(mat3(ubo.model)*tangent);
//    tangent_world[1] = normalize(mat3(ubo.model)*bitangent);
//    tangent_world[2] = normalize(mat3(ubo.model)*normal);
//    mat3 tangent_world_mat = mat3(tangent_world[0], tangent_world[1], tangent_world[2]);
//    mat3 tangent_world_mat = mat3(normal,tangent,bitangent);

//    vec3 normal_vec = normalize(tangent_world_mat*(normal_map_value*2.0-1.0));

    vec3 normal_vec = normal_map_value;
    //此处表示人眼观察点在（0，0，0）原点处
    vec3 view_dir = normalize(-position.xyz);
    //todo:传入光源方向
    vec3 light_dir = vec3(0, 1.0, 1.0);
    vec3 half_vec = normalize(view_dir +light_dir);

    //BRDF 世界坐标系下的计算
    float n_dot_l = max(0.0, dot(normal_vec, light_dir));
    float n_dot_v = max(0.0, dot(normal_vec, view_dir));
    float n_dot_h = max(0.0, dot(normal_vec, half_vec));
    float v_dot_h = max(0.0, dot(view_dir, half_vec));
    float f = 0.0;
    if (material.metallic_factor>0.0){
        float F0 = 0.04;
        f = F0 + (1.0 - F0) * pow(1.0 - v_dot_h, 5.0);
    }
    float d =0.0;
    if (material.roughness_factor>0.0){
        float roughness_sq = material.roughness_factor*material.roughness_factor;
        float denom = n_dot_h*n_dot_h*(roughness_sq-1.0)+1.0;
        d = roughness_sq/(M_PI*denom*denom);
    }
    float g = 0.0;
    if (material.roughness_factor>0.0){
        float k = (material.roughness_factor + 1.0) * (material.roughness_factor + 1.0) / 8.0;
        float G1 = n_dot_v / (n_dot_v * (1.0 - k) + k);
        float G2 = n_dot_l / (n_dot_l * (1.0 - k) + k);
        g = G1*G2;
    }
    vec3 diffuse_color = albedo.rgb*(1.0-metallic);
    vec3 specular_color = mix(vec3(0.04), albedo.rgb, metallic)*f*g*d;
    vec3 ambient_color = albedo.rgb*0.04;
    vec4 out_color = vec4(diffuse_color+specular_color+ambient_color, albedo.a);
    return out_color;
}