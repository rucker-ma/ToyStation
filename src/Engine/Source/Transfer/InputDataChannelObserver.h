#pragma once
#include <api/data_channel_interface.h>

namespace toystation {
class InpuDataChannelObserver : public webrtc::DataChannelObserver {
public:
    InpuDataChannelObserver()=default;
    virtual ~InpuDataChannelObserver();
    void SetDataChannel( rtc::scoped_refptr<webrtc::DataChannelInterface> datachannel);
    void OnStateChange() override;
    void OnMessage(const webrtc::DataBuffer& buffer) override;
    void OnBufferedAmountChange(uint64_t sent_data_size) override;

private:
    rtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
};
}  // namespace toystation