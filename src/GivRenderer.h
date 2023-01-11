#ifndef GIVRENDERER_H
#define GIVRENDERER_H

#include <vector>
#include "GivPainter.h"
#include "SutherlandHodgmanPolygonClipping.h"

class GivRenderer {
 public:
    GivRenderer(GPtrArray* datasets,
                GivPainter& painter,
                double _scale_x,
                double _scale_y,
                double _shift_x,
                double _shift_y,
                double width,
                double height,
                double _quiver_scale
                );
    void paint();
    void set_do_no_transparency(bool do_no_transparency)
    {
        this->do_no_transparency = do_no_transparency;
    }

 private:
    void add_clipped_poly(const sutherland_hodgeman_polygon_clipping::Polygon& poly, bool is_closed);
    void build_clip_rect(double margin);


    GPtrArray *datasets;
    GivPainter& painter;
    double scale_x;
    double scale_y;
    double shift_x;
    double shift_y;
    double width;
    double height;
    double quiver_scale;
    bool do_no_transparency;
    sutherland_hodgeman_polygon_clipping::Polygon clip_rect;
};

#endif /* GIVRENDERER */
