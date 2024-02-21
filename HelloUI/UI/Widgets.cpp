#include "Widgets.hpp"

namespace Widgets
{  
  bool Widget::isCollide(glm::vec2 Pos)
  {
    uint32_t xMin;
    uint32_t xMax;

    uint32_t yMin;
    uint32_t yMax;

    xMin = Pos.x;
    xMax = Pos.x+Width;

    yMin = Pos.y;
    yMax = Pos.y+Height;

    if(Pos.x > xMin && Pos.x < xMax)
    {
      if(Pos.y > yMin && Pos.y < yMax)
      {
        return false;
      }
    }

    return false;
  }
}

