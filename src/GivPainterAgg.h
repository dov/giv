//======================================================================
//  GivPainterAgg.h - An agg painter
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Nov  6 22:06:56 2007
//----------------------------------------------------------------------

#ifndef GIV_PAINTER_AGG
#define GIV_PAINTER_AGG

#include "GivPainter.h"

class GivPainterAggPriv;
class GivPainterAgg : public GivPainter {
 public:
    GivPainterAgg(GdkPixbuf *pixbuf,
                  bool do_antialiased);
    virtual ~GivPainterAgg();

    virtual void set_set_idx(int set_idx);
    virtual void set_color(double red, double green, double blue, double alpha);
    virtual int set_line_width(double line_width);

    virtual int add_mark(GivMarkType mark_type,
                         double mark_size_x, double mark_size_y,
                         double x, double y);
    virtual int add_ellipse(double x, double y,
                            double sizex, double sizey,
                            double angle);
    virtual int add_text(const char *text,
                         double x, double y,
                         int text_align,
                         bool do_pango_markup);
    virtual int add_line_segment(double x0, double y0,
                                 double x1, double y1,
                                 bool do_polygon = false
                                 );
    virtual void fill();
    virtual void stroke();
    virtual void draw_marks();
    virtual int set_text_size(double text_size);
    virtual int set_font(const char* font);
    void set_do_paint_by_index(bool do_paint_by_index);
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
    GivPainterAggPriv *d;
};

#endif
