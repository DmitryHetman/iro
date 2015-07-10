#pragma once

#include <string>

/*
const std::string sourceVS =
      "#version 300 es\n"
      "precision mediump float;\n"
      "in vec2 pos;\n"
      "in vec2 vs_uv;\n"
      "out vec2 uv;\n"
      "void main() {\n"
      "  gl_Position = vec4(pos, 1.0,1.0);\n"
      "  uv = vs_uv;\n"
      "}\n";
      */

const std::string sourceVS =
     "#version 100\n"
      "precision mediump float;\n"
      "attribute vec2 pos;\n"
      "attribute vec2 uv;\n"
      "varying vec2 vs_uv;\n"
      "void main() {\n"
      "  gl_Position = vec4(pos, 1., 1.);\n"
      "  vs_uv = uv;\n"
      "}\n";


const std::string argbFS =
      "#version 100\n"
      "precision mediump float;\n"
      "uniform sampler2D texture0;\n"
      "varying vec2 vs_uv;\n"
      "void main() {\n"
      "  gl_FragColor = texture2D(texture0, vs_uv);\n"
      "}\n";

const std::string rgbFS =
      "#version 100\n"
      "precision mediump float;\n"
      "uniform sampler2D texture0;\n"
      "varying vec2 vs_uv;\n"
      "void main() {\n"
      "  gl_FragColor = vec4(texture2D(texture0, vs_uv).rgb, 1.0);\n"
      "}\n";

/*
const std::string argbFS =
      "#version 300 es\n"
      "precision mediump float;\n"
      "uniform sampler2D texture0;\n"
      "in vec2 uv;\n"
      "out vec4 outColor;\n"
      "void main() {\n"
      "  outColor = texture2D(texture0, uv);\n"
      "}\n";

const std::string rgbFS =
      "#version 300 es\n"
      "precision mediump float;\n"
      "uniform sampler2D texture0;\n"
      "in vec2 uv;\n"
      "out vec4 outColor;\n"
      "void main() {\n"
      "  outColor = vec4(texture2D(texture0, uv).rgb, 1.0);\n"
      "}\n";
*/
