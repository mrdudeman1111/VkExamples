#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec2 vCoord;

layout(binding = 0, set = 0) uniform Matrices {
  mat4 World;
  mat4 View;
  mat4 Proj;
} MVP;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outCoord;

void main()
{
  gl_Position = MVP.Proj * MVP.View * MVP.World *vec4(vPos, 1.f);

  outColor = vColor;
  outCoord = vCoord;
}

