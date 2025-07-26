//======================================================================
//  GivRenderer.cc - Paint the giv data through the painter class
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Nov  6 22:55:53 2007
//----------------------------------------------------------------------

#include "GivRenderer.h"
#include <math.h>
#include "SutherlandHodgmanPolygonClipping.h"
#include <fmt/core.h>

using namespace sutherland_hodgeman_polygon_clipping;
using namespace fmt;

#define COLOR_NONE 0xfffe 
static constexpr const char* DEFAULT_FONT="Sans 15"; // does this exist on windows?

GivRenderer::GivRenderer(GPtrArray *_datasets,
                         GivPainter& _painter,
                         double _scale_x,
                         double _scale_y,
                         double _shift_x,
                         double _shift_y,
                         double _width,
                         double _height,
                         double _quiver_scale
                         ) : 
    datasets(_datasets),
    painter(_painter),
    scale_x(_scale_x),
    scale_y(_scale_y),
    shift_x(_shift_x),
    shift_y(_shift_y),
    width(_width),
    height(_height),
    quiver_scale(_quiver_scale),
    do_no_transparency(false)
{
}

#if 0
static void print_polygon(const ClippingPolygon& poly)
{
    print("{{");
    for (int i=0; i<(int)poly.size(); i++) {
        const auto& p = poly[i];
        print("{{{:.3f},{:.3f}}}", p.x,p.y);
        if (i<(int)poly.size()-1)
            print(",");
    }
    print("}}\n");
}
#endif

// Apply the clipping algorithm and add the given polygon
void GivRenderer::add_clipped_poly(const ClippingPolygon& poly,
                                   bool is_closed)
{
    int n = (int)poly.size();
    if (n<2)
        return;

    ClippingPolygon clip = poly_clip(poly, clip_rect, is_closed);
#if 0
    print("input_poly: "); print_polygon(poly);
    print("clip_poly: "); print_polygon(clip_rect);
    print("clipped_poly: "); print_polygon(clip);
#endif
    painter.new_path(); // Causes a moveto
    n = (int)clip.size();
    for (int i=0; i<(int)clip.size()-1; i++) 
        painter.add_line_segment(clip[i].x, clip[i].y,
                                 clip[(i+1)%n].x, clip[(i+1)%n].y, 
                                 true);
}

// Build a clipping path given the given margin
void GivRenderer::build_clip_rect(double margin)
{
    clip_rect.clear();
    clip_rect.push_back({-margin,-margin});
    clip_rect.push_back({-margin,height+margin});
    clip_rect.push_back({width+margin,height+margin});
    clip_rect.push_back({width+margin,-margin});
}

