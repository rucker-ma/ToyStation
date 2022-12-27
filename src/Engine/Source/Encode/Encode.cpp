#include "Encode.h"

#include <vk_video/vulkan_video_codecs_common.h>

#include <iostream>

namespace toystation {
static const uint32_t kH264MbSizeAlignment = 16;
template <typename SizeType>
SizeType AlignSize(SizeType size, SizeType alignment) {
    assert((alignment & (alignment - 1)) == 0);
    return (size + alignment - 1) & ~(alignment - 1);
}

IntraFrameInfo::IntraFrameInfo(uint32_t frame_count, uint32_t width,
                               uint32_t height,
                               StdVideoH264SequenceParameterSet sps,
                               StdVideoH264PictureParameterSet pps,
                               bool is_hdr) {
    const uint32_t max_pic_order_cnt_lsb =
        1 << (sps.log2_max_pic_order_cnt_lsb_minus4 + 4);

    slice_header_flags_.num_ref_idx_active_override_flag = 0;
    slice_header_flags_.no_output_of_prior_pics_flag = 0;
    slice_header_flags_.adaptive_ref_pic_marking_mode_flag = 0;
    slice_header_flags_.no_prior_references_available_flag = 0;

    slice_header_.flags = slice_header_flags_;
    slice_header_.slice_type = STD_VIDEO_H264_SLICE_TYPE_I;
    slice_header_.idr_pic_id = 0;
    slice_header_.num_ref_idx_l0_active_minus1 = 0;
    slice_header_.num_ref_idx_l1_active_minus1 = 0;
    slice_header_.cabac_init_idc = (StdVideoH264CabacInitIdc)0;
    slice_header_.disable_deblocking_filter_idc =
        (StdVideoH264DisableDeblockingFilterIdc)0;
    slice_header_.slice_alpha_c0_offset_div2 = 0;
    slice_header_.slice_beta_offset_div2 = 0;

    uint32_t pic_width_mbs = sps.pic_width_in_mbs_minus1 + 1;
    uint32_t pic_height_mbs = sps.pic_height_in_map_units_minus1 + 1;
    uint32_t pic_size_mbs = pic_width_mbs * pic_height_mbs;

    slice_info_.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_NALU_SLICE_INFO_EXT;
    slice_info_.pNext = nullptr;
    slice_info_.pSliceHeaderStd = &slice_header_;
    slice_info_.mbCount = pic_size_mbs;

    if (is_hdr) {
        picture_info_flags_.idr_flag = 1;
        picture_info_flags_.is_reference_flag = 1;
    }

    picture_info_.flags = picture_info_flags_;
    picture_info_.seq_parameter_set_id = 0;
    picture_info_.pic_parameter_set_id = pps.pic_parameter_set_id;
    picture_info_.pictureType = STD_VIDEO_H264_PICTURE_TYPE_I;

    // frame_num is incremented for each reference frame transmitted.
    // In our case, only the first frame (which is IDR) is a reference
    // frame with frame_num == 0, and all others have frame_num == 1.
    picture_info_.frame_num = is_hdr ? 0 : 1;

    // POC is incremented by 2 for each coded frame.
    picture_info_.PicOrderCnt = (frame_count * 2) % max_pic_order_cnt_lsb;

    h264_frame_info_.sType =
        VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_VCL_FRAME_INFO_EXT;
    h264_frame_info_.pNext = nullptr;
    h264_frame_info_.naluSliceEntryCount = 1;
    h264_frame_info_.pNaluSliceEntries = &slice_info_;
    h264_frame_info_.pCurrentPictureInfo = &picture_info_;
}
void EncodeContext::Init(EncodeConfig* config) {
    current_config_ = config;
    CreateCommandPool();
    InitVideoProfile();
    GetSupportedFormat();
    InitDeviceAllocator();
    CreateSession();
    InitFramePool();
    CreateSpsPps();
    VkCommandBuffer cmd_buffer = command_pool_.CreateCommandBuffer();
    InitRateControl(cmd_buffer, 30);
    picture_buffer_.PrepareInputImages(cmd_buffer);
    picture_buffer_.PrepareReferenceImages(cmd_buffer);
    command_pool_.SubmitAndWait(cmd_buffer);
}
void EncodeContext::CreateCommandPool() {
    VulkanQueueFamily queue_indices =
        VulkanContext::Instance().FindQueueFamilies(VulkanPhysicsDevice);
    if (queue_indices.encode_family.has_value()) {
        command_pool_.Init(VulkanDevice, queue_indices.encode_family.value(),
                           VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
                               VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    } else {
        LogError("Current Vulkan Device hasn't Encodec Capability");
    }
}
void EncodeContext::InitVideoProfile() {
    VkVideoCodecOperationFlagBitsKHR video_codec_operation =
        VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT;
    VkVideoChromaSubsamplingFlagsKHR chroma_subsamplimg =
        VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;  // yuv420
    VkVideoComponentBitDepthFlagsKHR luma_bitdepth =
        VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    VkVideoComponentBitDepthFlagsKHR chroma_bitdepth =
        VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    uint32_t video_h26x_profile_idc =
        STD_VIDEO_H264_PROFILE_IDC_MAIN;  // main profile

    video_profile_.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR;
    video_profile_.pNext = nullptr;
    video_profile_.videoCodecOperation = video_codec_operation;
    video_profile_.chromaSubsampling = chroma_subsamplimg;
    video_profile_.lumaBitDepth = luma_bitdepth;
    video_profile_.chromaBitDepth = chroma_bitdepth;

    if (video_codec_operation == VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT) {
        encode_h264_profiles_request_.sType =
            VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_EXT;
        encode_h264_profiles_request_.pNext = nullptr;
        encode_h264_profiles_request_.stdProfileIdc =
            (video_h26x_profile_idc == 0)
                ? STD_VIDEO_H264_PROFILE_IDC_INVALID
                : (StdVideoH264ProfileIdc)video_h26x_profile_idc;
        video_profile_.pNext = &encode_h264_profiles_request_;
    }
    // h265编码当前不支持
    //  else if (video_codec_operation ==
    //             VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_EXT) {
    //      encode_h265_profiles_request_.sType =
    //          VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_EXT;
    //      encode_h265_profiles_request_.pNext = nullptr;
    //      encode_h265_profiles_request_.stdProfileIdc =
    //          (video_h26x_profile_idc == 0)
    //              ? STD_VIDEO_H265_PROFILE_IDC_INVALID
    //              :
    //              static_cast<StdVideoH265ProfileIdc>(video_h26x_profile_idc);
    //      video_profile_.pNext = &encode_h265_profiles_request_;
    //  }
    else {
        LogDebug("cannot support codec");
    }
}
void EncodeContext::GetSupportedFormat() {
    std::vector<VkFormat> supported_reconstructued_pictures_format;
    std::vector<VkFormat> supported_input_formats;
    GetVideoFormat(VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
                   supported_input_formats);
    GetVideoFormat(VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
                   supported_reconstructued_pictures_format);

    VkVideoCapabilitiesKHR video_capabilities;
    ZeroVKStruct(video_capabilities, VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR);

    VkVideoEncodeCapabilitiesKHR video_encode_capabilities;
    ZeroVKStruct(video_encode_capabilities,
                 VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR);

    VkVideoEncodeH264CapabilitiesEXT h264_capabilities;
    ZeroVKStruct(h264_capabilities,
                 VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_EXT);
    video_capabilities.pNext = &video_encode_capabilities;
    video_encode_capabilities.pNext = &h264_capabilities;

    VkResult result = vkGetPhysicalDeviceVideoCapabilitiesKHR(
        VulkanPhysicsDevice, &video_profile_, &video_capabilities);

    if (video_profile_.videoCodecOperation ==
        VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT) {
        if (strncmp(
                (const char*)video_capabilities.stdHeaderVersion.extensionName,
                VK_STD_VULKAN_VIDEO_CODEC_H264_ENCODE_EXTENSION_NAME,
                sizeof(video_capabilities.stdHeaderVersion.extensionName) -
                    1U) != 0 ||
            (video_capabilities.stdHeaderVersion.specVersion !=
             VK_STD_VULKAN_VIDEO_CODEC_H264_ENCODE_SPEC_VERSION)) {
            LogFatal("Unsupported h.264 STD version");
        }
    } else {
        LogFatal("Unsupported codec");
    }
    // bool isSemiPlanar = chromaSubsampling !=
    // VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR;
    bool semi_planar = true;
    // image_format_ = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;

    image_format_ =
        CodecGetVkFormat(VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
                         VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR, true);

    if (supported_input_formats[0] != image_format_) {
        LogError("failed to get codec vkformat");
    }
}

bool EncodeContext::GetVideoFormat(VkImageUsageFlags image_usages,
                                   std::vector<VkFormat>& formats) {
    const VkVideoProfileListInfoKHR video_profiles = {
        VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR, nullptr, 1,
        &video_profile_};
    const VkPhysicalDeviceVideoFormatInfoKHR video_format_info = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR,
        &video_profiles, image_usages};

    uint32_t supported_format_count = 0;
    VkResult result = vkGetPhysicalDeviceVideoFormatPropertiesKHR(
        VulkanPhysicsDevice, &video_format_info, &supported_format_count,
        nullptr);

    std::vector<VkVideoFormatPropertiesKHR> supported_formats{
        supported_format_count};
    for (auto&& format : supported_formats) {
        ZeroVKStruct(format, VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR);
        // memset(&format, 0, sizeof(format));
        // format.sType = VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR;
    }
    result = vkGetPhysicalDeviceVideoFormatPropertiesKHR(
        VulkanPhysicsDevice, &video_format_info, &supported_format_count,
        supported_formats.data());

    for (auto&& format : supported_formats) {
        LogInfo("h264 encode formats: " + std::to_string(format.format));
    }

    for (auto&& supported_format : supported_formats) {
        formats.push_back(supported_format.format);
    }
    return !formats.empty();
}
void EncodeContext::InitDeviceAllocator() {
    mem_alloc_.Init(VulkanDevice, VulkanPhysicsDevice);
    // max_coded_extent_ = {width,height};
    // max_reeferent_pictures_slots_count_ = 16;

    max_reeferent_pictures_slots_count_ = 16;
    max_coded_extent_.width = current_config_->width;
    max_coded_extent_.height = current_config_->height;
}
void EncodeContext::CreateSession() {
    VkExtent2D coded_extent = {};
    static const VkExtensionProperties kH264StdExtensionVersion{
        VK_STD_VULKAN_VIDEO_CODEC_H264_ENCODE_EXTENSION_NAME,
        VK_STD_VULKAN_VIDEO_CODEC_H264_ENCODE_SPEC_VERSION};
    static const VkExtensionProperties kH265StdExtensionVersion{
        VK_STD_VULKAN_VIDEO_CODEC_H265_ENCODE_EXTENSION_NAME,
        VK_STD_VULKAN_VIDEO_CODEC_H265_ENCODE_SPEC_VERSION};

    VkVideoSessionCreateInfoKHR create_info = {
        VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR};
    create_info.pVideoProfile = &video_profile_;

    VulkanQueueFamily queue_indices =
        VulkanContext::Instance().FindQueueFamilies(VulkanPhysicsDevice);

    create_info.queueFamilyIndex = queue_indices.encode_family.value();
    create_info.pictureFormat = image_format_;
    create_info.maxCodedExtent = max_coded_extent_;
    create_info.maxDpbSlots = max_reeferent_pictures_slots_count_;
    create_info.maxActiveReferencePictures =
        max_reeferent_pictures_slots_count_;
    create_info.referencePictureFormat = image_format_;
    create_info.pStdHeaderVersion = &kH264StdExtensionVersion;
    VkResult result = vkCreateVideoSessionKHR(VulkanDevice, &create_info,
                                              nullptr, &video_session_);
    // const uint32_t max_memreqs = 8;
    uint32_t video_session_memory_requirements_count = 0;

    std::vector<VkVideoSessionMemoryRequirementsKHR>
        encode_session_memory_requirements;

    result = vkGetVideoSessionMemoryRequirementsKHR(
        VulkanDevice, video_session_, &video_session_memory_requirements_count,
        nullptr);

    encode_session_memory_requirements.resize(
        video_session_memory_requirements_count);
    for (auto&& reqs : encode_session_memory_requirements) {
        ZeroVKStruct(reqs,
                     VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR);
    }

    result = vkGetVideoSessionMemoryRequirementsKHR(
        VulkanDevice, video_session_, &video_session_memory_requirements_count,
        encode_session_memory_requirements.data());

    std::vector<VkBindVideoSessionMemoryInfoKHR> encode_session_bind_memory;

    encode_session_bind_memory.resize(video_session_memory_requirements_count);
    auto bind_info_iter = encode_session_bind_memory.begin();

    for (auto&& reqs : encode_session_memory_requirements) {
        if (reqs.memoryRequirements.memoryTypeBits == 8) {
            reqs.memoryRequirements.memoryTypeBits = 2;
        }
        MemoryAllocateInfo alloc_info(reqs.memoryRequirements);
        MemHandle handle = mem_alloc_.AllocMemory(alloc_info);
        if (!handle) {
            LogError("could not allocated buffer");
        }

        VkMemoryAllocator::MemoryInfo mem_info =
            mem_alloc_.GetMemoryInfo(handle);

        (*bind_info_iter).sType =
            VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR;
        (*bind_info_iter).pNext = nullptr;
        (*bind_info_iter).memory = mem_info.memory;
        (*bind_info_iter).memoryBindIndex = reqs.memoryBindIndex;
        (*bind_info_iter).memoryOffset = mem_info.offset;
        (*bind_info_iter).memorySize = mem_info.size;
        bind_info_iter++;
        // encode_session_bind_memory.push_back(bind_memory_info);
    }
    result = vkBindVideoSessionMemoryKHR(VulkanDevice, video_session_,
                                         encode_session_bind_memory.size(),
                                         encode_session_bind_memory.data());
}

void EncodeContext::InitFramePool() {
    resource_alloc_.Init(VulkanDevice, VulkanPhysicsDevice);
    current_config_->codec_block_alignment = kH264MbSizeAlignment;
    current_config_->aligned_width = AlignSize(
        current_config_->width, current_config_->codec_block_alignment);
    current_config_->aligned_height = AlignSize(
        current_config_->height, current_config_->codec_block_alignment);
    current_config_->luma_plane_size =
        current_config_->aligned_width * current_config_->aligned_height;
    current_config_->chroma_plane_size =
        ((current_config_->aligned_width + 1) / 2) *
        ((current_config_->aligned_height + 1) / 2);
    current_config_->full_image_size =
        current_config_->luma_plane_size + current_config_->chroma_plane_size;

    picture_buffer_.InitFramePool(
        VulkanDevice, &video_profile_, current_config_->codec_block_alignment,
        image_format_, current_config_->aligned_width,
        current_config_->aligned_height, current_config_->full_image_size,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
        &resource_alloc_, &command_pool_, 4);

    picture_buffer_.InitReferenceFramePool(
        current_config_->codec_block_alignment, image_format_,
        &resource_alloc_);
}
void EncodeContext::CreateSpsPps() {
    StdVideoH264SequenceParameterSetVui* vui = nullptr;

    StdVideoH264SpsFlags sps_flags = {};
    sps_flags.direct_8x8_inference_flag = 1;
    sps_flags.frame_mbs_only_flag = 1;
    sps_flags.vui_parameters_present_flag = (vui == nullptr) ? 0 : 1;

    // TODO:set encode width and height
    // uint32_t width = 0;
    // uint32_t height = 0;
    // const uint32_t aligned_width = AlignSize(width, kH264MbSizeAlignment);
    // const uint32_t aligned_height = AlignSize(height, kH264MbSizeAlignment);

    StdVideoH264SequenceParameterSet sps = {};
    sps.profile_idc = STD_VIDEO_H264_PROFILE_IDC_HIGH;
    sps.level_idc = STD_VIDEO_H264_LEVEL_IDC_4_1;
    sps.seq_parameter_set_id = 0;
    sps.chroma_format_idc = STD_VIDEO_H264_CHROMA_FORMAT_IDC_420;
    sps.bit_depth_luma_minus8 = 0;
    sps.bit_depth_chroma_minus8 = 0;
    sps.log2_max_frame_num_minus4 = 0;
    sps.pic_order_cnt_type = STD_VIDEO_H264_POC_TYPE_0;
    sps.max_num_ref_frames = 1;
    sps.pic_width_in_mbs_minus1 =
        current_config_->aligned_width / kH264MbSizeAlignment - 1;
    sps.pic_height_in_map_units_minus1 =
        current_config_->aligned_height / kH264MbSizeAlignment - 1;
    sps.flags = sps_flags;
    sps.pSequenceParameterSetVui = vui;
    sps.frame_crop_right_offset =
        current_config_->aligned_width - current_config_->width;
    sps.frame_crop_bottom_offset =
        current_config_->aligned_height - current_config_->height;

    // This allows for picture order count values in the range [0, 255].
    sps.log2_max_pic_order_cnt_lsb_minus4 = 4U;

    if (sps.frame_crop_right_offset || sps.frame_crop_bottom_offset) {
        sps.flags.frame_cropping_flag = true;

        if (sps.chroma_format_idc == STD_VIDEO_H264_CHROMA_FORMAT_IDC_420) {
            sps.frame_crop_right_offset >>= 1;
            sps.frame_crop_bottom_offset >>= 1;
        }
    }

    StdVideoH264PpsFlags pps_flags = {};
    pps_flags.transform_8x8_mode_flag = 1;
    pps_flags.constrained_intra_pred_flag = 0;
    pps_flags.deblocking_filter_control_present_flag = 1;
    pps_flags.entropy_coding_mode_flag = 1;

    StdVideoH264PictureParameterSet pps = {};
    pps.seq_parameter_set_id = 0;
    pps.pic_parameter_set_id = 0;
    pps.num_ref_idx_l0_default_active_minus1 = 0;
    pps.flags = pps_flags;

    VkVideoEncodeH264SessionParametersAddInfoEXT
        encode_h264_session_par_addinfo;

    encode_h264_session_par_addinfo.sType =
        VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_EXT;
    encode_h264_session_par_addinfo.pNext = nullptr;
    encode_h264_session_par_addinfo.stdSPSCount = 1;
    encode_h264_session_par_addinfo.pStdSPSs = &sps;
    encode_h264_session_par_addinfo.stdPPSCount = 1;
    encode_h264_session_par_addinfo.pStdPPSs = &pps;

    VkVideoEncodeH264SessionParametersCreateInfoEXT
        encode_h264_session_par_create_info;
    encode_h264_session_par_create_info.sType =
        VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_EXT;
    encode_h264_session_par_create_info.pNext = nullptr;
    encode_h264_session_par_create_info.maxStdSPSCount = 1;
    encode_h264_session_par_create_info.maxStdPPSCount = 1;
    encode_h264_session_par_create_info.pParametersAddInfo =
        &encode_h264_session_par_addinfo;

    VkVideoSessionParametersCreateInfoKHR encode_session_create_info;
    encode_session_create_info.sType =
        VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR;
    encode_session_create_info.pNext = &encode_h264_session_par_create_info;
    encode_session_create_info.videoSessionParametersTemplate = nullptr;
    encode_session_create_info.videoSession = video_session_;

    vkCreateVideoSessionParametersKHR(VulkanDevice, &encode_session_create_info,
                                      nullptr, &encode_session_parameters_);
    // TODO
    //  create command buffer
    //  set image layout
    //  submit and wait
}
void EncodeContext::InitRateControl(VkCommandBuffer cmd_buffer,
                                    int32_t encode_qp) {
    VkVideoBeginCodingInfoKHR encode_begin_info = {
        VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR};
    encode_begin_info.videoSession = video_session_;
    encode_begin_info.videoSessionParameters = encode_session_parameters_;

    VkVideoEncodeH264FrameSizeEXT encode_h264_framesize;
    encode_h264_framesize.frameISize = 0;

    VkVideoEncodeH264QpEXT encode_h264_qp;
    encode_h264_qp.qpI = encode_qp;

    VkVideoEncodeH264RateControlLayerInfoEXT
        encode_h264_ratecontrol_layer_info = {
            VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_EXT};
    encode_h264_ratecontrol_layer_info.useInitialRcQp = VK_TRUE;
    encode_h264_ratecontrol_layer_info.initialRcQp = encode_h264_qp;
    encode_h264_ratecontrol_layer_info.useMinQp = VK_TRUE;
    encode_h264_ratecontrol_layer_info.minQp = encode_h264_qp;
    encode_h264_ratecontrol_layer_info.useMaxQp = VK_TRUE;
    encode_h264_ratecontrol_layer_info.maxQp = encode_h264_qp;
    encode_h264_ratecontrol_layer_info.useMaxFrameSize = VK_TRUE;
    encode_h264_ratecontrol_layer_info.maxFrameSize = encode_h264_framesize;

    VkVideoEncodeRateControlLayerInfoKHR encode_ratecontrol_layer_info = {
        VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR};
    encode_ratecontrol_layer_info.pNext = &encode_h264_ratecontrol_layer_info;

    VkVideoCodingControlInfoKHR coding_control_info = {
        VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR};
    coding_control_info.flags = VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR;
    coding_control_info.pNext = &encode_ratecontrol_layer_info;

    VkVideoEndCodingInfoKHR encode_end_info = {
        VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR};

    // Reset the video session before first use and apply QP values.
    vkCmdBeginVideoCodingKHR(cmd_buffer, &encode_begin_info);
    vkCmdControlVideoCodingKHR(cmd_buffer, &coding_control_info);
    vkCmdEndVideoCodingKHR(cmd_buffer, &encode_end_info);
}

void EncodeContext::LoadFrame(EncodeConfig* config, uint32_t framecount,
                              uint32_t current_framebuffer_idx) {
    EncodeFrameData* current_frame_data =
        picture_buffer_.GetEncodeFrameData(current_framebuffer_idx);
    VkImage input_image = current_frame_data->picture_.image_.image;
    Buffer input_staging_buffer = current_frame_data->input_staging_buffer_;
    uint8_t* staging_buffer =
        reinterpret_cast<uint8_t*>(resource_alloc_.Map(input_staging_buffer));
    // TODO:copy yuv data to staging_buffer
    resource_alloc_.UnMap(input_staging_buffer);
}

// 4. begin command buffer
// 5. create SPS and PPS
// 6. create encode session parameters
// 7. begin video coding
// 8. if frame = 0 -- encode non vcl data
// 9. encode vcl data
// 10. end video encoding
void EncodeContext::EncodeFrame(EncodeConfig* config, uint32_t frame_count,
                                bool non_vcl,
                                uint32_t current_framebuffer_idx) {
    VkResult result = VK_SUCCESS;

    // GOP structure config all intra:
    // only using 1 input frame (I) - slot 0
    // only using 1 reference frame - slot 0
    // update POC
    picture_buffer_.AddRefPicture(current_framebuffer_idx,
                                  static_cast<int8_t>(current_framebuffer_idx),
                                  frame_count);

    EncodeFrameData* current_encode_framedata =
        picture_buffer_.GetEncodeFrameData(current_framebuffer_idx);
    VkBuffer out_bitstream =
        current_encode_framedata->out_bistream_buffer_.buffer;
    VkCommandBuffer cmd_buffer =
        current_encode_framedata->encode_command_buffer_;

    // Begin command buffer
    VkCommandBufferBeginInfo begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd_buffer, &begin_info);

