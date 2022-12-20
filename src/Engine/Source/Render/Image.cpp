#include "Image.h"



namespace TSEngine {
	Image::Image(bool SwapChainImg)
		:image_(nullptr),image_view_(nullptr),image_memory_(nullptr),sampler_(nullptr),memory_size_(0),swap_chain_img_(SwapChainImg)
	{
	}

	Image::~Image()
	{
        if (sampler_)
        {
            vkDestroySampler(VulkanDevice, sampler_, nullptr);
        }
        if (image_view_)
        {
            vkDestroyImageView(VulkanDevice, image_view_, nullptr);
        }
        if (image_&& !swap_chain_img_)
        {
            vkDestroyImage(VulkanDevice, image_, nullptr);
        }
        if (image_memory_)
        {
            vkFreeMemory(VulkanDevice, image_memory_, nullptr);
        }

	}

	const VkImage& Image::GetVkImage()
	{
		return image_;
	}

	const VkImageView& Image::GetVkImageView(VkFormat Format, VkImageAspectFlags AspectFlags)
	{
		if (!image_view_) {
			CreateVkImageView(Format,AspectFlags);
		}
		return image_view_;
	}

    void Image::FreeImageView()
    {
        if (image_view_)
        {
            vkDestroyImageView(VulkanDevice, image_view_, nullptr);
            image_view_ = nullptr;
        }
    }

	const VkSampler& Image::GetImageSampler()
	{
		if (!sampler_) {
			CreateSampler();
		}
		return sampler_;
	}

	const VkDeviceMemory& Image::GetMemory()
	{
		return image_memory_;
	}

	const uint64_t Image::GetMemorySize()
	{
		return memory_size_;
	}

	void Image::CreateVkImageView(VkFormat Format, VkImageAspectFlags AspectFlags)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image_;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = Format;
		createInfo.subresourceRange.aspectMask = AspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		
		auto res = vkCreateImageView(VulkanDevice, &createInfo, nullptr, &image_view_);
		//todo: check res 
	}

	void Image::CreateSampler()
	{
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.anisotropyEnable = VK_TRUE;
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanPhysicsDevice, &properties);
		sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.unnormalizedCoordinates = VK_FALSE;
		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;
		if (vkCreateSampler(VulkanDevice, &sampler_info, nullptr, &sampler_) != VK_SUCCESS)
		{

		}
	}
}
