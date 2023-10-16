#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vColor;

layout(binding = 0, set = 0) uniform Matrices {
  mat4 World;
  mat4 View;
  mat4 Proj;
} MVP;

layout(location = 0) out vec3 outColor;

void main()
{
  gl_Position = MVP.Proj * MVP.View * MVP.World *vec4(vPos, 1.f);

  outColor = vColor;
}

