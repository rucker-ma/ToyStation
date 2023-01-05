#pragma once
#include <api/media_stream_interface.h>

namespace toystation {
class RenderAudioSource : public webrtc::AudioTrackSinkInterface {
public:
    void Initialize();
    void OnData(const void* audio_data, int bits_per_sample, int sample_rate,
                size_t number_of_channels, size_t number_of_frames) override;
};
}  // namespace toystation