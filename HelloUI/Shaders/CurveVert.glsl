#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main()
{
  gl_Position = vec4(inPos, 0.f, 1.f);
  outUV = inUV;
}