    picture_buffer_.CopyToVkImage(current_framebuffer_idx, 0, cmd_buffer);

    // Begin video coding

    VkQueryPool query_pool = picture_buffer_.GetQueryPool();
    // VCL: video coding layer
    //  query slot id for VCL                   - slots [0, 1, ...
    //  INPUT_FRAME_BUFFER_SIZE-1] query slot id for correspondent non VCL -
    //  slots [0+INPUT_FRAME_BUFFER_SIZE,  1+INPUT_FRAME_BUFFER_SIZE, ...
    //  INPUT_FRAME_BUFFER_SIZE+INPUT_FRAME_BUFFER_SIZE-1]
    uint32_t query_slotid_vcl = current_framebuffer_idx;
    uint32_t query_slotid_novcl = current_framebuffer_idx + 16;

    VkVideoBeginCodingInfoKHR encode_begin_info = {
        VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR};
    encode_begin_info.sType = VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR;
    encode_begin_info.videoSession = video_session_;
    ;
    encode_begin_info.videoSessionParameters = encode_session_parameters_;
    encode_begin_info.referenceSlotCount = 0;
    encode_begin_info.pReferenceSlots = nullptr;

    vkCmdBeginVideoCodingKHR(cmd_buffer, &encode_begin_info);

    uint32_t bitstream_offset = 0;  // necessary non zero value for first frame
    if (non_vcl) {
        // Encode Non VCL data - SPS and PPS
        EncodeInfoNonVcl encode_info_nonvcl(
            &video_session_parameters_.sequence_parameter_set,
            &video_session_parameters_.picture_parameter_set, &out_bitstream);
        VkVideoEncodeInfoKHR* video_encode_info_nonvcl =
            encode_info_nonvcl.GetVideoEncodeInfo();
        vkCmdResetQueryPool(cmd_buffer, query_pool, query_slotid_novcl, 1);
        vkCmdBeginQuery(cmd_buffer, query_pool, query_slotid_novcl,
                        VkQueryControlFlags());
        vkCmdEncodeVideoKHR(cmd_buffer, video_encode_info_nonvcl);
        vkCmdEndQuery(cmd_buffer, query_pool, query_slotid_novcl);
        bitstream_offset = 4096;  // use 4k for first frame and then update
                                  // with size of last frame
    }
    // Encode Frame
    // encode info for vkCmdEncodeVideoKHR
    IntraFrameInfo intra_frame_info(
        frame_count, config->width, config->height,
        video_session_parameters_.sequence_parameter_set,
        video_session_parameters_.picture_parameter_set, frame_count == 0);
    VkVideoEncodeH264VclFrameInfoEXT* encode_h264_frame_info =
        intra_frame_info.GetEncodeH264FrameInfo();

