#include "RenderAudioSource.h"
namespace toystation {
void RenderAudioSource::Initialize() {}

void RenderAudioSource::OnData(const void* audio_data, int bits_per_sample,
                               int sample_rate, size_t number_of_channels,
                               size_t number_of_frames) {}
}  // namespace toystation