<template>
  <div class="main-container">
    <div class="top-container">
      <div class="left-container"></div>
      <div class="left-splitter" ref="left_splitter"></div>
      <div class="right-container">
        <RemoteVideo></RemoteVideo>
      </div>
    </div>
    <div class="splitter" ref="splitter"></div>
    <div class="bottom-container" ></div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from "vue";
import RemoteVideo from "./RemoteVideo.vue";
const splitter = ref<HTMLElement | null>(null);
const left_splitter = ref<HTMLElement | null>(null);

let isDragging = false;
let startY = 0;
let startX =0;
let startWidth =0;
let startHeight = 0;
let lastHeight = 0;
let lastWidth = 0;

let splitter_dragging = false;
let left_splitter_dragging = false;

const handleSplitterMouseDown = (event: MouseEvent) => {
  startY = event.clientY;
  startHeight = parseInt(
    window.getComputedStyle(
      splitter.value!.previousElementSibling as HTMLElement
    ).height,
    10
  );
  isDragging = true;
  splitter_dragging = true;
};
const handleLeftSplitterMouseDown=(event:MouseEvent)=>{
    startX = event.clientX;
    startWidth = parseInt(
    window.getComputedStyle(
      left_splitter.value!.previousElementSibling as HTMLElement
    ).width,
    10
  );
  isDragging = true;  
  left_splitter_dragging = true;
};
const handleMouseMove = (event: MouseEvent) => {
  if (!isDragging) {
    return;
  }
  if(splitter_dragging){
  const containerHeight = document.documentElement.clientHeight;
  const newHeight =
    startHeight +
    (event.clientY - startY) * (document.documentElement.offsetHeight / containerHeight);
  if (newHeight !== lastHeight) {
    const prevElem = splitter.value!.previousElementSibling as HTMLElement;
    const nextElem = splitter.value!.nextElementSibling as HTMLElement;
    prevElem.style.height = `${newHeight}px`;
    // nextElem.style.top = `${newHeight + splitter.value!.clientHeight}px`;
    lastHeight = newHeight;
  }
  }
  if(left_splitter_dragging){
  const containerWidth = document.documentElement.clientWidth;
  const newWidth = startWidth + (event.clientX-startX)*(document.documentElement.offsetWidth/containerWidth);
  if(newWidth!== lastWidth){
    const prevElem = left_splitter.value!.previousElementSibling as HTMLElement;
    const nextElem = left_splitter.value!.nextElementSibling as HTMLElement;
    prevElem.style.width = `${newWidth}px`;
    // nextElem.style.top = `${newHeight + splitter.value!.clientHeight}px`;
    lastWidth = newWidth;
  }
}

};

const handleMouseUp = () => {
  isDragging = false;
   splitter_dragging = false;
 left_splitter_dragging = false;
};

onMounted(() => {
  if (splitter.value) {
    splitter.value.addEventListener("mousedown", handleSplitterMouseDown);
  }
  if(left_splitter.value){
    left_splitter.value.addEventListener("mousedown", handleLeftSplitterMouseDown);
  }
    document.addEventListener("mousemove", handleMouseMove);
    document.addEventListener("mouseup", handleMouseUp);
  
});
</script>

<style scoped>
.main-container {
  height: 100%;
  display: flex;
  flex-direction: column;
}

.top-container {
  height: 75%;
  display: flex;
}

.left-container {
  /* flex: 0 0 30%; */
  width: 20%;
  background-color: #f0f0f0;
}
.left-splitter{
    width: 5px;
    background-color: brown;
    cursor: col-resize;
}
.left-splitter:hover{
    background-color: azure;
}
.right-container {
  flex: 1;
  background-color: #e0e0e0;
}

.splitter {
  height: 5px;
  background-color: #c0c0c0;
  cursor: row-resize;
}

.bottom-container {
  background-color: #d0d0d0;
  flex: 1;
}

.splitter:hover {
  background-color: #a0a0a0;
}
</style>
