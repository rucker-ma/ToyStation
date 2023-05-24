#extension GL_GOOGLE_include_directive : enable

layout(set=0,binding = 1) uniform samplerCube irradiancemap;
layout(set=0,binding = 2) uniform samplerCube radiancemap;
layout(set=0,binding = 3) uniform sampler2D brdfmap;

layout(set=2, binding =0) uniform MaterialFactor{
    vec4 basecolor_factor;
    float metallic_factor;
    float roughness_factor;
    int has_normal_map;
}material;
#include "meshdata.glsl"

layout(location = 1) out vec4 gBuffer0;
layout(location = 2) out vec4 gBuffer1;

//对于gltf 2.0模型使用的metallic-roughness工作流
//需要查看gltf 文档进行核对https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation

vec3 background_shading_color(){

    vec3 world_normal = normalize(fragNormal);
    vec3 view_dir= normalize(ubo.camera_position - fragPosition);
    float roughness = get_roughness();
    vec3 albedo = texture(albedo_map, fragTexCoord).rgb * material.basecolor_factor.rgb;

    vec3 irradiance = textureLod(irradiancemap,world_normal,0.0).rgb;

    vec3 diffuse = irradiance* pow(albedo,vec3(2.2));
    vec3 F0 = vec3(0.04);
    float metallic = get_metallic();
    F0 = mix(F0, albedo, metallic);
    float normal_dot_view = clamp(dot(world_normal,view_dir),0.0,1.0);
    vec3 ks = fresnel_schlick_roughness(normal_dot_view,F0,roughness);
    vec3 kd = (1.0 -ks)*(1-metallic);

    vec3 light_dir = reflect(-view_dir,world_normal);
    const float MAX_REFLECTION_LOD = 6.0;
    vec3 prefilter_color = textureLod(radiancemap,light_dir,MAX_REFLECTION_LOD*roughness).rgb;
    //r: fresnel响应系数
    //g: 偏差值？
    vec2 brdf = texture(brdfmap,vec2(normal_dot_view,roughness)).rg;
    vec3 specular = prefilter_color* (ks*brdf.x+brdf.y);

    float ao = 1.0;
    vec3 ambient = (kd*diffuse + specular)*ao;
    return ambient;
}


vec3 light_shading_color(){
    // 材质属性
    vec4 albedo = texture(albedo_map, fragTexCoord)*material.basecolor_factor;
    float metallic = get_metallic();
    float roughness = get_roughness();
    vec3 world_normal =vec3(1.0);
    if(material.has_normal_map == 1){
        //如果存在法线贴图，此片段的法线向量可以从法线贴图上获取
        //对于从法线贴出上获取的法线，方向是相对于切线空间的，需要将其转换到世界空间的入射光线保持一致
        vec3 local_normal = texture(normal_map, fragTexCoord).rgb;
        //将值从[0,1]映射到[-1,1]
        local_normal = local_normal*2.0-1.0;

        vec3 q1 = dFdx(fragPosition);
        vec3 q2 = dFdy(fragPosition);
        vec2 st1 = dFdx(fragTexCoord);
        vec2 st2 = dFdy(fragTexCoord);
        vec3 t = normalize(q1*st2.t - q2*st1.t);
        vec3 b = -normalize(cross(world_normal,t));
        mat3 tbn = mat3(t,b,fragNormal);

        world_normal = normalize(tbn*local_normal);
    }else{
        //如果没有法线贴图，此着色点的法线需要通过顶点法线自动插值获得
        world_normal = normalize(fragNormal);
    }

    vec3 view_dir= normalize(ubo.camera_position - fragPosition);
    //todo:传入光源方向
    vec3 light_position = vec3(0, -10.0, 5.0);

    vec3 light_dir =normalize(light_position - fragPosition);
    vec3 half_vec = normalize(view_dir +light_dir);
    //0.04 近似表示绝缘体的基础反射率
    vec3 f0 = vec3(0.04);
    f0 = mix(f0,vec3(albedo),metallic);

    float distance = length(light_position - fragPosition);
    float attenuation = 1.0/(distance*distance);

    vec3 radiance = ubo.light_color*attenuation;

    float ndf = distrubution_ggx(world_normal,half_vec,roughness);
    float g = geometry_smith(world_normal,view_dir,light_dir,roughness);

    float v_dot_h = clamp( dot(view_dir, half_vec),0.0,1.0);
    vec3 f = fresnel_schlick(v_dot_h,f0);
    vec3 numerator  = ndf*g*f;
    //4.0: Cook-Torrance specular BRDF 被除系数
    float denominator = 4.0 * max(dot(world_normal, view_dir), 0.0) * max(dot(world_normal, light_dir), 0.0) + 0.0001;
    vec3 specular = numerator  / denominator;
    vec3 kd = vec3(1.0)-f;
    kd=kd*(1.0-metallic);
    float n_dot_l = max(0.0, dot(world_normal, light_dir));
    vec3 lo = (kd * vec3(albedo) / M_PI + specular) * radiance * n_dot_l;

    // ambient lighting
    float ao = 1;
    //0.03: ？？
    //这个的ambient模拟的光源被环境反射产生的光强度
    vec3 ambient = vec3(0.03) * vec3(albedo) * ao;

    return ambient;
}

vec4 shading_color(){
    // vec3 ambient1 = light_shading_color();
    vec3 ambient1 = vec3(0.0);
    vec3 ambient2 = background_shading_color();
    vec3 color = ambient1+ambient2;
    color = color/(color+vec3(1.0));
    color = pow(color,vec3(1.0/2.2));
    return vec4(color,1.0);
}