void GivRenderer::paint()
{
    const double cs = 1.0/65535;
    
    for (int ds_idx=0; ds_idx<(int)datasets->len; ds_idx++) {
        giv_dataset_t *dataset = (giv_dataset_t*)g_ptr_array_index(datasets, ds_idx);
        if (!dataset->is_visible)
            continue;
        
        painter.set_set_idx(ds_idx);

        if (dataset->svg)
            // TBD - add transformation
            painter.render_svg_path(dataset->svg,
                                    -shift_x,-shift_y,scale_x,scale_y);

        // Create negative color values for "invisible" datasets
        double rr = cs*dataset->color.red;
        double gg = cs*dataset->color.green;
        double bb = cs*dataset->color.blue;
        double alpha = cs*dataset->color.alpha;
        if (this->do_no_transparency)
            alpha = 1.0;

#if 0
        printf("datasets[%d]->color.pixel = %d\n",
               ds_idx, dataset->color.pixel);
#endif
        if (dataset->color.alpha == COLOR_NONE) 
            painter.set_color(-1,-1,-1);
        else
            painter.set_color(rr,gg,bb,alpha);

        double line_width = dataset->line_width;
        painter.set_line_width(line_width);
        painter.set_line_cap(dataset->line_cap);
        painter.set_dashes(dataset->num_dashes,
                           dataset->dashes);
        GivArrowType arrow = dataset->arrow_type;
        painter.set_arrow(arrow & ARROW_TYPE_START,
                          arrow & ARROW_TYPE_END);
        double old_x=-9e9, old_y=-9e9;
        bool need_paint = false;
        bool has_text = false; // Assume by default we don't have text

        // Whether we need a a separate sweep for text
        bool need_check_for_text = !(dataset->do_draw_lines
                                     || dataset->do_draw_marks);

        // Loop three times and draw as follows:
        //    It 0: Draw filled area of polygons.
        //    It 1: Draw contours of polygons and other line graphs
        //    It 2: Draw quiver
        bool has_ellipse = false;
        for (int i=0; i<3; i++) {
            ClippingPolygon poly;

            if ((i==0 && dataset->do_draw_polygon && dataset->color.alpha != COLOR_NONE)
                || (i==1 && dataset->do_draw_lines)
                || (i==2 && dataset->has_quiver)
                ) {
                // Set properties for quiver
                if (i==2) {
                    painter.set_arrow(false, dataset->quiver_head);
                    painter.set_line_width(2);
                }

                if (i==1 && dataset->do_draw_polygon
                    && !dataset->do_draw_polygon_outline)
                    continue;
                int n = (int)dataset->points->len;
                if (!n)
                    continue;
                for (int p_idx=0; p_idx<n+1; p_idx++) {
                    point_t p = g_array_index(dataset->points, point_t, p_idx%n);

                    double m_x = p.x * scale_x - shift_x;
                    double m_y = p.y * scale_y - shift_y;

                    if (p_idx==0 || p.op == Op::OP_MOVE) {
                        if (i<2 && p.op == Op::OP_MOVE) {
                            // is_closed is always true for polygons! 
                            add_clipped_poly(poly, dataset->do_draw_polygon);
                            poly.clear();
                            painter.new_path();
                            poly.push_back({m_x,m_y});
                        }
                    }

                    if (i < 2 && p.op == Op::OP_DRAW && (p_idx<n || dataset->do_draw_polygon)) {
                        double margin = line_width * 20;
                        build_clip_rect(margin);

                        poly.push_back({m_x,m_y});
                    }
                    else if (i < 2 && p.op == Op::OP_CURVE) {
                        // TBD - add clipping
                        double cpx0 = m_x, cpy0 = m_y;
                        p = g_array_index(dataset->points, point_t, p_idx+1);
                        double cpx1 = p.x * scale_x - shift_x;
                        double cpy1 = p.y * scale_y - shift_y;
                        p = g_array_index(dataset->points, point_t, p_idx+2);
                        p_idx += 2;
                        
                        m_x = p.x * scale_x - shift_x;
                        m_y = p.y * scale_y - shift_y;
  
                        painter.add_curve_segment(old_x, old_y,
                                                  cpx0, cpy0,
                                                  cpx1, cpy1,
                                                  m_x, m_y,
                                                  i==0);
                        need_paint = true;

                        // Needing for clipping of subsequent lineto
                        poly.clear();
                        poly.push_back({m_x,m_y});
                    }
                    else if (i < 2 && p.op == Op::OP_ELLIPSE) {
                        p_idx++; p_idx++;
                        has_ellipse = true;
                    }
                    else if (p.op == Op::OP_QUIVER) {
                        double qscale = dataset->quiver_scale * this->quiver_scale;
                        double q_x = old_x + p.x * scale_x * qscale;
                        double q_y = old_y + p.y * scale_y * qscale;
                        painter.new_path();
                        painter.add_line_segment(old_x, old_y, q_x, q_y,
                                                 false);
                        need_paint = true;
                    }
                    else if (p.op == Op::OP_TEXT) 
                        has_text = true;
#if 0
                    // When is this ever needed??? Ι need an example
                    else if (p.op == Op::OP_MOVE
                             && i > 0
                             && (dataset->do_draw_polygon_outline && dataset->do_draw_polygon)
                             && p_idx > 0) {
                        painter.add_line_segment(old_x,old_y,last_move_to_x,last_move_to_y);
                    }
#endif
                    else if (p.op == Op::OP_CLOSE_PATH)
                      {
                        double margin = line_width * 20;
                        build_clip_rect(margin);

                        add_clipped_poly(poly, true);
                        painter.close_path();
                        poly.clear();
                      }
                    old_x = m_x;
                    old_y = m_y;
                }

                if (i<2) { // draw polygon
                    add_clipped_poly(poly, true);
                    if (i==0)
                        painter.fill();
                }
                if (i==1 && dataset->do_draw_polygon) {
                    if (dataset->outline_color.alpha == COLOR_NONE) 
                        painter.set_color(-1,-1,-1);
                    else {
                        double rr_s = cs*dataset->outline_color.red;
                        double gg_s = cs*dataset->outline_color.green;
                        double bb_s = cs*dataset->outline_color.blue;
                        double alpha_s = cs*dataset->outline_color.alpha;

                        painter.set_color(rr_s,gg_s,bb_s,alpha_s);
                    }
                }
                if (i==2) {
                    // Todo: extract quiver color
                    double rr_s = cs*dataset->quiver_color.red;
                    double gg_s = cs*dataset->quiver_color.green;
                    double bb_s = cs*dataset->quiver_color.blue;
                    double alpha = cs*dataset->quiver_color.alpha;

                    painter.set_color(rr_s,gg_s,bb_s,alpha);
                }
                if (i>=1) {
                    if (dataset->do_draw_polygon)
                      painter.close_path();
                    painter.stroke();
                }
            }
        }
        if (has_ellipse) {
            for (int p_idx=0; p_idx<(int)dataset->points->len; p_idx++) {
                point_t p = g_array_index(dataset->points, point_t, p_idx);

                if (p.op == Op::OP_ELLIPSE) {
                    double x = p.x;
                    double y = p.y;
                    p_idx++;
                    p = g_array_index(dataset->points, point_t, p_idx);
                    double xsize = p.x;
                    double ysize = p.y;
                    p_idx++;
                    p = g_array_index(dataset->points, point_t, p_idx);
                    double angle = p.x;

                    double m_x = x * scale_x - shift_x;
                    double m_y = y * scale_y - shift_y;
                    double m_xsize = xsize * fabs(scale_x);
                    double m_ysize = ysize * fabs(scale_y);

                    painter.add_ellipse(m_x, m_y, m_xsize, m_ysize, angle);
                }
            }
            painter.draw_marks();
        }
        if (dataset->do_draw_marks)
          {
            GivMarkType mark_type = GivMarkType(dataset->mark_type);
            double mark_size_x = dataset->mark_size;
            double mark_size_y = mark_size_x;
            if (dataset->do_scale_marks)
              {
                mark_size_x *= fabs(scale_x);
                mark_size_y *= fabs(scale_y);
              }
            painter.set_color(rr,gg,bb,alpha);
            // Reset line width as it may have been changed for quiver
            painter.set_line_width(line_width);
            painter.set_svg_mark(dataset->svg_mark);
            
            for (int p_idx=0; p_idx<(int)dataset->points->len; p_idx++) {
              point_t p = g_array_index(dataset->points, point_t, p_idx);

              if (p.op == Op::OP_QUIVER)
                continue;
              if (p.op == Op::OP_TEXT)
                {
                  has_text = true;
                }
              else {
                double m_x = p.x * scale_x - shift_x;
                double m_y = p.y * scale_y - shift_y;

                // Crop marks 
                if (m_x < -mark_size_x || m_x > width+mark_size_x
                    || m_y < -mark_size_y || m_y > height+mark_size_y)
                  continue;
                if (dataset->svg_mark)
                  painter.add_svg_mark(m_x, m_y, scale_x, scale_y);
                else
                  {
                    painter.add_mark(mark_type,
                                     mark_size_x, mark_size_y,
                                     m_x, m_y);
                    need_paint = true;
                  }
              }
              if (need_paint)
                painter.draw_marks();
            }
        }
        if (need_check_for_text || has_text) {
            if (dataset->font_name)
                painter.set_font(dataset->font_name);
            else
                painter.set_font(DEFAULT_FONT);
            painter.set_text_angle(dataset->text_angle);
            if (dataset->text_size > 0 || dataset->do_scale_fonts) {
                double scale = 1.0;
                if (dataset->do_scale_fonts)
                    scale = scale_x;
                double font_size = dataset->text_size * scale;
                // A hack when font size has not been set for a scalable
                // font!
                double epsilon = 1e-9;
                if (font_size < epsilon)
                    font_size = 14 * scale;
                painter.set_text_size(font_size);
            }
            painter.set_color(rr,gg,bb,alpha);
            for (int p_idx=0; p_idx<(int)dataset->points->len; p_idx++) {
                point_t p = g_array_index(dataset->points, point_t, p_idx);

                if (p.op == Op::OP_TEXT)
                  {
                    double m_x = p.x * scale_x - shift_x;
                    double m_y = p.y * scale_y - shift_y;
                    const char *text = p.text_object->string;
                    int text_align = p.text_object->text_align;
                    if (dataset->text_style == TEXT_STYLE_DROP_SHADOW)
                      {
                        double rr = cs*dataset->shadow_color.red;
                        double gg = cs*dataset->shadow_color.green;
                        double bb = cs*dataset->shadow_color.blue;
                        double alpha = cs*dataset->shadow_color.alpha;

                        double shift_x = dataset->shadow_offset_x;
                        double shift_y = dataset->shadow_offset_y;
                        if (dataset->do_scale_fonts)
                          {
                            shift_x *= scale_x;
                            shift_y *= scale_y;
                          }
                        painter.set_color(rr,gg,bb,alpha);
                        painter.add_text(text,
                                         m_x+shift_x,
                                         m_y+shift_y,
                                         text_align,
                                         dataset->do_pango_markup);
                        // Reset color
                        rr = cs*dataset->color.red;
                        gg = cs*dataset->color.green;
                        bb = cs*dataset->color.blue;
                        alpha = cs*dataset->color.alpha;
                        painter.set_color(rr,gg,bb,alpha);
                      }
                    painter.add_text(text, m_x, m_y, text_align, dataset->do_pango_markup);
                }
            }
            painter.fill();
        }
    }
}

