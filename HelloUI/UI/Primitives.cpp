#include "Primitives.hpp"

#include <vector>

namespace Primitives
{
  Plane::Plane()
  {
    // {  {Pos.x, Pos.y}  {UV.x, UV.y}  }
    Vertices[0] = {{0.f, 1.f} , {0, 1.f}};
    Vertices[1] = {{0.f, 0.f} , {0.f, 0.f}};
    Vertices[2] = {{1.f, 1.f} , {1.f, 1.f}};
    Vertices[3] = {{1.f, 0.f} , {1.f, 0.f}};

    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 1;
    Indices[4] = 3;
    Indices[5] = 2;
  }

  Triangle::Triangle()
  {
    // 0.f  1.f 
    // 0.5f 0.f
    // 1.f  1.f
    Vertices[0] = {{-1.f, 1.f}, {0.f, 1.f}};
    Vertices[1] = {{0.f, -1.f}, {0.5f, 0.f}};
    Vertices[2] = {{1.f, 1.f}, {1.f, 1.f}};
  }

  PrimitiveBuffer::PrimitiveBuffer()
  {

  }
}

