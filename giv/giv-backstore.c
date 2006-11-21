/**
 * @file   giv-backstore.c
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Sun Aug  7 09:49:35 2005
 * 
 * @brief  
 * 
 * 
 */
#include "giv-backstore.h"
#include <stdlib.h>
#include <stdio.h>

static int sign(int x);

giv_backstore_t *
new_giv_backstore (GtkWidget * ref_widget)
{
  giv_backstore_t *giv_backstore = g_new0 (giv_backstore_t, 1);
  GdkColor color;

  giv_backstore->ref_widget = ref_widget;
  giv_backstore->window = ref_widget->window;

  /* Set additional default parameters */
  giv_backstore->backing_store = NULL;
  giv_backstore->gc_backing_storage = NULL;
  giv_backstore->storage_coords = g_array_new (FALSE, FALSE, sizeof (GdkRectangle));

  giv_backstore->cairo = gdk_cairo_create (giv_backstore->window);
  giv_backstore->gc_window = gdk_gc_new (giv_backstore->window);
  gdk_gc_copy (giv_backstore->gc_window,
	       giv_backstore->ref_widget->style->
	       bg_gc[GTK_WIDGET_STATE (giv_backstore->ref_widget)]);

  /* Set the color of the lasso */
  gdk_color_parse ("Pink", &color);	/* Color of xgraph */
#if 0
  gdk_colormap_alloc_color (gdk_colormap_get_system (), &color, FALSE, TRUE);
  gdk_gc_set_foreground (giv_backstore->gc_window, &color);
  gdk_gc_set_line_attributes(giv_backstore->gc_window, 2, GDK_LINE_SOLID, GDK_CAP_ROUND,
			     GDK_JOIN_ROUND);
#endif
  giv_backstore->line_width = 2.0;
  giv_backstore->line_cap_extra_store = 0;
  cairo_set_line_width (giv_backstore->cairo, giv_backstore->line_width);
  cairo_set_line_cap (giv_backstore->cairo,
                      CAIRO_LINE_CAP_ROUND);
  cairo_set_source_rgba (giv_backstore->cairo,
                         1.0/65535*color.red,
                         1.0/65535*color.green,
                         1.0/65535*color.blue,
                         0.5);

  /* We still don't store anything */
  giv_backstore->has_store = FALSE;

  return giv_backstore;
}

static void
free_backing_storage (giv_backstore_t * giv_backstore)
{
  g_list_foreach (giv_backstore->backing_store,
                  (GFunc)gdk_pixmap_unref,
                  NULL);
  g_list_free(giv_backstore->backing_store);
  giv_backstore->backing_store = NULL;

  g_list_foreach (giv_backstore->gc_backing_storage,
                  (GFunc)gdk_gc_unref,
                  NULL);
  g_list_free(giv_backstore->gc_backing_storage);
  giv_backstore->gc_backing_storage = NULL;
}

void
free_giv_backstore (giv_backstore_t * giv_backstore)
{
  free_backing_storage (giv_backstore);
  gdk_gc_unref (giv_backstore->gc_window);
  g_free (giv_backstore);
}

