#version 450

layout(location = 0) in vec2 inPos;

void main()
{
  gl_Position = vec4(inPos, 1.f, 1.f);
}

