<script setup lang="ts">
import { ref } from "vue";
import { RtcClient } from "../transfer/rtc";
const client = new RtcClient();
const remoteVideo = ref<InstanceType<typeof HTMLVideoElement>>();

function connectToServer() {
  client.pc.ontrack = getTrack;
  client.connect();
}


function getTrack(ev: RTCTrackEvent): void {
  if (ev.track.kind === "video") {
    if (remoteVideo.value) {
      remoteVideo.value.srcObject = ev.streams[0];
    }
  }
}
let filter_key: string[] = ["w", "s", "a", "d"];

document.addEventListener("keydown", function (e) {
  if (filter_key.find((candidate) => candidate === e.key) !== undefined) {
    const keyinfo={
      type:'keydown',
      value:e.keyCode
    }
    let content = JSON.stringify(keyinfo)
    client.sendInput(content)    
  }
});
document.addEventListener("keyup",(e:KeyboardEvent)=>{
  if (filter_key.find((candidate) => candidate === e.key) !== undefined) {
    const keyinfo={
      type:'keyup',
      value:e.keyCode
    }
    let content = JSON.stringify(keyinfo)
    client.sendInput(content)    
  }
});
</script>

<template>
  <div class="videoBox">
    <video id="remote-video" ref="remoteVideo" playsinline autoplay></video>
  </div>
  <div>
    <button @click="connectToServer">Connect</button>
  </div>
</template>

<style scoped>
.videoBox {
  background-color: rgba(183, 184, 179, 0.637);
  width: 100%;
  height: calc(100%-70px);
  align-items: center;
  justify-content: center;
  display: flex;
  overflow: hidden;
}
video {
  object-fit: fill;
  width: 100%;
  height: 100%;
}
</style>