int
giv_backstore_store_background_rect (giv_backstore_t * giv_backstore,
                                     int bbox[4])
{
  int side_idx;
  int width = abs(bbox[2] - bbox[0]);
  int height = abs(bbox[3] - bbox[1]);
  int line_width = (int)(giv_backstore->line_width+1);
  GdkRectangle rr;
  int lw_2 = line_width/2;
  if (lw_2 == 0)
      lw_2 = 1;

  // sanity check
  if (width == 0)
    width = line_width;
  if (height == 0)
    height = line_width;

  if (bbox[0] > bbox[2]) {
      int tmp = bbox[0];
      bbox[0] = bbox[2];
      bbox[2] = tmp;
  }

  if (bbox[1] > bbox[3]) {
      int tmp = bbox[1];
      bbox[1] = bbox[3];
      bbox[3] = tmp;
  }
  
  // Build the storage coordinates from  the bounding box
  g_array_set_size(giv_backstore->storage_coords,0);

  // 0: top
  rr.x = bbox[0]-lw_2;
  rr.y = bbox[1]-lw_2;
  rr.width = width+lw_2*2;
  rr.height = lw_2*2;
  g_array_append_val(giv_backstore->storage_coords, rr);

  // 1: right side
  rr.x = bbox[2]-lw_2;
  rr.y = bbox[1]-lw_2;
  rr.width = lw_2*2;
  rr.height = height+lw_2*2;
  g_array_append_val(giv_backstore->storage_coords, rr);

  // 2: bottom side
  rr.x = bbox[0]-lw_2;
  rr.y = bbox[3]-lw_2;
  rr.width = width+lw_2*2;
  rr.height = lw_2*2;
  g_array_append_val(giv_backstore->storage_coords, rr);

  // 3: left side
  rr.x = bbox[0]-lw_2;
  rr.y = bbox[1]-lw_2;
  rr.width = lw_2*2;
  rr.height = height+lw_2*2;
  g_array_append_val(giv_backstore->storage_coords, rr);

  // Now get the four areas
  free_backing_storage (giv_backstore);
  for (side_idx = 0; side_idx < 4; side_idx++)
    {
        GdkPixmap *pm = NULL;
        GdkGC *gc;
        GdkRectangle r = g_array_index(giv_backstore->storage_coords, GdkRectangle, side_idx);

        pm = gdk_pixmap_new (giv_backstore->window, r.width, r.height, -1);
        giv_backstore->backing_store
            = g_list_append(giv_backstore->backing_store,
                            pm);

        gc = gdk_gc_new (pm);
        giv_backstore->gc_backing_storage
            = g_list_append(giv_backstore->gc_backing_storage,
                            gc);
        gdk_draw_drawable (pm,
                           gc,
                           giv_backstore->window,
                           r.x, r.y, 0, 0, r.width, r.height);
    }

  giv_backstore->has_store = TRUE;

  return 0;
}

int
giv_backstore_restore_background (giv_backstore_t * giv_backstore)
{
  int side_idx;
  int n = giv_backstore->storage_coords->len;

  GList *gdl = giv_backstore->backing_store;
  for (side_idx = 0; side_idx < n; side_idx++)
    {
        GdkDrawable *gd = (GdkDrawable*)gdl->data;
        
        GdkRectangle r = g_array_index(giv_backstore->storage_coords,
                                       GdkRectangle,
                                       side_idx);
#if 0
        printf("Restorting r.x r.y r.width r.height = %d %d %d %d\n",
               r.x,r.y,r.width, r.height);
#endif
        gdk_draw_drawable (giv_backstore->window,
                           giv_backstore->gc_window,
                           gd,
                           0, 0, r.x, r.y, r.width, r.height);
        gdl = gdl->next;
    }

  // Get rid of the backing storage
  free_backing_storage (giv_backstore);
  giv_backstore->has_store = FALSE;
  g_array_set_size(giv_backstore->storage_coords, 0);

  return 0;
}

int
giv_backstore_draw_rect (giv_backstore_t * giv_backstore,
                         int x0, int y0,
                         int width, int height
                         )
{
  int bbox[4] = {x0,y0,x0+width,y0+height};

  if (giv_backstore->has_store)
    giv_backstore_restore_background (giv_backstore);

  giv_backstore_store_background_rect (giv_backstore, bbox);

#if 0
  for (side_idx = 0; side_idx < 4; side_idx++)
    {
        GdkRectangle rect = g_array_index(giv_backstore->storage_coords,
                                          GdkRectangle,
                                          side_idx);

        cairo_move_to (giv_backstore->cairo,rect.x,rect.y);
        cairo_line_to (giv_backstore->cairo,rect.x+rect.width,rect.y);
        cairo_line_to (giv_backstore->cairo,rect.x+rect.width,rect.y+rect.height);
        cairo_line_to (giv_backstore->cairo,rect.x,rect.y+rect.height);
        cairo_close_path(giv_backstore->cairo);

#if 0
        gdk_draw_rectangle (giv_backstore->window,
                            giv_backstore->gc_window,
                            TRUE, rect.x, rect.y, rect.width, rect.height);
#endif
    }
#endif
  cairo_move_to (giv_backstore->cairo,x0, y0);
  cairo_line_to (giv_backstore->cairo,x0+width, y0);
  cairo_line_to (giv_backstore->cairo,x0+width, y0+height);
  cairo_line_to (giv_backstore->cairo,x0, y0+height);
  cairo_close_path(giv_backstore->cairo);
  cairo_stroke (giv_backstore->cairo);

  return 0;
}

