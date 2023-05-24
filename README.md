
![WindowsBuild](https://github.com/rucker-ma/ToyStation/actions/workflows/dotnet-desktop.yml/badge.svg) ![Format](https://github.com/rucker-ma/ToyStation/actions/workflows/format-check.yml/badge.svg)



TODO:
 - 资产系统
    - gltf资产解析
      * 完成对metallic-roughness模式的读取
 - 搭建基本PBR渲染流程
    - 抽出camera对象
      * 完成
    - 天空大气和体积云
    - PBR
      * metallic-roughness工作流基本完成
    - 抗锯齿
    - 物体的可视编辑（通过脚本层向下传递？）
    - 阴影
    
 - 编码和传输
    - 视频源向编码器传递中由于编码器busy导致的丢帧
    - 视频编码器的编码延时太长，zerolatency模式也有问题，需要解决
      * 采用硬件编码完成
    - 需要查看webrtc中videostreamencoder对象逻辑


### Vukan和渲染的记录
1. Vulkan中NDC空间范围为[0,1]
2. 渲染系统使用左手系
3. 相机移动规则,针对相机视角而言
   * e：up d: right w:in

4. ue系统参考：
   left hand: x left ,y in ,z up //x需要再确认
5. 当前系统：
   left hand: x right ,y up,  z in