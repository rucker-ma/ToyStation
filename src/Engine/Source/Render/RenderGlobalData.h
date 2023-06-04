//
// Created by ma on 2023/4/3.
//

#pragma once

#include "RenderResource.h"
#include "RenderContext.h"
#include "Framework/MeshComponent.h"
#include "Framework/TObject.h"
#include "Framework/Level.h"
namespace toystation{

//存储渲染相关全局资源，如vkdevice,vkinstance等
class RenderGlobalData {
public:
    std::shared_ptr<RenderContext> render_context;
    std::shared_ptr<RenderResource> render_resource;
    void AddRenderObject(std::shared_ptr<TObject> obj);
    void LoadMapAsTexture(MapType type, std::string path);
private:
    RHITexture CreateTextureFromKtx(std::string path);
    RHITexture CreateTextureFromOther(std::string path);
    RHITexture CreateTextureFromValue(int width, int height, Vector4 vec);
    void CreateRenderTexture(VkCommandBuffer& cmd,std::shared_ptr<Texture> texture_data,RHITexture& texture, 
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    void CreateRenderTexture(VkCommandBuffer& cmd,
                             std::shared_ptr<Texture> texture_data,RHITexture& texture,
                             VkFormat format,VkSamplerCreateInfo samplerinfo);
};

}
