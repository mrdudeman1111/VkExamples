#version 420
#pragma shader_stage(vertex)

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vUV;

layout(location = 2) in vec2 instPos;
layout(location = 3) in vec2 instSize;
layout(location = 4) in int inTxIdx;

layout(location = 0) out vec2 uv;
layout(location = 1) out int outTxIdx;

void main()
{
  vec2 scaledPos = vec2(vPos.x*instSize.x, vPos.y*instSize.y)*2.f;

  vec2 ndcPos = (instPos*2.f)-1.f;

  vec2 transformPos = vec2(scaledPos.x+ndcPos.x, scaledPos.y+ndcPos.y);

  gl_Position = vec4(transformPos, 0.f, 1.f);

  uv = vUV;
  outTxIdx = inTxIdx;
}

