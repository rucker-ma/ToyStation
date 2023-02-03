#pragma once

#include <api/create_peerconnection_factory.h>
#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>
#include <api/task_queue/default_task_queue_factory.h>
#include <rtc_base/logging.h>
#include <rtc_base/win32_socket_server.h>
#include "RenderVideoSource.h"
#include "TransferSession.h"

namespace toystation {
class WebRtcLogSink : public rtc::LogSink {
public:
    virtual ~WebRtcLogSink() {}
    void OnLogMessage(const std::string& message,
                      rtc::LoggingSeverity severity) override;
    void OnLogMessage(const std::string& message) override;
};

struct WebRtcInitInfo {
    std::unique_ptr<rtc::Thread> worker_thread;
    std::unique_ptr<rtc::Thread> signaling_thread;
    std::unique_ptr<rtc::Thread> network_thread;
};

class SessionCreator {
public:
    void Initialize();
    std::shared_ptr<TransferSession> CreateSession(
        std::unique_ptr<SessionClient> client);

private:
    void InitWebRtc();
    void SetupFactory();
    void SetupTracks();

    WebRtcLogSink rtc_log_;

    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
        peer_connection_factory_;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;

    rtc::scoped_refptr<rtc::AdaptedVideoTrackSource> video_source_;

    std::shared_ptr<webrtc::PeerConnectionInterface::RTCConfiguration> config_;

    std::unique_ptr<rtc::Thread> worker_thread_;
    std::unique_ptr<rtc::Thread> signaling_thread_;
    std::unique_ptr<rtc::Thread> network_thread_;
    rtc::Win32SocketServer win32_server_;
    
    webrtc::PeerConnectionInterface::RTCConfiguration config;
};
}  // namespace toystation