    VkVideoPictureResourceInfoKHR input_pic_resource = {
        VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR};
    VkVideoPictureResourceInfoKHR dpb_pic_resource = {
        VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR};
    picture_buffer_.GetFrameResourcesByIndex(current_framebuffer_idx,
                                             &input_pic_resource);
    picture_buffer_.GetReferenceFrameResourcesByIndex(current_framebuffer_idx,
                                                      &dpb_pic_resource);

    EncodeInfoVcl encode_info_vcl(&out_bitstream, bitstream_offset,
                                  encode_h264_frame_info, &input_pic_resource,
                                  &dpb_pic_resource);
    VkVideoEncodeInfoKHR* video_encode_info_vcl =
        encode_info_vcl.GetVideoEncodeInfo();

    vkCmdResetQueryPool(cmd_buffer, query_pool, query_slotid_vcl, 1);
    vkCmdBeginQuery(cmd_buffer, query_pool, query_slotid_vcl,
                    VkQueryControlFlags());
    vkCmdEncodeVideoKHR(cmd_buffer, video_encode_info_vcl);
    vkCmdEndQuery(cmd_buffer, query_pool, query_slotid_vcl);

    VkVideoEndCodingInfoKHR encode_end_info = {
        VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR};
    vkCmdEndVideoCodingKHR(cmd_buffer, &encode_end_info);
    vkEndCommandBuffer(cmd_buffer);

