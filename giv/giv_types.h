#ifndef GIV_TYPES_H
#define GIV_TYPES_H

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
    text_mark_t *text_object;
  } data;
} point_t;

typedef struct {
  GdkColor color;
  gint line_width;
  gint line_style;
  gint mark_type;
  gdouble mark_size;
  gboolean do_scale_marks;
  gboolean do_draw_marks;
  gboolean do_draw_lines;
  GArray *points;
  gchar *path_name;
  gchar *file_name;
  gchar *tree_path_string;
  gboolean is_visible;
} mark_set_t;

extern GtkWidget *image_viewer;
extern GPtrArray *mark_set_list;

#endif
