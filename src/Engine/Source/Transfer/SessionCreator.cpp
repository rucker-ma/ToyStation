#include "SessionCreator.h"

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/create_peerconnection_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/win32_socket_init.h>

#include "Base/Macro.h"
#include "TransferSystem.h"

#include "WebRTC/VideoEncoderFactory.h"
namespace toystation {

void WebRtcLogSink::OnLogMessage(const std::string& message,
                                 rtc::LoggingSeverity severity) {
    switch (severity) {
        case rtc::LS_VERBOSE:
            LogDebug(message);
            break;
        case rtc::LS_INFO:
            LogInfo(message);
            /* code */
            break;
        case rtc::LS_WARNING:
            LogWarn(message);
            break;
        case rtc::LS_ERROR:
            LogError(message);
            break;
        default:
            break;
    }
}
void WebRtcLogSink::OnLogMessage(const std::string& message) {
    LogInfo(message);
}

void SessionCreator::Initialize() {
    InitWebRtc();
    SetupFactory();
    SetupTracks();
}
std::shared_ptr<TransferSession> SessionCreator::CreateSession(
    std::unique_ptr<SessionClient> client) {
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;
    auto session = std::make_shared<TransferSession>(std::move(client));

    session->peer_connection_ = peer_connection_factory_->CreatePeerConnection(
        config, nullptr, nullptr, session.get());

    // session->peer_connection_->AddTrack(audio_track_, {"audio"});
    session->peer_connection_->AddTrack(video_track_, {"video"});
    return session;
}
void SessionCreator::InitWebRtc() {
    rtc::LogMessage::AddLogToStream(&rtc_log_, rtc::LS_WARNING);
    rtc::InitializeSSL();
    rtc::WinsockInitializer winsock_init;

    // rtc::Win32Thread win32_thread(&win32_server_);
    //  network_thread_ = std::make_unique<rtc::Win32Thread>(&win32_server_);

    // rtc::ThreadManager::Instance()->wr(network_thread_.get());
    rtc::ThreadManager::Instance()->WrapCurrentThread();

    signaling_thread_ = rtc::Thread::Create();
    signaling_thread_->SetName("signaling_thread", nullptr);
    worker_thread_ = rtc::Thread::Create();
    worker_thread_->SetName("worker_thread", nullptr);
    if (!signaling_thread_->Start() || !worker_thread_->Start()) {
        LogError("Start Thread Error");
    }
}
void SessionCreator::SetupFactory() {
    RTC_CHECK(!peer_connection_factory_);

    // auto task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
    // auto adm =
    //     worker_thread_->Invoke<rtc::scoped_refptr<webrtc::AudioDeviceModule>>(
    //         RTC_FROM_HERE, [this, &task_queue_factory] {
    //             auto audio_module = webrtc::AudioDeviceModule::Create(
    //                 webrtc::AudioDeviceModule::AudioLayer::kWindowsCoreAudio,
    //                 task_queue_factory.get());
    //             return audio_module;
    //         });

    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
        nullptr, worker_thread_.get(), signaling_thread_.get(), nullptr,
        webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        // webrtc::CreateBuiltinVideoEncoderFactory(),
        std::make_unique<ToyVideoEncoderFactory>(),
         webrtc::CreateBuiltinVideoDecoderFactory(),
        //std::make_unique<ToyVideoDecoderFactory>(),
         nullptr, nullptr);

    if (!peer_connection_factory_) {
        LogError("Failed to initialize webrtc PeerConnectionFactory");
    }
}
void SessionCreator::SetupTracks() {
    audio_track_ = peer_connection_factory_->CreateAudioTrack(
        "audio",
        peer_connection_factory_->CreateAudioSource(cricket::AudioOptions()));
    // TODO:imple video source

    auto source = new rtc::RefCountedObject<RenderVideoSource>();
    source->Initialize();
    video_source_ = source;
    video_track_ =
        peer_connection_factory_->CreateVideoTrack("video", video_source_);
}
}  // namespace toystation