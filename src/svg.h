/*======================================================================
//  svg.h - 
//
//  Dov Grobgeld <dov.weizmann@weizmann.ac.il>
//  Sun Sep 18 21:51:07 2005
//----------------------------------------------------------------------
*/
#ifndef GIV_SVG_H
#define GIV_SVG_H

#include <gtk/gtk.h>

void             draw_image_in_svg(GtkWidget *widget, FILE *PS);
void draw_marks_in_svg(GtkWidget *image_viewer,
		       int canvas_width,
		       int canvas_height,
		       FILE *SVG);

#endif
