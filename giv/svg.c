/*======================================================================
//  svg.c - svg output for giv
//
//  Dov Grobgeld <dov.weizmann@weizmann.ac.il>
//  Sun Sep 18 21:50:35 2005
//----------------------------------------------------------------------
*/

#include <stdio.h>
#include "giv.h"
#include "svg.h"

#define VDPI			72.0
#define LDIM			11.0
#define SDIM			8.5
#define MICRONS_PER_INCH	2.54E+04
#define POINTS_PER_INCH		72.0
#define INCHES_PER_POINT	1.0/72.0
#define DEV(val)        ((double)val * POINTS_PER_INCH / VDPI)
#define SVGY(y) (y)

void giv_print_svg(const char* filename,
		   GtkWidget *image_viewer,
		   int canvas_width,
		   int canvas_height
		   )
{
  FILE *SVG;

  if (!filename)
    return;
  SVG = fopen(filename, "w");
  if (!SVG)
    return;
  fprintf(stderr, "Creating file %s\n", filename);

  /* Print PROLOGUE */
  fprintf (SVG,
	   "<?xml version=\"1.0\" standalone=\"no\"?>\n"
	   "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20000303 Stylable//EN\"\n"
	   "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");

  /* This corresponds to bounding box */
  fprintf (SVG,
	   "<svg width=\"%.1f\" height=\"%.1f\">\n",
	   DEV (canvas_width), DEV (canvas_height));

  /* Print IMAGE */
  draw_image_in_svg(image_viewer,
		    SVG);

  /* Print MARKS */
  draw_marks_in_svg(image_viewer,
		    canvas_width,
		    canvas_height,
		    SVG);

  fprintf (SVG, "</svg>\n");

  fclose (SVG);
    
  fclose(SVG);
}

void draw_image_in_svg(GtkWidget *widget, FILE *SVG)
{
    
}

void draw_marks_in_svg(GtkWidget *image_viewer,
		       int canvas_width,
		       int canvas_height,
		       FILE *SVG)
{
  int set_idx;
  GdkColor current_color;
  double current_line_width = -1;
  double current_mark_size_x = -1;
  double current_mark_size_y = -1;
  double scale_x, scale_y;

  gtk_image_viewer_get_scale(image_viewer, &scale_x, &scale_y);
    
  if (!mark_set_list)
    return;
  if (!do_show_marks)
    return;

  for (set_idx=0; set_idx < mark_set_list->len; set_idx++) {
    mark_set_t *mark_set = g_ptr_array_index (mark_set_list, set_idx);
    gboolean is_first_point;
    gint p_idx;
    double old_cx=-1, old_cy=-1;
    double mark_size_x, mark_size_y;
    gint mark_type;
    gboolean in_path;
    int rr, gg, bb;

    if (!mark_set->is_visible)
      continue;
    
    /* Choose the color and line widths, etc*/
    if (set_idx == 0
	|| !color_eq(&current_color, &mark_set->color)) {
	  rr = (int)(1.0 * mark_set->color.red / 256);
	  gg = (int)(1.0 * mark_set->color.green / 256);
	  bb = (int)(1.0 * mark_set->color.blue / 256);

	  current_color = mark_set->color;
    }
	    
    if (current_line_width != mark_set->line_width)
      {
	current_line_width = mark_set->line_width;
      }

    /* Draw marks*/
    mark_size_x = mark_size_y = mark_set->mark_size;
    mark_type = mark_set->mark_type;
    if (mark_set->do_scale_marks) {
      mark_size_x *= scale_x;
      mark_size_y *= scale_y;
    }
	
    if (mark_set->do_draw_marks) {
      if (current_mark_size_x != mark_size_x
	  || current_mark_size_y != mark_size_y) {
	current_mark_size_x = mark_size_x;
	current_mark_size_y = mark_size_y;
      }
    }
	      
    is_first_point = TRUE;
    for (p_idx=0; p_idx<mark_set->points->len; p_idx++) {
      point_t p = g_array_index(mark_set->points, point_t, p_idx);
      double x = p.data.point.x;
      double y = p.data.point.y;
      double cx, cy;
      int op = p.op;
      in_path = FALSE;

      gtk_image_viewer_img_coord_to_canv_coord(image_viewer,
					       x,y,
					       &cx, &cy);
	    
#ifdef DEBUG_CLIP
      printf("scale = %.0f  x y = %g %g   cx cy = %.2f %.2f\n", scale, x,y, cx, cy);
#endif

      if (mark_set->do_draw_lines) {
	  if (is_first_point || op == OP_MOVE) {
	      fprintf (SVG, "<path d=\"");
	      fprintf (SVG, "M %f,%f ",
		       DEV (cx), DEV (SVGY (cy)));
	  }
	  if (!is_first_point) {
	      double x1,y1,x2,y2;
	      
	      if (clip_line_to_rectangle(old_cx, old_cy, cx, cy,
					 0, 0, canvas_width, canvas_height,
					 &x1,&y1,&x2,&y2)) {
		  if (mark_set->do_draw_marks || old_cx != x1 || old_cy != y1) {
		      if (cx < 0 || cy < 0
			  || cx > canvas_width || cy > canvas_height)
			  {}
		      else {
			  /* fix this to make postscript file smaller */
#if 0
			  if (in_path)
			      fprintf(SVG, "S\n");
			  fprintf(SVG, "%.4g %.4g M\n", x1,y1);
#endif
		      }
		  }
		  if (op == OP_DRAW) {
		      fprintf (SVG,
			       "M %f,%f L %f,%f ",
			       x1,y1,
			       x2,y2);
		  }
	      }
	  }
      }
      // Needs to be moved outside of loop
      fprintf (SVG, "\"/>\n</g>\n");
	  

      if (mark_set->do_draw_marks) {
	/* Trivial clipping for marks */
	if (   cx+mark_size_x > 0
	       && cy+mark_size_y > 0
	       && cx-mark_size_x < canvas_width
	       && cy-mark_size_y < canvas_height) {
	  if (mark_type == MARK_TYPE_CIRCLE)
	    fprintf(SVG, "%.4g %.4g mC\n", cx, cy);
	  else if (mark_type == MARK_TYPE_FCIRCLE)
	    fprintf(SVG, "%.4g %.4g mFC\n", cx, cy);
	  else if (mark_type == MARK_TYPE_SQUARE)
	    fprintf(SVG, "%.4g %.4g mS\n", cx, cy);
	  else if (mark_type == MARK_TYPE_FSQUARE)
	    fprintf(SVG, "%.4g %.4g mFS\n", cx, cy);
	  else
	    g_message("Unknown mark type %d!\n", (int)mark_type);
	}
      }
      is_first_point = FALSE;
      old_cx = cx;
      old_cy = cy;
    }
    if (in_path)
      fprintf(SVG, "S\n");
  }
    
}

