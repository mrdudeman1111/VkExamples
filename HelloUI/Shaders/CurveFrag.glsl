#version 440
#pragma shader_stage(fragment)

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

vec2 evalBezier(vec2 point1, vec2 point2, vec2 point3, float T)
{
  vec2 Ret;

// X represents the First point's influence
  float X = (1.f-T);

// first point
  Ret = pow(X, 2)*point1;

// mid point
  Ret += 2*X*T*point2;

// last point
  Ret += pow(T, 2)*point3;

  return Ret;
}

void main()
{
  // Compute position on the Bezier curve using the UV coordinates
  float t = inUV.x; // Use the U component of UV as the parameter

  vec2 P1 = vec2(0.f, 1.f);
  vec2 P2 = vec2(0.5f, 0.f);
  vec2 P3 = vec2(1.f, 1.f);

  vec2 curvePosition = evalBezier(P1, P2, P3, t);

  if(inUV.y >= curvePosition.y)
  {
    outColor = vec4(1.f);
  }
  else
  {
    outColor = vec4(0.f);
  }

  return;
}

