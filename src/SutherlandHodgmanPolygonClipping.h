// Sutherland Hodgeman Polygon clipping in C++
//
// License: MIT
//
// Dov Grobgeld <dov.grobgeld@gmail.com>
// 2022-07-23 Sat

#ifndef SUTHERLAND_HODGEMAN_POLYGON_CLIPPING_H
#define SUTHERLAND_HODGEMAN_POLYGON_CLIPPING_H

// Sutherland Hodgeman Polygon clipping in C++

#include <glm/vec2.hpp>
#include <vector>

namespace sutherland_hodgeman_polygon_clipping
{
using vec2 = glm::vec2;

// A line or an edge
struct vec2pair { 
  vec2 p0, p1;
};

using Polygon = std::vector<vec2>;

Polygon poly_clip(const Polygon& path,
                  const Polygon& rect_clip_path,
                  bool is_closed);
}

#endif
