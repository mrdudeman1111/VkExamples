#version 450

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec3 outColor;

void main()
{
  gl_Position = vec4(vPos, 0.f, 1.f);

  outColor = vColor;
}

