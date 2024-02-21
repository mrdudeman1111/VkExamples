#version 450

#pragma shader_stage(vertex)

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 2) in vec2 Position;
layout(location = 3) in vec2 Size;

layout(location = 0) out vec2 outUV;

void main()
{
  vec2 scaledPos = vec2(inPos.x*Size.x, inPos.y*Size.y)*2.f;

  vec2 ndcPos = (Position*2.f)-1.f;

  vec2 transformedPos = vec2(scaledPos.x+ndcPos.x, scaledPos.y+ndcPos.y);

  gl_Position = vec4(transformedPos, 0.f, 1.f);

  outUV = inUV;
}

