export { InputProcess };
import { ref } from "vue";
class InputProcess {
  private _video_ele: HTMLVideoElement;
  private _dragging: boolean;
  private _moving: boolean;
  private _filter_key: string[] = ["w", "s", "a", "d", "e", "q"];
  private _pressed_key:number[] = [];
  onSendMessage:Function;

  constructor(func:Function) {
    // @ts-ignore
    this._video_ele = null;
    this._dragging = false;
    this._moving = false;
    this.onSendMessage = func;
  }
  // 设置视频窗口的时间监听
  setupVideoMouseInput(video_element: HTMLVideoElement): void {
    this._video_ele = video_element;
    this._video_ele.oncontextmenu = function(){
      return false;
    }
    this._video_ele.addEventListener("mousedown", this.videoMouseDown.bind(this));
    this._video_ele.addEventListener("mousemove", this.videoMouseMove.bind(this));
    this._video_ele.addEventListener("mouseup", this.videoMouseUp.bind(this));
  }
  videoMouseDown(e: MouseEvent) {
    if (e.button == 0) {
      this._dragging = true;
      // document.body.style.cursor = "move";

    } else if (e.button == 2 ) {
      //this._video_ele.requestPointerLock();
      this._moving = true;
      // document.body.style.cursor = "pointer";
    }
    const keyinfo = {
      trigger:"mouse",
      type: "down",
      key: e.button,
      //在后端引擎中，以坐下角为（x,y)最小点，因此y = 1-y
      position: {
        x: e.offsetX/this._video_ele.clientWidth,
        y: 1.0- e.offsetY/this._video_ele.clientHeight,
      },
      delta:{
        x:e.movementX/this._video_ele.clientWidth,
        y:e.movementY/this._video_ele.clientHeight
      }
    };
    console.log(keyinfo.position.x,keyinfo.position.y);
    this.onSendMessage(keyinfo);
  }
  videoMouseMove(e: MouseEvent) {
    //if (this._dragging) {
    if(e.movementX == 0 && e.movementY == 0){
      return;
    }
    const keyinfo = {
      trigger:"mouse",
      type: "move",
      key: e.button,
      position: {
        x: e.offsetX/this._video_ele.clientWidth,
        y: e.offsetY/this._video_ele.clientHeight,
      },
      delta:{
        x:e.movementX/this._video_ele.clientWidth,
        y:e.movementY/this._video_ele.clientHeight
      }
    };
    if(this._moving){
      keyinfo.key = 2;//右键按下移动
      console.log(keyinfo.delta)
    }
    // console.log(keyinfo);
    this.onSendMessage(keyinfo);
    //}
    // if(this._moving){
    //   const keyinfo = {
    //     type: "move",
    //     value: {
    //       deltax: e.movementX,
    //       deltay: e.movementY,
    //     },
    //   };
    //   this.onSendMessage(keyinfo);
    // }
  }
  videoMouseUp(e: MouseEvent) {
    this._dragging = false;
    this._moving = false;
  }
  setupKeyboardInput():void{
    document.addEventListener("keydown",this.keyDown.bind(this));
    document.addEventListener("keyup",this.keyUp.bind(this));
    document.addEventListener("pointerlockchange",this.changeMoving.bind(this));
    setInterval(this.sendKeyEventPeriod.bind(this), 16);
  }
  sendKeyEventPeriod(){
    this._pressed_key.forEach(key=>{
      const keyinfo = {
        trigger:"keyboard",
        type: "down",
        key: key,
        repeat:1
      };
      this.onSendMessage(keyinfo);
    });
  }
  changeMoving(){
    this._moving = !this._moving;
  }
  keyDown(e:KeyboardEvent){
    if (this._filter_key.find((candidate) => candidate === e.key) !== undefined) {

      if(this._pressed_key.indexOf(e.keyCode) == -1){
        this._pressed_key.push(e.keyCode);
      }
    }
    if (e.key == "Escape") {
      document.exitPointerLock();
      this._moving = false;
    }
  }
  keyUp(e:KeyboardEvent){
    if (this._filter_key.find((candidate) => candidate === e.key) !== undefined) {
      const position = this._pressed_key.indexOf(e.keyCode);
      if(position != -1){
        delete this._pressed_key[position];
      }
    }
  }
}
