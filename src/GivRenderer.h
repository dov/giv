#ifndef GIVRENDERER_H
#define GIVRENDERER_H

#include <vector>
#include "GivPainter.h"

class GivRenderer {
 public:
    GivRenderer(GPtrArray* datasets,
                GivPainter& painter,
                double _scale_x,
                double _scale_y,
                double _shift_x,
                double _shift_y,
                double width,
                double height
                );
    void paint();

 private:
    GPtrArray *datasets;
    GivPainter& painter;
    double scale_x;
    double scale_y;
    double shift_x;
    double shift_y;
    double width;
    double height;
        
};

#endif /* GIVRENDERER */
