#version 450

#pragma shader_stage(fragment)

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Box_t {
  vec3 Color;
  float cornerRadius;
} Box;

const vec2 Center = vec2(0.5f, 0.5f); // UV center

float sdfRoundBox()
{
  vec2 tranPoint = UV-Center;

  return length( max(abs(tranPoint)-Center+Box.cornerRadius, 0.f) ) - Box.cornerRadius;
}

void main()
{
  float Alpha = sdfRoundBox();
  Alpha = (Alpha>0.f) ? 0.f : 1.f;

  outColor = vec4(Box.Color, Alpha);
}