int
giv_backstore_draw_line (giv_backstore_t *giv_backstore,
                         int x0, int y0,
                         int x1, int y1)
{
  if (giv_backstore->has_store)
    giv_backstore_restore_background (giv_backstore);

  giv_backstore_store_background_line (giv_backstore, x0,y0,x1,y1);

#if 0
  gdk_draw_line (giv_backstore->window,
                 giv_backstore->gc_window,
                 x0,y0,
                 x1,y1);
#endif
  cairo_move_to (giv_backstore->cairo,x0,y0);
  cairo_line_to (giv_backstore->cairo,x1,y1);
  cairo_stroke (giv_backstore->cairo);
  
  return 0;
}

int
giv_backstore_store_background_line(giv_backstore_t *giv_backstore,
                                    int x0, int y0,
                                    int x1, int y1)
{
    int w = abs(x1-x0)+1;
    int h = abs(y1-y0)+1;
    int line_width = (int)(giv_backstore->line_width+1);
    int lw_2 = line_width/2;
    int side_idx;

    g_array_set_size(giv_backstore->storage_coords, 0);

    if (giv_backstore->line_cap_extra_store > 0) {
        int extra_store = giv_backstore->line_cap_extra_store; // shortcut
        GdkRectangle rr;
        rr.x = x0 - extra_store/2;
        rr.y = y0 - extra_store/2;
        rr.width = extra_store;
        rr.height = extra_store;
        
        g_array_append_val(giv_backstore->storage_coords,
                           rr);
        rr.x = x1 - extra_store/2;
        rr.y = y1 - extra_store/2;
        g_array_append_val(giv_backstore->storage_coords,
                           rr);
    }
    
    // 1st, 4th, 5th, 8th octants
    if (abs(y1-y0)<abs(x1-x0)) {
        double tan_angle, xs;
        int wr;

        if (x1 < x0) {
            int tmp = x0;
            x0 = x1;
            x1 = tmp;

            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        
        tan_angle = 1.0*(y1-y0)/(x1-x0);
        xs = -lw_2-2;

        if (y1 == y0)
            wr = w + line_width + 2;
        else
            wr = abs((int)(line_width/tan_angle));
        wr*=2;
#if 0
        printf("tan_angle wr = %f %d\n", tan_angle, (int)wr);
#endif
        while (xs < w+lw_2+2) {
            GdkRectangle rr;
            int hr = abs((int)(wr*tan_angle)) + line_width + 4;
            int ys = (int)(tan_angle * (xs+wr/2) - hr/2);

            rr.x = (int)(x0+xs);
            rr.y = (int)(y0+ys);
            rr.width = wr;
            rr.height = hr;

#if 0
            printf("  (x y w h) = (%d %d %d %d)\n",
                   rr.x,rr.y,rr.width,rr.height);
#endif
            g_array_append_val(giv_backstore->storage_coords,
                               rr);
            
            xs+= wr;
        }
    }
    // 1st, 4th, 5th, 8th octants
    else {
        double tan_angle, ys;
        int hr;

        if (y1 < y0) {
            int tmp = x0;
            x0 = x1;
            x1 = tmp;

            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        
        tan_angle = 1.0*(x1-x0)/(y1-y0);
        ys = -lw_2-2;

        if (x1 == x0)
            hr = h + line_width + 2;
        else
            hr = abs((int)(line_width/tan_angle));
        hr*=2;
#if 0
        printf("tan_angle wr = %f %d\n", tan_angle, (int)wr);
#endif
        while (ys < h+lw_2+2) {
            GdkRectangle rr;
            int wr = abs((int)(hr*tan_angle)) + line_width + 4;
            int xs = (int)(tan_angle * (ys+hr/2) - wr/2);

            rr.x = (int)(x0+xs);
            rr.y = (int)(y0+ys);
            rr.width = wr;
            rr.height = hr;

#if 0
            printf("  (x y w h) = (%d %d %d %d)\n",
                   rr.x,rr.y,rr.width,rr.height);
#endif
            g_array_append_val(giv_backstore->storage_coords,
                               rr);
            
            ys+= hr;
        }
    }
    
#if 0
    // Tile the path with rectangles. This should be changed to support
    // multiple line widths. There are several numbers here that are
    // totally heuristic...
    {
        int x,y;
        GdkRectangle rr;
        x=x0;
        y=y0;
        int s = 8; // rectangle size
        int overlap = 2;

	if (x1 < x) 
	    x = x1;
	if (y1 < y)
            y = y1;

	// 1st and 3rd quadrants must start at ymin
	int sgn = 1;
	if (sign(x1-x0) * sign(y1-y0) < 0) 
	    sgn = -1;

        g_array_set_size(giv_backstore->storage_coords, 0);

	if (w > h) {
	    if (h == 0) 
		s = w+1;
	    else {
		s *= w/h;
                if (s>w)
                    s = w+1;
	    }
            int sy = h*s/w+4+line_width;
            int xs = -1;
            int ystart = sgn < 0 ? h-sy+2 : -1;

	    while (xs < w) {
		double ys = 1.0* ystart + xs*sgn * h/w;

                rr.x = x+xs;
                rr.y = (int)(y+ys);
                rr.width = s;
                rr.height = sy;
                
                g_array_append_val(giv_backstore->storage_coords,
                                   rr);
		
		xs+= s-overlap;
	    }
	}
	else {
	    if (w == 0) {
		s = h+1;
	    }
	    else {
		s *= h/w;
                if (s>w)
                    s = h+1;
	    }
	    int sx = (int)(w*s/h + 4+line_width);
	    int ys = -1;
	    int xstart = sgn < 0 ? w-sx+2 : -1;

	    while(ys < h+s) {
		int xs = xstart + ys * sgn * w/h;

                rr.x = x+xs;
                rr.y = y+ys;
                rr.width = sx;
                rr.height = s;

                g_array_append_val(giv_backstore->storage_coords,
                                   rr);

		ys+= s-overlap;
	    }
	}
    }
#endif
    
    // Now get the areas - This code is identical to the code for the rectangles!
    int n = giv_backstore->storage_coords->len;
    free_backing_storage (giv_backstore);
    for (side_idx = 0; side_idx < n; side_idx++) {
        GdkPixmap *pm = NULL;
        GdkGC *gc;
        GdkRectangle r = g_array_index(giv_backstore->storage_coords, GdkRectangle, side_idx);

        pm = gdk_pixmap_new (giv_backstore->window, r.width, r.height, -1);
        giv_backstore->backing_store
            = g_list_append(giv_backstore->backing_store,
                            pm);

        gc = gdk_gc_new (pm);
        giv_backstore->gc_backing_storage
            = g_list_append(giv_backstore->gc_backing_storage,
                            gc);

        gdk_draw_drawable (pm,
                           gc,
                           giv_backstore->window,
                           r.x, r.y, 0, 0, r.width, r.height);
    }

  giv_backstore->has_store = TRUE;
  return 0;
}

int giv_backstore_set_color(giv_backstore_t* giv_backstore,
                            const char *color_name,
                            double alpha
                            )
{
    GdkColor color;
    gdk_color_parse (color_name, &color);
    cairo_set_source_rgba (giv_backstore->cairo,
                           1.0/65535*color.red,
                           1.0/65535*color.green,
                           1.0/65535*color.blue,
                           alpha);
    return 0;
}

int giv_backstore_set_line_width(giv_backstore_t *giv_backstore,
                                 gdouble line_width)
{
    giv_backstore->line_width = line_width;
    cairo_set_line_width (giv_backstore->cairo, giv_backstore->line_width);
    return 0;
}

int giv_backstore_set_line_cap_extra_store(giv_backstore_t *giv_backstore,
                                           gint extra_store)
{
    giv_backstore->line_cap_extra_store = extra_store;

    return 0;
}

#if 0
static int sign(int x)
{
    if (x < 0) 
	return -1;
    else if (x > 0)
	return 1;
    return 0;
}
#endif
