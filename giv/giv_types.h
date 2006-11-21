#ifndef GIV_TYPES_H
#define GIV_TYPES_H

#include <gtk/gtk.h>

typedef struct {
  char *string;
  double x, y;
  double size;
} text_mark_t;

typedef struct {
  gint op;
  union {
    struct {
      gdouble x,y;
    } point;
    double arc_dev;
    text_mark_t *text_object;
  } data;
} point_t;

typedef struct {
  GdkColor color;
  GdkColor outline_color;
  gint line_width;
  gint line_style;
  gint mark_type;
  gint text_size;
  gint num_dashes;
  gdouble mark_size;
  gboolean do_scale_marks;
  gboolean do_draw_marks;
  gboolean do_draw_lines;
  gboolean do_draw_polygon;
  gboolean do_draw_polygon_outline;
  GArray *points;
  gchar *path_name;
  gchar *file_name;
  gchar *tree_path_string;
  gboolean is_visible;
  gint8 *dash_list;
  gchar *balloon_string;
} mark_set_t;

extern GtkWidget *image_viewer;
extern GPtrArray *mark_set_list;

#endif
