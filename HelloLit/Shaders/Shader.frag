#version 450

  // this confused me at first, The "set = #" refers to the index of the associated descriptor layout in the pipeline layout's array
  // and the binding refers which binding in that set we are referring to
layout(set = 1, binding = 0) uniform sampler2D MeshTexture;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inCoord;
layout(location = 3) in vec3 inPos;

  // this specifies the image Attachment at location 0. In almost every case, this is the SwapchainImage
  // if we had more color attachments, we could read or write them, for example lets say we have an RGB color attachment at location 3 that we want to read,
  // layout(location = 3) in vec3 ColorAttachment;
  // would let us read the current pixel from the attachment. And swapping "in" with "out" would let us write to it instead.
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform ConstStruct
{
  vec3 CameraPos;
} Constants;

vec3 LightPos = vec3(0.f, 0.f, 5.f);

void main()
{
  vec4 CurrTexel = texture(MeshTexture, inCoord);

  vec3 CamDir = normalize(Constants.CameraPos - inPos);
  vec3 LightDir = normalize(LightPos - inPos);

  vec3 Halfway = normalize(CamDir+LightDir);

  float Intensity = clamp(dot(Halfway, inNorm), 0.0, 1.0);

  outColor = CurrTexel*Intensity;
}

