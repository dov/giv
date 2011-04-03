//======================================================================
//  GivPainterCairo.h - A painter for cairo. May be used for
//                      creating svg.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Apr 24 18:02:52 2008
//----------------------------------------------------------------------
#ifndef GIVPAINTERCAIRO_H
#define GIVPAINTERCAIRO_H

#include "GivPainter.h"

class GivPainterCairo : public GivPainter {
 public:
    GivPainterCairo(cairo_t *cairo = NULL,
                    bool do_antialiased = false);
    virtual ~GivPainterCairo();

    void set_cairo(cairo_t *cairo,
                   bool do_antialiased);
    void set_swap_blue_red(bool whether);
    virtual void set_set_idx(int set_idx);
    virtual void set_color(double red, double green, double blue, double alpha);
    virtual int set_line_width(double line_width);

    virtual int add_mark(GivMarkType mark_type,
                         double mark_size_x, double mark_size_y,
                         double x, double y);
    virtual int add_text(const char *text,
                         double x, double y,
                         int text_align,
                         bool do_pango_markup);
    virtual int add_line_segment(double x0, double y0,
                                 double x1, double y1,
                                 bool do_polygon=false);
    virtual void fill();
    virtual void stroke();
    virtual void draw_marks();
    void set_do_paint_by_index(bool do_paint_by_index);
    virtual int set_text_size(double text_size);
    virtual int set_font(const char* font_name);
    virtual void set_dashes(int num_dashes,
                            double* dashes);
    virtual void set_arrow(bool do_start_arrow,
                           bool do_end_arrow,
                           double arrow_d1=-1,
                           double arrow_d2=-1,
                           double arrow_d3=-1,
                           double arrow_d4=-1,
                           double arrow_d5=-1
                           );
    
    static void label_to_color(int label,
                               // output
                               double& rr,
                               double& gg,
                               double& bb);

 private:
    class Priv;
    Priv *d;
};

#endif /* GIVPAINTERCAIRO */
