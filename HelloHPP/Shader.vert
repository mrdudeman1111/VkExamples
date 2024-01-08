#version 440

const vec2 Triangle[] = 
  {
    vec2(0.75, 0.75),
    vec2(0.50, 0.25),
    vec2(0.25, 0.75)
  };

void main()
{
  gl_Position = vec4(Triangle[gl_VertexIndex], 0.f, 1.f);
}
