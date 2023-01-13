#include "SessionCreator.h"

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>

#include "Base/Macro.h"

namespace toystation {
void SessionCreator::Initialize() { SetupFactory(); }
std::shared_ptr<TransferSession> SessionCreator::CreateSession() {
    return std::make_shared<TransferSession>();
}
void SessionCreator::SetupFactory() {
    RTC_CHECK(!peer_connection_factory_);
    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
        rtc::ThreadManager::Instance()->CurrentThread(), nullptr, nullptr,
        nullptr, webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(),
        webrtc::CreateBuiltinVideoDecoderFactory(), nullptr, nullptr);
    if (!peer_connection_factory_) {
        LogError("Failed to initialize webrtc PeerConnectionFactory");
    }
}
void SessionCreator::SetupTracks() {
    // audio_track_ = peer_connection_factory_->CreateAudioTrack(
    //     "audio",
    //     peer_connection_factory_->CreateAudioSource(cricket::AudioOptions()));
    // TODO:imple video source

    video_source_ = new rtc::RefCountedObject<RenderVideoSource>();
    video_track_ =
        peer_connection_factory_->CreateVideoTrack("video", video_source_);
}
}  // namespace toystation