static inline gboolean
line_hor_line_intersect(double x0, double y0, double x1, double y1,
                        double line_x0, double line_x1, double line_y,
                        /* output */
                        double *x_cross, double *y_cross)
{
    if (y1 == y0) {
        *y_cross = x0;
        *x_cross = 0; /* Any x is a crossing */
        if (y1 == line_y)
            return TRUE;
        return FALSE;
    }
    
    *y_cross = line_y; /* Obviously! */
    *x_cross = x0 + (x1 - x0)*(line_y - y0)/(y1-y0);
    
    if (y1<y0) {
        double tmp = y0;
        y0=y1;
        y1=tmp;
    }
    
    return (*x_cross >= line_x0 && *x_cross <= line_x1 && *y_cross >= y0 && *y_cross <= y1);
}

static inline gboolean
line_ver_line_intersect(double x0, double y0, double x1, double y1,
                        double line_y0, double line_y1, double line_x,
                        /* output */
                        double *x_cross, double *y_cross)
{
    if (x1 == x0) {
        *x_cross = x0;
        *y_cross = 0; /* Any y is a crossing */
        if (x1 == line_x)
            return TRUE;
        return FALSE;
    }

    
    *x_cross = line_x; /* Obviously! */
    *y_cross = y0 + (y1 - y0)*(line_x - x0)/(x1-x0);

    if (x1<x0) {
        double tmp = x0;
        x0=x1;
        x1=tmp;
    }
    return (*y_cross >= line_y0 && *y_cross <= line_y1 && *x_cross >= x0 && *x_cross <= x1);
}