    // reset ref pic
    picture_buffer_.RemoveRefPicture(current_framebuffer_idx);
}
int32_t EncodeContext::BatchSubmit(uint32_t first_framebuffer_idx,
                                   uint32_t frames_in_batch) {
    if (!(frames_in_batch > 0)) {
        return 0;
    }
    const uint32_t max_frames_in_batch = 8;
    assert(frames_in_batch <= max_frames_in_batch);
    VkCommandBuffer cmd_buffer[max_frames_in_batch];

    for (uint32_t cmd_buf_idx = 0; cmd_buf_idx < frames_in_batch;
         cmd_buf_idx++) {
        EncodeFrameData* current_encode_frame_data =
            picture_buffer_.GetEncodeFrameData(first_framebuffer_idx +
                                               cmd_buf_idx);
        cmd_buffer[cmd_buf_idx] =
            current_encode_frame_data->encode_command_buffer_;
        current_encode_frame_data->frame_submitted = true;
    }

    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = frames_in_batch;
    submit_info.pCommandBuffers = static_cast<VkCommandBuffer*>(cmd_buffer);
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;

    VkResult result = vkQueueSubmit(queue_, 1, &submit_info, VK_NULL_HANDLE);

    if (result == VK_SUCCESS) {
        return frames_in_batch;
    }

    return -1;
}
int32_t EncodeContext::GetEncodedData(EncodeConfig* config, bool non_vcl,
                                      uint32_t current_framebuffer_idx) {
    VkResult result = VK_SUCCESS;

    EncodeFrameData* current_encode_frame_data =
        picture_buffer_.GetEncodeFrameData(current_framebuffer_idx);
    if (!current_encode_frame_data->frame_submitted) {
        return 0;
    }

    Buffer out_bitstream_buffer =
        current_encode_frame_data->out_bistream_buffer_;

    // get output results
    struct VideoEncodeStatus {
        uint32_t bitstream_start_offset;
        uint32_t bitstream_size;
        VkQueryResultStatusKHR status;
    };
    VideoEncodeStatus encode_result[2];  // 2nd slot is non vcl data
    memset(&encode_result, 0, sizeof(encode_result));

    int8_t* data =
        static_cast<int8_t*>(resource_alloc_.Map(out_bitstream_buffer));

    VkQueryPool query_pool = picture_buffer_.GetQueryPool();

    uint32_t bitstream_offset = 0;  // necessary non zero value for first frame
    if (non_vcl) {
        // only on frame 0
        bitstream_offset = 4096;
        uint32_t query_slotid_nonvcl = current_framebuffer_idx + 16;
        result = vkGetQueryPoolResults(
            VulkanDevice, query_pool, query_slotid_nonvcl, 1,
            sizeof(VideoEncodeStatus), &encode_result[1],
            sizeof(VideoEncodeStatus),
            VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WAIT_BIT);
        if (result != VK_SUCCESS) {
            LogError(
                "\nRetrieveData Error: Failed to get non vcl query pool "
                "results.\n");
            return -1;
        }
        // fwrite(data + encode_result[1].bitstream_start_offset, 1,
        //        encode_result[1].bitstream_size, encodeConfig->outputVid);
    }

    uint32_t query_slot_idvcl = current_framebuffer_idx;
    result = vkGetQueryPoolResults(
        VulkanDevice, query_pool, query_slot_idvcl, 1,
        sizeof(VideoEncodeStatus), &encode_result[0], sizeof(VideoEncodeStatus),
        VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        LogError(
            "\nRetrieveData Error: Failed to get vcl query pool results.\n");
        return -1;
    }
    // fwrite(data + bitstream_offset + encode_result[0].bitstream_start_offset,
    // 1,
    //        encode_result[0].bitstream_size, config->output_vid);

    resource_alloc_.UnMap(out_bitstream_buffer);

    current_encode_frame_data->frame_submitted = false;

    return 0;
}
void EncodeContext::Close() {
    // vkDestroyVideoSessionParametersKHR
    //
}

