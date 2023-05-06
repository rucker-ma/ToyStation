export { RtcClient };
class RtcClient {
  pc: RTCPeerConnection;
  ws: WebSocket | undefined;
  input_datachannel:RTCDataChannel |undefined;

  constructor() {
    this.pc = new RTCPeerConnection();
  }
  connect(): void {
    this.ws = new WebSocket("ws://localhost:9002");
    this.ws.onmessage = this.wsMessage.bind(this);

    this.pc.ondatachannel = this.getDatachannel.bind(this)
    console.log("connect to ...");
  }

  getDatachannel(event:RTCDataChannelEvent):void{
    this.input_datachannel = event.channel;
  }
  sendInput(message:string):void{
    this.input_datachannel?.send(message)
  }
  async wsMessage(ev: MessageEvent): Promise<void> {
    let payload = JSON.parse(ev.data);
    if (payload.type == "welcome") {
      payload.type = "join";
      this.ws?.send(JSON.stringify(payload));
    }
    if (payload.type == "offer") {
      if (payload.payload != null) {
        let session_init = new RTCSessionDescription({
          sdp: payload.payload,
          type: "offer",
        });
        await this.pc.setRemoteDescription(session_init);
        const answer = await this.pc.createAnswer();
        await this.pc.setLocalDescription(answer);
        payload.type = "answer";
        payload.payload = answer.sdp;
        this.ws?.send(JSON.stringify(payload));
      }
    }
    if (payload.type == "candidate") {
      const recvCandidate = JSON.parse(payload.payload);
      let candidate = new RTCIceCandidate({
        candidate: recvCandidate.candidate,
        sdpMid: recvCandidate.sdpMid,
        sdpMLineIndex: recvCandidate.sdpMLineIndex,
        usernameFragment: recvCandidate.usernameFragment,
      });
      await this.pc.addIceCandidate(candidate);
    }
  }
}
