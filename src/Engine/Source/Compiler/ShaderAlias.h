//
// Created by ma on 2023/3/29.
//

#pragma once

#include <string>

namespace toystation{

using ShaderType=std::string;

const ShaderType kMainCameraPassVert="mainpass_vert";
const ShaderType kMainCameraPassFrag= "mainpass_frag";
const ShaderType kConvertNV12PassComp = "convert_nv12_comp";
const ShaderType kConvertYUVPassComp = "convert_yuv_comp";
}