VkFormat EncodeContext::CodecGetVkFormat(
    VkVideoChromaSubsamplingFlagBitsKHR chrome_format_idc,
    VkVideoComponentBitDepthFlagBitsKHR luma_bit_depth, bool semi_planar) {
    VkFormat vkformat = VK_FORMAT_UNDEFINED;
    switch (chrome_format_idc) {
        case VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR:
            switch (luma_bit_depth) {
                case VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR:
                    vkformat = VK_FORMAT_R8_UNORM;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR:
                    vkformat = VK_FORMAT_R10X6_UNORM_PACK16;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR:
                    vkformat = VK_FORMAT_R12X4_UNORM_PACK16;
                    break;
                default:
                    assert(0);
            }
            break;
        case VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR:
            switch (luma_bit_depth) {
                case VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR:
                    vkformat = semi_planar
                                   ? VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
                                   : VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR:
                    vkformat =
                        semi_planar
                            ? VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16
                            : VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR:
                    vkformat =
                        semi_planar
                            ? VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16
                            : VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
                    break;
                default:
                    assert(0);
            }
            break;
        case VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR:
            switch (luma_bit_depth) {
                case VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR:
                    vkformat = semi_planar
                                   ? VK_FORMAT_G8_B8R8_2PLANE_422_UNORM
                                   : VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR:
                    vkformat =
                        semi_planar
                            ? VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16
                            : VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR:
                    vkformat =
                        semi_planar
                            ? VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16
                            : VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
                    break;
                default:
                    assert(0);
            }
            break;
        case VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR:
            switch (luma_bit_depth) {
                case VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR:
                    vkformat = semi_planar
                                   ? VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT
                                   : VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR:
                    vkformat =
                        semi_planar
                            ? VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT
                            : VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16;
                    break;
                case VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR:
                    vkformat =
                        semi_planar
                            ? VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT
                            : VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16;
                    break;
                default:
                    assert(0);
            }
            break;
        default:
            assert(0);
    }

    return vkformat;
}

}  // namespace toystation