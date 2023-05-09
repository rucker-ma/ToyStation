<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from "vue";
import { RtcClient } from "../transfer/rtc";

const client = new RtcClient();

const container = ref<HTMLDivElement | null>(null);
const remoteVideo = ref<HTMLVideoElement | null>(null);

let dragging: boolean = false;
let moving:boolean = false;
// let mouse_delta_position = { x: 0, y: 0 };

onMounted(() => {
  if (remoteVideo.value) {
    // remoteVideo.value.load();
    remoteVideo.value.addEventListener("mousedown", (e: MouseEvent) => {
      if(e.button == 0){ //鼠标左键按下
        dragging = true;
        // mouse_delta_position = { x: e.clientX, y: e.clientY };
      }else if(e.button == 2){
        remoteVideo.value?.requestPointerLock(); //监听请求结果：document.onpointerlockchange
      }
      
    });

    remoteVideo.value.addEventListener("mousemove", (e: MouseEvent) => {
      if (dragging) {
        // const deltaX = e.clientX - mouse_delta_position.x;
        // const deltaY = e.clientY - mouse_delta_position.y;
        // console.log("deltax:",deltaX, deltaY);
        // console.log("offset",deltaX,deltaY);
        // mouse_delta_position = { x: e.clientX, y: e.clientY };
        const keyinfo = {
          type: "dragmove",
          value: {
            deltax:e.offsetX,
            deltay:e.offsetY
          },
        };
        let content = JSON.stringify(keyinfo);
        client.sendInput(content);
      }
      if(moving){
          // console.log(e.movementX,e.movementY);
          const keyinfo = {
          type: "lockmove",
          value: {
            deltax:e.movementX,
            deltay:e.movementY
          },
        };
        let content = JSON.stringify(keyinfo);
        client.sendInput(content);          
      }
    });
    remoteVideo.value.addEventListener("mouseup", (e: MouseEvent) => {
      dragging = false;
    });
  }
});

function connectToServer() {
  client.pc.ontrack = getTrack;
  client.connect();
}
function renderdocCapture() {
  const keyinfo = {
    type: "capture",
    value: 0,
  };
  let content = JSON.stringify(keyinfo);
  client.sendInput(content);
}
function shaderUpdate(){
  const keyinfo = {
    type: "shader",
    value: 0,
  };
  let content = JSON.stringify(keyinfo);
  client.sendInput(content);
}
function getTrack(ev: RTCTrackEvent): void {
  if (ev.track.kind === "video") {
    if (remoteVideo.value) {
      remoteVideo.value.srcObject = ev.streams[0];
      const video_track :MediaStreamTrack = ev.streams[0].getVideoTracks()[0];
       video_track.onmute = function (ev) {
         console.log("mute...")
       }
       video_track.onunmute = function (ev){
         console.log("unmute...")
       }
      remoteVideo.value.addEventListener("error",function (err) {
        console.log("remote video error" + err);
      });
      remoteVideo.value.addEventListener("loadedmetadata",function () {
        remoteVideo.value!.play();
      })
    }
  }
}

let filter_key: string[] = ["w", "s", "a", "d"];

document.addEventListener("keydown", function (e) {
  if (filter_key.find((candidate) => candidate === e.key) !== undefined) {
    const keyinfo = {
      type: "keydown",
      value: e.keyCode,
    };
    let content = JSON.stringify(keyinfo);
    client.sendInput(content);
  }
  if(e.key == "Escape"){
    document.exitPointerLock();
    moving = false;
  }
});
document.addEventListener("pointerlockchange",function(e) {
  moving = !moving;
  console.log(moving);
});

document.addEventListener("keyup", (e: KeyboardEvent) => {
  if (filter_key.find((candidate) => candidate === e.key) !== undefined) {
    const keyinfo = {
      type: "keyup",
      value: e.keyCode,
    };
    let content = JSON.stringify(keyinfo);
    client.sendInput(content);
  }
});
const maxWidth = computed(() => {
  if (!remoteVideo.value) return "none";
  const { width, height } = remoteVideo.value;
  const aspectRatio = width / height;
  const containerWidth = container.value!.clientWidth;
  const containerHeight = container.value!.clientHeight;
  const maxHeight = containerHeight;
  const maxWidth = maxHeight * aspectRatio;
  return maxWidth > containerWidth ? containerWidth : "none";
});

const maxHeight = computed(() => {
  if (!remoteVideo.value) return "none";
  const { width, height } = remoteVideo.value;
  const aspectRatio = width / height;
  const containerHeight = container.value!.clientHeight;
  const maxHeight = containerHeight;
  return maxHeight;
});
</script>

<template>
   <div ref="container" class="container">
     <div class="video-container">
      <video
        id="remote-video"
        ref="remoteVideo"
        class="remote-video"
        playsinline
        preload="metadata"
        :style="`max-width: ${maxWidth}px; max-height: ${maxHeight}px; width: 100%; height: 100%;`"
      ></video>
     </div>
     <div class="floating-button-container">
      <el-button @click="connectToServer">Connect</el-button>
     </div>
     <div class="capture-button-container">
      <el-button @click="renderdocCapture">Capture</el-button>
     </div>
     <div class="shader-button-container">
       <el-button @click="shaderUpdate">Shader</el-button>
     </div>
   </div>
</template>
<style scoped>
.container {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
}
.video-container {
  position: relative;
  width: 100%;
  justify-items: center;
  justify-content: center;
}

.remote-video {
  position: relative;
  top: 0;
  left: 0;
  object-fit: contain;
  background-color: rgb(0, 0, 0);
}

.floating-button-container {
  position: absolute;
  top: 10px;
  right: 10px;
  z-index: 2;
}
.capture-button-container {
  position: absolute;
  top: 60px;
  right: 10px;
  z-index: 2;
}
.shader-button-container{
  position: absolute;
  top: 110px;
  right: 10px;
  z-index: 2;
}
</style>
