/*======================================================================
//  giv.h - Shared data for giv
//
//  Dov Grobgeld <dov.weizmann@weizmann.ac.il>
//  Sun Sep 18 22:10:28 2005
//----------------------------------------------------------------------
*/
#ifndef GIV_H
#define GIV_H

#include "giv_types.h"

extern GPtrArray *mark_set_list;
extern gboolean do_show_marks;

#define OP_MOVE 0
#define OP_DRAW 1
#define OP_TEXT 2
#define OP_ARC 3

#define MARK_TYPE_CIRCLE 1
#define MARK_TYPE_SQUARE 2
#define MARK_TYPE_FCIRCLE 3
#define MARK_TYPE_FSQUARE 4
#define MARK_TYPE_PIXEL 5
#define MARK_TYPE_ARC 6

gboolean         color_eq(GdkColor *color1, GdkColor *color2);
gboolean
clip_line_to_rectangle(double x0, double y0, double x1, double y1,
		       int rect_x0, int rect_y0, int rect_x1, int rect_y1,
		       /* output */
		       double *cx0, double *cy0, double *cx1, double *cy1);

#endif
