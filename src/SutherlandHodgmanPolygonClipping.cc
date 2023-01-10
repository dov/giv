// Sutherland Hodgeman Polygon clipping in C++
//
// License: 

// Dov Grobgeld <dov.grobgeld@gmail.com>
// 2022-07-23 Sat

#include <math.h>
#include "SutherlandHodgmanPolygonClipping.h"
#include <fmt/core.h>

using namespace std;
using namespace fmt;

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

// Check if the  candidate point is colinear with the edge.
// The "epsilon" is totally heuristic.
//
// This works quite well, though there is a chance for a false
// positive if there is an valid edge on the inside of the polyline
// that is colinear with the first to last edge.
static bool contained_by(const vec2pair& candidate, const vec2pair& edge)
{
  double c_dx = candidate.p1.x - candidate.p0.x;
  double c_dy = candidate.p1.y - candidate.p0.y;
  double e_dx = edge.p1.x - edge.p0.x;
  double e_dy = edge.p1.y - edge.p0.y;

  // Currently just look for colinearity. This should also look for
  // "inside"
  double epsilon = 10;
  double diff = fabs(c_dx * e_dy - c_dy * e_dx);
  if (fabs(diff) > epsilon)
    return false;

  // What other heuristics? Perhaps touches edge?
  return true;
}


// Sutherland Hodgman Polygon clipping algorithm. The disadvantage
// of this algorithm is that it does not distinguish between
// an edge between the last and the first segment of the path
// vs a real edge. This is hackishly taken care of in an
// additional sweep that rotates the path so that it the
// last to the first point occur at the end of the clipped path.  
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

  // Sweep through list looking for a pair that is contained by
  // the first to last point, and rotate the list to put
  // it at the end.
  vec2pair first_last {path.back(), path[0]};
  int n = (int)out_list.size();
  for (int i=0; i<(int)out_list.size(); i++)
  {
    if (contained_by(first_last, {out_list[(i-1+n)%n], out_list[i]}))
    {
      // Rotate the output list so that it starts at the same point
      // as the path.
      Polygon new_list;
      for (int j=0; j<(int)out_list.size(); j++)
        new_list.push_back(out_list[(i+j)%n]);
      out_list = new_list;
      break;
    }
  }

  return out_list;
}

}
