#include "VideoEncoder.h"

#include <modules/video_coding/include/video_codec_interface.h>
#include <modules/video_coding/include/video_error_codes.h>

#include "Base/Macro.h"

namespace toystation {
ToyVideoEncoder::~ToyVideoEncoder() {}
int ToyVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
                                const VideoEncoder::Settings& settings) {
    if (!codec_settings ||
        codec_settings->codecType != webrtc::VideoCodecType::kVideoCodecH264) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (codec_settings->maxFramerate == 0) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (codec_settings->width < 1 || codec_settings->height < 1) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }

    return InitX264Encoder(codec_settings, settings);
}
int32_t ToyVideoEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
    encoded_done_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}
int32_t ToyVideoEncoder::Release() {
    if (ctx_) {
        avcodec_free_context(&ctx_);
    }
    if (encode_frame_) {
        av_frame_free(&encode_frame_);
    }
    return WEBRTC_VIDEO_CODEC_OK;
}
int32_t ToyVideoEncoder::Encode(
    const webrtc::VideoFrame& frame,
    const std::vector<webrtc::VideoFrameType>* frame_types) {
    if (!encoder_) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (!encode_frame_) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (frame_types->front() == webrtc::VideoFrameType::kEmptyFrame) {
        return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
    }
    // copy buffer;

    encode_frame_->flags = 0;
    encode_frame_->pts = frame.timestamp();
    AVPacket* packet = av_packet_alloc();
    int ret = avcodec_send_frame(ctx_, encode_frame_);
    if (ret < 0) {
        LogWarn("Send frame encoding error");
        av_packet_free(&packet);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx_, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_packet_free(&packet);
            return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
        }
        if (ret < 0) {
            LogError("frame encoding failed,encodeframe return");
            av_packet_free(&packet);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        webrtc::CodecSpecificInfo codec_spec;
        codec_spec.codecType = webrtc::kVideoCodecH264;
        codec_spec.codecSpecific.H264.packetization_mode =
            webrtc::H264PacketizationMode::NonInterleaved;
        codec_spec.codecSpecific.H264.temporal_idx = webrtc::kNoTemporalIdx;
        codec_spec.codecSpecific.H264.idr_frame =
            packet->flags == AV_PKT_FLAG_KEY;

        webrtc::EncodedImage image;
        image._encodedWidth = ctx_->width;
        image._encodedHeight = ctx_->height;
        image.SetSpatialIndex(0);
        image._frameType = (packet->flags == 1)
                               ? webrtc::VideoFrameType::kVideoFrameKey
                               : webrtc::VideoFrameType::kVideoFrameDelta;

        auto buffer = webrtc::EncodedImageBuffer::Create(packet->size);
        image.SetEncodedData(buffer);
        image.set_size(0);
        memcpy(buffer->data(), packet->data, packet->size);
        image.set_size(image.size() + packet->size);
        image.SetTimestamp((uint32_t)packet->pts);
        encoded_done_->OnEncodedImage(image, &codec_spec);

        av_packet_unref(packet);
    }
    av_packet_free(&packet);
    return WEBRTC_VIDEO_CODEC_OK;
}
void ToyVideoEncoder::SetRates(const RateControlParameters& parameters) {
    ctx_->bit_rate = parameters.target_bitrate.get_sum_bps();
}
void ToyVideoEncoder::OnPacketLossRateUpdate(float packet_loss_rate) {}
void ToyVideoEncoder::OnRttUpdate(int64_t rtt_ms) {}
void ToyVideoEncoder::OnLossNotification(
    const LossNotification& loss_notification) {}
webrtc::VideoEncoder::EncoderInfo ToyVideoEncoder::GetEncoderInfo() const {
    webrtc::VideoEncoder::EncoderInfo encoder_info;
    encoder_info.supports_native_handle = false;
    encoder_info.implementation_name = "ToyStation";
    encoder_info.preferred_pixel_formats = {
        webrtc::VideoFrameBuffer::Type::kI420};
    return encoder_info;
}
int ToyVideoEncoder::InitX264Encoder(const webrtc::VideoCodec* codec_settings,
                                     const VideoEncoder::Settings& settings) {
    encoder_ = (AVCodec*)avcodec_find_encoder_by_name("libx264");
    if (!encoder_) {
        LogError("Not find libx264 encoder");
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    ctx_ = avcodec_alloc_context3(encoder_);
    if (!ctx_) {
        LogError("Cannot init AVCodecContext");
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    if (codec_settings->numberOfSimulcastStreams != 1) {
        LogError("Encoder not support simulcast now.");
        return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
    }
    ctx_->width = codec_settings->width;
    ctx_->height = codec_settings->height;
    ctx_->bit_rate = codec_settings->maxBitrate * 1000;
    ctx_->time_base = {1, 90000};
    ctx_->gop_size = codec_settings->H264().keyFrameInterval;
    ctx_->codec_type = AVMediaType::AVMEDIA_TYPE_VIDEO;
    ctx_->pix_fmt = AV_PIX_FMT_YUV420P;

    int ret = avcodec_open2(ctx_, encoder_, nullptr);
    if (ret < 0) {
        LogError("open encoder error");
        avcodec_free_context(&ctx_);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    encode_frame_ = av_frame_alloc();
    encode_frame_->width = ctx_->width;
    encode_frame_->height = ctx_->height;
    encode_frame_->format = AVPixelFormat::AV_PIX_FMT_YUV420P;
    ret = av_frame_get_buffer(encode_frame_, 0);
    if (ret < 0) {
        LogError("encode frame init error");
        avcodec_free_context(&ctx_);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    return WEBRTC_VIDEO_CODEC_OK;
}
}  // namespace toystation