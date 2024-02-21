#version 420
#pragma shader_stage(fragment)

// ascii-8 only has 93 characters with Images
layout(binding = 0) uniform utexture2D FontTextures[93];
layout(binding = 1) uniform sampler FontSampler;

layout(location = 0) in vec2 UV;
layout(location = 1) flat in int TxIdx;

layout(location = 0) out vec4 outColor;

const float opacity = 0.5f;

void main()
{
  float Strength = texture(usampler2D(FontTextures[TxIdx], FontSampler), UV).x;

  if(Strength > 0)
  {
    outColor = vec4(Strength, Strength, Strength, opacity);
  }
  else
  {
    discard;
  }
}

