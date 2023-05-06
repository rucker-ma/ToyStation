//
// Created by ma on 2023/5/2.
//

#pragma once
#include <vector>

#include "VideoEncoder.h"

namespace toystation{
//debug helper for render and encode
class VideoSaver{
public:
    void Init(const webrtc::VideoCodec* codec_settings);
    void WritePacket(std::vector<uint8_t>& data,std::vector<uint8_t> extra_data);
    void End();
private:
    AVCodecContext* ctx_;
    AVFormatContext* oc_;
    AVStream* out_stream_;

    AVPacket* pkt;
    long long index_;
};

}
