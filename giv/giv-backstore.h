/**
 * @file   giv-backstore.h
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Sun Aug  7 09:47:27 2005
 * 
 * @brief  A class allowing interactive rectangles and lines on any
 *         gtk widget.
 * 
 * 
 */
#ifndef GIV_BACKSTORE_H
#define GIV_BACKSTORE_H

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget *ref_widget;
    GdkDrawable *window;
    GList *backing_store;               // (GdkPixmap)
    GList *gc_backing_storage;          // (GdkGC)
    GArray *storage_coords;             // (GdkRectangle)
    GdkGC *gc_window;
    cairo_t *cairo;
    gboolean has_store;
    gdouble line_width;
    gint line_cap_extra_store;       // Extra storage at line cap positions
} giv_backstore_t;

/** 
 * Used to store a rectangular region into a moving ants structure.
 * 
 * @param giv_backstore 
 * @param bbox 
 * 
 * @return 
 */
giv_backstore_t *new_giv_backstore (GtkWidget * ref_widget);
int giv_backstore_store_background (giv_backstore_t * giv_backstore, int bbox[4]);
int giv_backstore_draw_rect (giv_backstore_t * giv_backstore,
                             int x0, int y0,
                             int width, int height
                             );
int giv_backstore_restore_background (giv_backstore_t * giv_backstore);
void free_giv_backstore (giv_backstore_t *giv_backstore);

int
giv_backstore_draw_line (giv_backstore_t *giv_backstore,
                         int x0, int y0,
                         int x1, int y1);
int
giv_backstore_store_background_line(giv_backstore_t *giv_backstore,
                                    int x0, int y0,
                                    int x1, int y1);

int giv_backstore_set_color(giv_backstore_t* giv_backstore,
                            const char *color_name,
                            double alpha
                            );
int giv_backstore_set_line_width(giv_backstore_t *giv_backstore,
                                 gdouble line_width);

// If there are extra annotations (e.g. arrows) at the end of the lines
// this allows specifying an extra square to be stored in addition to the
// line.
int giv_backstore_set_line_cap_extra_store(giv_backstore_t *giv_backstore,
                                           gint extra_store);
#endif /* GIV-BACKSTORE */
