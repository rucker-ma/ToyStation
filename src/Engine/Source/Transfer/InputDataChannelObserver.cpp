#include "InputDataChannelObserver.h"

#include "Base/Logger.h"
#include "File/FileUtil.h"
#include "ToyEngine.h"


namespace toystation {

namespace {
using namespace webrtc;
const char* ToString(DataChannelInterface::DataState state) {
    switch (state) {
        case DataChannelInterface::kClosed:
            return "Closed";
        case DataChannelInterface::kClosing:
            return "Closing";
        case DataChannelInterface::kConnecting:
            return "Connecting";
        case DataChannelInterface::kOpen:
            return "Open";
    }
    return nullptr;
}


}  // namespace
const static JsonParseHelper kJsonParse;
InpuDataChannelObserver::~InpuDataChannelObserver() {
    if (channel_.get()) {
        channel_->UnregisterObserver();
    }
}
void InpuDataChannelObserver::SetDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> datachannel) {
    channel_ = datachannel;
    channel_->RegisterObserver(this);
}
void InpuDataChannelObserver::OnStateChange() {
    LogInfo("Datachannel: " + channel_->label() +
            " changed:" + ToString(channel_->state()));
}
void InpuDataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
    LogDebug("Input channel data:"+buffer.data.data<char>());
    Json::Value msg;
    if (kJsonParse.parse(buffer.data.data<char>(), buffer.size(), msg)) {
        if(msg["type"].asString() == std::string("keydown")){
            kEngine.GetInputSystem().OnKey(PRESS,msg["value"].asInt());
            kEngine.PushRenderFlag(RenderAction::Render_RenderDocCapture);
        }
        if(msg["type"].asString() == std::string("keyup")){
            kEngine.GetInputSystem().OnKey(RELEASE,msg["value"].asInt());

        }
    }
}
void InpuDataChannelObserver::OnBufferedAmountChange(uint64_t sent_data_size) {}
}  // namespace toystation