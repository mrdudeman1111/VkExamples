#version 450

// this confused me at first, The "set = #" refers to the index of the layout in pipeline layout
// and the binding refers which binding in that set we are referring to
layout(set = 1, binding = 0) uniform sampler2D MeshTexture;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inCoord;

// this specifies the image Attacment at location 0. In almost every case, this is the SwapchainImage
// if we had more color attachments, we could read or write them, for example lets say we have an RGB color attachment at location 3 that we want to read,
// layout(location = 3) in vec3 ColorAttachment;
// would let us read the current pixel from the attachment. And swapping "in" with "out" would let us write to it instead.
layout(location = 0) out vec4 outColor;

void main()
{
  outColor = texture(MeshTexture, inCoord);
}

