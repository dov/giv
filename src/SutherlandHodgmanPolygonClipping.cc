// Sutherland Hodgeman Polygon clipping in C++
//
// License: 

// Dov Grobgeld <dov.grobgeld@gmail.com>
// 2022-07-23 Sat

#include <math.h>
#include "SutherlandHodgmanPolygonClipping.h"

using namespace std;

namespace sutherland_hodgeman_polygon_clipping {

static vec2 compute_intersection(const vec2pair& line1,
                                 const vec2pair& line2)
{
  double epsilon = 1e-6;

  const vec2& p1 = line1.p0;
  const vec2& p2 = line1.p1;
  const vec2& p3 = line2.p0;
  const vec2& p4 = line2.p1;
  double x,y; 

  // line1 horizontal
  if (fabs(p2.x - p1.x) < epsilon)
  {
    x = p1.x;
    double m2 = (p4.y - p3.y) / (p4.x - p3.x);
    double b2 = p3.y - m2 * p3.x;
    y = m2 * x + b2;
  }
  // line2 horizontal
  else if (fabs(p4.x - p3.x)< epsilon)
  {
    x = p3.x;
    double m1 = (p2.y - p1.y) / (p2.x - p1.x);
    double b1 = p1.y - m1 * p1.x;
    y = m1 * x + b1;
  }
  // Neither - Note we don't need proct from parallell
  else
  {
    double m1 = (p2.y - p1.y) / (p2.x - p1.x);
    double b1 = p1.y - m1 * p1.x;
    double m2 = (p4.y - p3.y) / (p4.x - p3.x);
    double b2 = p3.y - m2 * p3.x;
    x = (b2 - b1) / (m1 - m2);
    y = m1 * x + b1;
  }
  
  return vec2 {x,y};
}
        
static bool is_inside_edge(const vec2& q, const vec2pair& edge)
{
  const vec2& p1 = edge.p0;
  const vec2& p2 = edge.p1;

  return ((p2.x - p1.x) * (q.y - p1.y) - (p2.y - p1.y) * (q.x - p1.x)
          <=0);
}

// Sutherland Hodgman Polygon clipping algorithm
Polygon poly_clip(const Polygon& path,
                  const Polygon& rect_clip_path)
{
  Polygon out_list = path;

  int m = (int)rect_clip_path.size();
  for (int i=0; i<m; i++)
  {
    vec2pair clip_edge {rect_clip_path[(i-1+m)%m], rect_clip_path[i]};

    Polygon in_list = out_list;
    int n = (int)in_list.size();
    out_list.clear();

    for (int j=0; j<n; j++)
    {
      const vec2& prev_point = in_list[(j + n - 1) % n];
      const vec2& current_point = in_list[j];

      if (is_inside_edge(current_point, clip_edge))
      {
        if (!is_inside_edge(prev_point, clip_edge))
          out_list.push_back(compute_intersection(
                               {prev_point, current_point}, clip_edge));
        out_list.push_back(current_point);
      }
      else if (is_inside_edge(prev_point, clip_edge))
        out_list.push_back(compute_intersection(
                             {prev_point, current_point}, clip_edge));
    }
  }

  return out_list;
}

}
