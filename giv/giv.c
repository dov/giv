/* 
 * Copyright (c) 2000 by Dov Grobgeld <dov@imagic.weizmann.ac.il>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

/*======================================================================
//  This is an advanced and currently very messy example of the use
//  of the gtk_image_viewer widget. 
//
//  Dov Grobgeld
//  5 Mar 2001
//----------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef G_PLATFORM_WIN32
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <strings.h>
#include <gtk_image_viewer/gtk_image_viewer.h>

// #define DEBUG_CLIP 1
#define CASE(s) if (!strcmp(s, S_))
#define NCASE(s) if (!g_strcasecmp(s, S_))

#define STR_EQUAL 0

#define OP_MOVE 0
#define OP_DRAW 1
#define OP_TEXT 2

#define STRING_DRAW 0
#define STRING_COMMENT 1
#define STRING_MOVE 2
#define STRING_TEXT 3
#define STRING_CHANGE_COLOR 5
#define STRING_CHANGE_LINE_WIDTH 6
#define STRING_CHANGE_MARKS 7
#define STRING_CHANGE_NO_LINE 8
#define STRING_CHANGE_SCALE_MARKS 9
#define STRING_CHANGE_MARK_SIZE 10
#define STRING_CHANGE_LINE 11
#define STRING_CHANGE_NO_MARK 12
#define STRING_IMAGE_REFERENCE 13
#define STRING_MARKS_REFERENCE 14
#define STRING_LOW_CONTRAST 15

#define MARK_TYPE_CIRCLE 1
#define MARK_TYPE_SQUARE 2
#define MARK_TYPE_FCIRCLE 3
#define MARK_TYPE_FSQUARE 4
#define MARK_TYPE_PIXEL 5

#define LOW_CONTRAST_LOW (128-32)
#define LOW_CONTRAST_HIGH (128+32)

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
} mark_set_t;

/* Some static configuration */
GdkColor set_colors[] =
  { {0, 0xffff, 0, 0},
    {0, 0, 0xffff, 0},  
    {0, 0, 0, 0xffff},
    {0, 0xffff, 0xffff, 0},
    {0, 0, 0xffff, 0xffff},
    {0, 0xffff, 0, 0xffff} };
gint nmarks_colors = 6;

GdkColor histogram_color = { 0, 0, 0, 0};

typedef enum {
  TRANS_FUNC_RESET,
  TRANS_FUNC_INVERT,
  TRANS_FUNC_NORM,
  TRANS_FUNC_EQ,
  TRANS_FUNC_CURVE,
  TRANS_FUNC_LOW_CONTRAST
} trans_func_t;

typedef enum {
  TRANSFORM_NONE,
  TRANSFORM_VFLIP,
  TRANSFORM_HFLIP,
  TRANSFORM_ROT180,
  TRANSFORM_ROT90,
  TRANSFORM_ROT270
} transform_t;

/*======================================================================
//  Forward declarations.
//----------------------------------------------------------------------*/
int			create_widgets    ();
static void		create_control_window    ();
static void		create_marks_window    ();
static void		create_print_window    ();
static void		create_goto_point_window    ();
static void		cb_quit		  (void);
static gint             cb_nextprev_image(gboolean do_prev);
static GPtrArray        *read_mark_set_list(GPtrArray *mark_file_name_list,
                                            /* output */
                                            GPtrArray *image_file_name_list);
/* static void		print_mark_set_list(GPtrArray *mark_set_list); */
static void		free_mark_set_list(GPtrArray *mark_set_list);
static mark_set_t*	new_mark_set      ();
static void             free_mark_set     (mark_set_t *mark_set);
static void		draw_marks	  (GtkImageViewer *drawing_area);
static void		gc_set_attribs	  (GdkGC *gc,
					   GdkColor *color,
					   gint line_width,
					   gint line_style);
static gint             cb_histogram_zoom (GtkWidget *widget,
					   GdkEventButton *event);
static gint             cb_histogram_expose_event   (GtkWidget *widget,
						     GdkEventExpose *event);
static void             giv_load_image(const char *img_name);
static void             giv_load_marks(const char *mark_file_name);
#ifndef G_PLATFORM_WIN32
static void		cb_sigint(int dummy);
#endif
static void		set_transfer_function(trans_func_t which_trans_func);
static void		cb_reset_image();
static void		cb_invert_image();
static void		cb_normalize_image();
static void		cb_equalize_image();
static void		cb_low_contrast_image();
static void		cb_show_histogram();
static void		cb_color_image();
static void		cb_red_only_image();
static void		cb_green_only_image();
static void		cb_blue_only_image();
static void		cb_toggle_marks();
static gint		cb_load_image();
static gint		cb_load_marks();
static void             shrink_wrap();
static void		draw_histogram();
static void		calc_histogram();
static gboolean         color_eq(GdkColor *color1, GdkColor *color2);
static void             init_globals();
static void             draw_marks_in_postscript(GtkImageViewer *image_viewer,
						 FILE *PS);
/* Private floor in order to work with Cygwin! */
static double           giv_floor(double x);
static void             set_last_directory_from_filename(const gchar *filename);
static void             add_filename_to_image_list(gchar *image_file_name,
                                                   GPtrArray *image_file_name_list);
static void             fit_marks_in_window();

/*======================================================================
//  Global variables. These really should be packed in a data structure.
//----------------------------------------------------------------------*/
gchar *img_name, *marks_name;
gboolean img_is_mono;
GtkWidget *w_window;
GtkImageViewer *image_viewer = NULL;
GtkWidget *w_info_label;
GtkWidget *w_control_window;
GdkPixbuf *img_org, *img_display;
GdkPixmap *histogram_drawing_pixmap;
int canvas_width, canvas_height;
double current_scale_x, current_scale_y, current_x0, current_y0;
GPtrArray *mark_set_list = NULL;
GPtrArray *mark_file_name_list;
GPtrArray *img_file_name_list;
int img_idx; /* The current image being displayed */
gboolean control_window_is_shown;
gboolean histogram_window_is_shown;
GtkWidget *w_histogram_window, *histogram_drawing_area;
gdouble histogram_scale;
gboolean default_draw_lines;
gboolean default_draw_marks;
gboolean default_scale_marks;
gint default_mark_type;
gdouble default_mark_size;
gint default_line_width;
double global_mark_max_x;
double global_mark_max_y;
double global_mark_min_x;
double global_mark_min_y;
double default_render_type;
guint8 current_maps[3][256];
gboolean do_mono;
gint color_component;
gboolean do_show_marks;
gboolean do_erase_img;
transform_t load_transformation;
gint hist[3][256]; // Image histogram
int histogram_height;
gchar *giv_last_directory = NULL;
trans_func_t giv_current_transfer_function = TRANS_FUNC_RESET;

struct {
  GtkWidget *filename_entry;
} print_dialog;

/*======================================================================
//  Functions.
//----------------------------------------------------------------------*/
int
main (int argc, char *argv[])
{
  int argp = 1;
  
  init_globals();
  
  mark_file_name_list = g_ptr_array_new ();
  img_file_name_list = g_ptr_array_new ();

  /* Parse arguments */
  while(argp < argc && argv[argp][0] == '-')
  {
    char *S_ = argv[argp++];
    
    CASE("-help") {
      printf("giv -  A gtk image viewer\n"
	     "\n"
	     "Syntax:\n"
	     "    giv [-marks marks] [-nl] [-P] [-ms ms] [-sm] [lw lw] [img] [-expand e]\n"
	     "\n"
	     "Description:\n"
	     "    giv is a program for viewing images and vector graphics. The vector\n"
	     "    graphics come in the form of marks and are stored in a marks \n"
	     "    file. The format is quite similar to xgraph in that you have\n"
	     "    datasets separated by newlines.\n"
	     "\n"
	     "    The major difference to xgraph is that you may chage the properties of these\n"
	     "    marks by lots of modifiers that are put in the beginning of the dataset\n"
	     "\n"
	     "Options:\n"
	     "    -marks markfile  Specify a marks file.\n"
	     "    -nl		 Don't draw lines by default.\n"
	     "    -ms ms		 Specify default mark size.\n"
	     "	-sm		 Marks should scale by default.\n"
	     "	-P		 Draw marks by default.\n"
	     "	-lw lw		 Default line width.\n"
	     "	-expand e	 Initial expansion.\n"
	     "\n"
	     "Example:\n"
	     "    Here is an example of a marks file:\n"
	     "       $COLOR pink3\n"
	     "       $NOLINE\n"
	     "       $SCALE_MARKS\n"
	     "       10 20\n"
	     "       20 20\n"
	     );
      exit(0);
    }
    CASE("-") { img_name = "-"; continue; }
    CASE("-expand") { current_scale_x = current_scale_y = atof(argv[argp++]); continue; }
    CASE("-scale") {
      current_scale_x = atof(argv[argp++]);
      current_scale_y = atof(argv[argp++]);
      continue;
    }
    CASE("-marks") {
      g_ptr_array_add(mark_file_name_list, strdup(argv[argp++]));
      continue;
    }
    CASE("-nl") { default_draw_lines = FALSE; continue; }
    CASE("-ms") { default_mark_size = atof(argv[argp++]); continue; }
    CASE("-sm") { default_scale_marks = TRUE; continue; }
    CASE("-lc") { giv_current_transfer_function = TRANS_FUNC_LOW_CONTRAST; continue; }
    CASE("-P")  { default_draw_marks = TRUE;  continue; }
    CASE("-pix")
      {
	default_draw_marks = TRUE;
	default_mark_type = MARK_TYPE_PIXEL;
	default_draw_lines = 0;
	continue;
      }
    CASE("-lw") { default_line_width = atoi(argv[argp++]); continue; }
    CASE("-rt") { default_render_type = atoi(argv[argp++]); continue; }
    CASE("-rot180") { load_transformation = TRANSFORM_ROT180; continue; }
    CASE("-vflip") { load_transformation = TRANSFORM_VFLIP; continue; }
    CASE("-vf") { load_transformation = TRANSFORM_VFLIP; continue; }
    CASE("-hflip") { load_transformation = TRANSFORM_HFLIP; continue; }
    fprintf(stderr, "Unknown option %s!\n", S_);
    exit(0);
  }

  gtk_init (&argc, &argv);
  {
    gchar rc_file[255];
    
    sprintf(rc_file, "%s/.giv/gtkrc", getenv("HOME"));
    gtk_rc_parse (rc_file);
  }
  
  if (!img_name)
    {
      while (argp<argc)
        {
          gchar *filename = strdup(argv[argp++]);
          
          /* Check if the filename ends with .marks */
          if (g_strrstr(filename, ".marks") != NULL
              || g_strrstr(filename, ".giv") != NULL
              )
            g_ptr_array_add(mark_file_name_list, filename);
          else
            g_ptr_array_add(img_file_name_list, filename);
        }
        
    }
  img_idx = 0;
  
  /* The image name is the first image */
  mark_set_list = read_mark_set_list(mark_file_name_list,
                                     img_file_name_list
                                     );
	
  if (img_file_name_list->len > 0)
    img_name = (gchar*)g_ptr_array_index (img_file_name_list, 0);

  create_widgets();

  if (img_file_name_list->len == 0)
    fit_marks_in_window();

#ifndef G_PLATFORM_WIN32
  {
    struct sigaction action;

    action.sa_handler = cb_sigint;
    memset(&action.sa_flags, 0, sizeof(sigset_t));

#ifdef LINUX
    sigaction(SIGINT, cb_sigint, NULL);
#else
    sigset(SIGINT, cb_sigint);
#endif
  }
#endif
  gtk_main ();

  return 0;
}

/*======================================================================
//  init_globals initializes the global variables as gcc doesn't like
//  doing this in the declarations.
//----------------------------------------------------------------------*/
static void init_globals()
{
  img_name = NULL;
  img_is_mono = FALSE;
  w_window = NULL;
  w_control_window = NULL;
  img_org = NULL;
  img_display = NULL;
  current_scale_x = 1;
  current_scale_y = 1;
  current_x0=0;
  current_y0=0;
  mark_file_name_list = NULL;
  control_window_is_shown = FALSE;
  histogram_window_is_shown = FALSE;
  histogram_scale = 1.0;
  default_draw_lines = TRUE;
  default_draw_marks = FALSE;
  default_scale_marks = FALSE;
  default_mark_type = 1;
  default_mark_size = 5;
  default_line_width = 1;
  default_render_type = -1;
  do_mono = FALSE;
  color_component = 0;
  do_show_marks = TRUE;
  do_erase_img = FALSE;
  load_transformation = TRANSFORM_NONE;
  histogram_height = 50;
}

/*======================================================================
//
// General string handling in C. These functions should be replaced
// by something in glib.
//
//----------------------------------------------------------------------*/
static int string_count_words(char *string)
{
  int nwords = 0;
  char *p = string;
  int in_word = 0;
  while(*p) {
    if (!in_word) {
      if ((*p != ' ') && (*p != '\n') && (*p != '\t'))
	in_word = 1;
    }
    else if (in_word) {
      if ((*p == ' ') || (*p == '\n') || (*p != '\t')) {
	in_word = 0;
	nwords++;
      }
    }
    p++;
  }
  if (in_word)
  nwords++;
  return nwords;
}

static char* string_strdup_word(const char *string, int idx)
{
  const char *p = string;
  int in_word = 0;
  int word_count = -1;
  int nchr = 1;
  char *word = NULL;
  const char *word_start = string;

  /* printf("p = 0x%x\n", p); */
  while(*p) {
    /* printf("%c\n", *p); fflush(stdout); */
    if (!in_word) {
      if (*p != ' ' && *p != '\n' && *p != '\t') {
	in_word = 1;
	word_count++;
	nchr = 1;
	word_start = p;
      }
    }
    else if (in_word) {
      if (*p == ' ' || *p == '\n' || *p == '\t') {
	if (idx == word_count)
	  break;
	in_word = 0;
      }
    }
    nchr++;
    p++;
  }
  if (in_word) {
    word = g_new(char, nchr);
    strncpy(word, word_start, nchr-1);
    word[nchr-1]=0;
  }
  return word;
}

static int string_to_atoi(char *string, int idx)
{
  char *word = string_strdup_word(string, idx);
  int value = atoi(word);
  g_free(word);

  return value;
}

static gdouble string_to_atof(char *string, int idx)
{
  char *word = string_strdup_word(string, idx);
  gdouble value = atof(word);
  g_free(word);

  return value;
}

/*======================================================================
//  Classify a string.
//----------------------------------------------------------------------
*/
gint parse_string(const char *string, char *fn, gint linenum)
{
  gint type = -1;
  gchar first_char = string[0];

  /* Shortcut for speeding up drawing */
  if (first_char >= '0' && first_char <= '9')
    return STRING_DRAW;

  if (first_char == '#') 
    type = STRING_COMMENT;
  else if (first_char == '$')
    {
      char *S_ = string_strdup_word(string, 0);
      NCASE("$lw")
        {
          type = STRING_CHANGE_LINE_WIDTH;
        }
      NCASE("$color")
        {
          type = STRING_CHANGE_COLOR;
        }
      NCASE("$marks")
        {
          type = STRING_CHANGE_MARKS;
        }
      NCASE("$noline")
        {
          type = STRING_CHANGE_NO_LINE;
        }
      NCASE("$scale_marks")
        {
          type = STRING_CHANGE_SCALE_MARKS;
        }
      NCASE("$mark_size")
        {
          type = STRING_CHANGE_MARK_SIZE;
        }
      NCASE("$nomark")
        {
          type = STRING_CHANGE_NO_MARK;
        }
      NCASE("$line")
        {
          type = STRING_CHANGE_LINE;
        }
      NCASE("$image")
        {
          type = STRING_IMAGE_REFERENCE;
        }
      NCASE("$mark_file")
        {
          type = STRING_MARKS_REFERENCE;
        }
      NCASE("$low_contrast")
        {
          type = STRING_LOW_CONTRAST;
        }
      if (type == -1)
        {
          fprintf(stderr, "Unknown parameter %s in file %s line %d!\n", S_, fn, linenum);
        }
      g_free(S_);
    }
  else if (first_char == 'M' || first_char=='m')
    {
      type = STRING_MOVE;
    }
  else if (first_char == 'T' || first_char=='t')
    {
      type = STRING_TEXT;
    }
  else
    {
      type = STRING_DRAW;
    }
  return type;
}

static gint
parse_mark_type(char *S_, gchar *fn, gint linenum)
{
  NCASE("circle")
    return MARK_TYPE_CIRCLE;
  NCASE("fcircle")
    return MARK_TYPE_FCIRCLE;
  NCASE("square")
    return MARK_TYPE_SQUARE;
  NCASE("fsquare")
    return MARK_TYPE_FSQUARE;
  NCASE("pixel")
    return MARK_TYPE_PIXEL;
  fprintf(stderr,"Unknown mark %s in file %s line %d\n", S_, fn, linenum);
  return MARK_TYPE_CIRCLE;
}

/*======================================================================
//  Read a mark set list. The mark set may also modify the image list
//  as it is possible to reference images in the mark set by the
//  $image: command.
//----------------------------------------------------------------------*/
static GPtrArray *
read_mark_set_list(GPtrArray *mark_file_name_list,
                   /* output */
                   GPtrArray *image_file_name_list
                   )
{
  GPtrArray *mark_set_list;
  FILE *IN;
  mark_set_t *marks = NULL;
  gboolean is_new_set;
  int num_sets = 0;
  int linenum = 0;
  int name_idx;
  double min_x = HUGE_VAL;
  double max_x = 0;
  double min_y = HUGE_VAL;
  double max_y = 0;

  mark_set_list = g_ptr_array_new ();
    
  for (name_idx =0; name_idx < mark_file_name_list->len; name_idx++)
    {
      gchar *fn = (gchar*)g_ptr_array_index (mark_file_name_list, name_idx);
      
      if (fn == NULL)
	continue;
      IN = fopen(fn, "r");
      if (!IN)
	return NULL;
      
      is_new_set = TRUE;
      while(!feof(IN))
	{
	  char S_[256];
	  char dummy[256];
	  gint type;
	  point_t p;
	  
	  linenum++;
	  fgets(S_, sizeof(S_), IN);
	  if (is_new_set)
	    {
	      marks = new_mark_set();
	      marks->color = set_colors[num_sets];
	      g_ptr_array_add(mark_set_list, marks);
	      is_new_set = FALSE;
	      num_sets++;
	    }
	  if (strlen(S_) == 1) {
	    if (marks && ((GArray*)marks->points)->len > 0)
	      is_new_set++;
	    continue;
	  }
	  
	  /* Parse the line */
	  type = parse_string(S_, fn, linenum);
	  switch (type) {
	  case STRING_COMMENT:
	    break;
	  case STRING_DRAW:
	  case STRING_MOVE:
	    if (type == STRING_DRAW)
              {
                sscanf(S_, "%lf %lf", &p.data.point.x, &p.data.point.y);
                p.op = OP_DRAW;
              }
	    else
              {
                sscanf(S_, "%s %lf %lf", dummy, &p.data.point.x, &p.data.point.y);
                p.op = OP_MOVE;
              }
	    
	    /* Find marks bounding box */
	    if (p.data.point.x < min_x)
	      min_x = p.data.point.x;
	    else if (p.data.point.x > max_x)
	      max_x = p.data.point.x;
	    if (p.data.point.y < min_y)
	      min_y = p.data.point.y;
	    else if (p.data.point.y > max_y)
	      max_y = p.data.point.y;
	    
	    g_array_append_val(marks->points, p);
	    break;
	  case STRING_TEXT:
	    {
	      text_mark_t *tm = (text_mark_t*)g_new(text_mark_t, 1);
	      tm->string = (char*)g_new(char, 64);
	      sscanf(S_, "%s %lf %lf %s", dummy, &tm->x, &tm->y, &tm->string);
	      p.op = OP_TEXT;
	      p.data.text_object = tm;
	      g_array_append_val(marks->points, p);
	    }
	    break;
	  case STRING_CHANGE_LINE_WIDTH:
	    marks->line_width = string_to_atoi(S_, 1);
	    break;
	  case STRING_IMAGE_REFERENCE:
              {
                  char *image_filename = string_strdup_word(S_, 1);
                  
                  // Todo: Make image relative to the marks list
                  add_filename_to_image_list(image_filename,
                                             image_file_name_list);
                  
                  free(image_filename);
                  break;
              }
	  case STRING_LOW_CONTRAST:
              {
		giv_current_transfer_function = TRANS_FUNC_LOW_CONTRAST;
		break;
              }
	  case STRING_CHANGE_NO_LINE:
	    marks->do_draw_lines = FALSE;
	    break;
	  case STRING_CHANGE_LINE:
	    marks->do_draw_lines = TRUE;
	    break;
	  case STRING_CHANGE_NO_MARK:
	    marks->do_draw_marks = FALSE;
	    break;
	  case STRING_CHANGE_MARK_SIZE:
	    marks->mark_size = string_to_atof(S_, 1);
	    break;
	  case STRING_CHANGE_COLOR:
	    {
	      char *color_name = string_strdup_word(S_, 1);
	      GdkColor color;
	      if (gdk_color_parse(color_name,&color))
		marks->color = color;
	      g_free(color_name);
	      break;
	    }
	  case STRING_CHANGE_MARKS:
	    {
	      char *mark_name = string_strdup_word(S_, 1);
	      marks->do_draw_marks = TRUE;
	      
	      marks->mark_type = parse_mark_type(mark_name, fn, linenum);
	      
	      g_free(mark_name);
	      break;
	    }
	  case STRING_CHANGE_SCALE_MARKS:
	    if (string_count_words(S_) == 1)
	      marks->do_scale_marks = 1;
	    else
	      marks->do_scale_marks = string_to_atoi(S_, 1);
	    break;
	  }
	}

      /* Get rid of empty data sets */
      if (marks && marks->points->len == 0)
	{
	  g_ptr_array_remove_index(mark_set_list, mark_set_list->len-1);
	  g_free(marks);
	}
      
      global_mark_max_x = max_x;
      global_mark_max_y = max_y;
      global_mark_min_x = min_x;
      global_mark_min_y = min_y;
    }
  
  return mark_set_list;
}

/*======================================================================
//  Add a filename to the image file name list. This function will
//  search the list if the filename already exists.
//----------------------------------------------------------------------*/
static void
add_filename_to_image_list(gchar *image_filename,
                           GPtrArray *image_filename_list)
{
  int i;
  for(i=0; i<image_filename_list->len; i++) {
    gchar *fn = g_ptr_array_index (mark_set_list, i);
    if (strcmp(image_filename, fn) == 0)
        return;
  }

  /* It doesn't exist. Add it. */
  g_ptr_array_add(image_filename_list, strdup(image_filename));
}

/*======================================================================
//  print_marks prints out the marks.
//----------------------------------------------------------------------*/
#if 0
static void
print_mark_set_list(GPtrArray *mark_set_list)
{
  int i;
  for(i=0; i<mark_set_list->len; i++) {
    mark_set_t *mark_set = g_ptr_array_index (mark_set_list, i);
    printf("Set #%d\n", i);
    printf("  num_points = %d\n", mark_set->points->len);
  }
}
#endif

/*======================================================================
//  destroy_marks destroys a mark list.
//----------------------------------------------------------------------*/
static void
free_mark_set_list(GPtrArray *mark_set_list)
{
  int i;
  if (mark_set_list) {
    for(i=0; i<mark_set_list->len; i++) {
      mark_set_t *mark_set = g_ptr_array_index (mark_set_list, i);
      free_mark_set(mark_set);
    }
    g_ptr_array_free(mark_set_list, FALSE);
  }
}

static mark_set_t*
new_mark_set()
{
  mark_set_t *marks;

  marks = g_new(mark_set_t, 1);
  marks->points = g_array_new(FALSE, FALSE, sizeof(point_t));
  marks->do_draw_marks = default_draw_marks;
  marks->do_draw_lines = default_draw_lines;
  marks->do_scale_marks = default_scale_marks;
  marks->mark_type = default_mark_type;
  marks->mark_size = default_mark_size;
  marks->line_width = default_line_width;
  marks->line_style = GDK_LINE_SOLID;
    
  return marks;
}

void free_mark_set(mark_set_t *marks)
{
  g_array_free(marks->points, TRUE);
  g_free(marks);
}

static gint
redraw_histogram(GtkWidget *widget)
{
  GdkEventExpose expose;

  if (!widget)
    return 0;
    
  expose.area.x = 0;
  expose.area.y = 0;
  expose.area.width = widget->allocation.width;
  expose.area.height = widget->allocation.height;

  cb_histogram_expose_event (widget, &expose);
  return 1;
}

/*======================================================================
//  Control window.
//----------------------------------------------------------------------*/
static void
toggle_control_window()
{
  if (control_window_is_shown) {
    gtk_widget_destroy(w_control_window);
    w_control_window = NULL;
  }
  else {
    create_control_window();
    gtk_widget_show(w_control_window);
  }
  control_window_is_shown = !control_window_is_shown;
}

/*======================================================================
//  Marks window.
//----------------------------------------------------------------------*/
gboolean marks_window_is_shown = FALSE;
GtkWidget *w_marks_window = NULL;

static void
cb_toggle_marks_window()
{
  if (marks_window_is_shown) {
    gtk_widget_destroy(w_marks_window);
    w_marks_window = NULL;
  }
  else {
    create_marks_window();
    gtk_widget_show(w_marks_window);
  }
  marks_window_is_shown = !marks_window_is_shown;
}


/*======================================================================
//  Print window.
//----------------------------------------------------------------------*/
gboolean print_window_is_shown = FALSE;
GtkWidget *w_print_window = NULL;

static void
cb_toggle_print_window()
{
  if (print_window_is_shown) {
    gtk_widget_destroy(w_print_window);
    w_print_window = NULL;
  }
  else {
    create_print_window();
    gtk_widget_show(w_print_window);
  }
  print_window_is_shown = !print_window_is_shown;
}


/*======================================================================
//  Goto point window.
//----------------------------------------------------------------------*/
gboolean goto_point_window_is_shown = FALSE;
GtkWidget *w_goto_point_window = NULL;

static void
cb_toggle_goto_point_window()
{
  if (goto_point_window_is_shown) {
    gtk_widget_destroy(w_goto_point_window);
    w_goto_point_window = NULL;
  }
  else {
    create_goto_point_window();
    gtk_widget_show(w_goto_point_window);
  }
  goto_point_window_is_shown = !goto_point_window_is_shown;
}

/*======================================================================
//  Event callbacks.
//----------------------------------------------------------------------*/

#ifndef G_PLATFORM_WIN32
/* Signal handler for SIGINT */
static void
cb_sigint(int dummy)
{
  giv_load_image(img_name);
}
#endif

static void
cb_quit()
{
  if (do_erase_img)
    unlink(img_name);
  exit(0);
}

static
gint cb_load_cancel(gpointer dummy1,
		    GtkWidget *dialog_window)
{
  gtk_widget_destroy(dialog_window);
  return 1;    
}

static
gint cb_load_image_ok(gpointer dummy1,
		      GtkWidget *dialog_window)
{
  GtkFileSelection *file_selection_window = GTK_FILE_SELECTION(dialog_window);
  const gchar *fn = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_window));
  GString *info_label = g_string_new("");
  
  g_string_sprintf(info_label, "Loading file %s", fn);
  set_last_directory_from_filename(fn);
  gtk_label_set(GTK_LABEL(w_info_label), info_label->str);
  g_string_free(info_label, TRUE);
  
  giv_load_image(fn);
  cb_reset_image();
  
  return 1;
}

static gint
cb_load_image()
{
  GtkFileSelection *window = GTK_FILE_SELECTION(gtk_file_selection_new("giv: load image"));

  gtk_file_selection_hide_fileop_buttons(window);
  if (giv_last_directory)
      gtk_file_selection_set_filename(window, giv_last_directory);
  
  g_signal_connect (G_OBJECT (window->ok_button),
		    "clicked", GTK_SIGNAL_FUNC(cb_load_image_ok),
		    window);

  g_signal_connect (G_OBJECT (window->cancel_button),
		    "clicked", GTK_SIGNAL_FUNC(cb_load_cancel),
		    window);

  gtk_widget_show (GTK_WIDGET(window));

  return 0;
}

static
gint cb_load_marks_ok(gpointer dummy1,
		      GtkWidget *dialog_window)
{
  GtkFileSelection *file_selection_window = GTK_FILE_SELECTION(dialog_window);
  const gchar *fn = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selection_window));
    
  giv_load_marks(fn);
  set_last_directory_from_filename(fn);
  
  /*    redraw(drawing_area); */
  
  return 1;
}

static gint
cb_load_marks()
{
  GtkFileSelection *window = GTK_FILE_SELECTION(gtk_file_selection_new("giv: load marks"));

  gtk_file_selection_hide_fileop_buttons(window);
  if (giv_last_directory)
      gtk_file_selection_set_filename(window, giv_last_directory);

  gtk_signal_connect (GTK_OBJECT (window->ok_button),
		      "clicked", GTK_SIGNAL_FUNC(cb_load_marks_ok),
		      window);

  gtk_signal_connect (GTK_OBJECT (window->cancel_button),
		      "clicked", GTK_SIGNAL_FUNC(cb_load_cancel),
		      window);

  gtk_widget_show (GTK_WIDGET(window));

  return 0;
}

/* Create a new backing pixmap of the appropriate size */
static gint
cb_configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
  canvas_width = widget->allocation.width;
  canvas_height = widget->allocation.height;
    
  return TRUE;
}

static gint
cb_configure_histogram_event (GtkWidget *widget, GdkEventConfigure *event)
{
  if (histogram_drawing_pixmap)
    gdk_pixmap_unref(histogram_drawing_pixmap);

  histogram_height = widget->allocation.height;

  histogram_drawing_pixmap = gdk_pixmap_new(widget->window,
					    256, histogram_height, -1);
    
  /* And draw it to the background pixmap */
  gdk_draw_rectangle(histogram_drawing_pixmap,
		     widget->style->bg_gc[GTK_WIDGET_STATE (widget)],
		     TRUE,
		     0,0,256, histogram_height);
    
  draw_histogram();
    
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  histogram_drawing_pixmap,
		  0, 0, 0, 0,
		  256, histogram_height);
    
  gtk_widget_grab_focus(widget);
  return TRUE;
}

static gint
cb_histogram_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
  if (!widget)
    return FALSE;

  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  histogram_drawing_pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  return FALSE;
}

static gint
cb_key_press_event(GtkWidget *widget, GdkEventKey *event)
{
  gint k = event->keyval;
    
  if (k == 'm')
    cb_toggle_marks_window();
  else if (k == 'h')
    cb_equalize_image();
  else if (k == 'l')
    cb_low_contrast_image();
  else if (k == 'r')
    cb_reset_image();
  else if (k == 'i')
    cb_invert_image();
  else if (k == 'N')
    cb_normalize_image();
  else if (k == 't')
    shrink_wrap();
  else if (k == 'q')
    cb_quit();
  else if (k == 'g')
    cb_toggle_goto_point_window();
  else if (k == ' ')
    cb_nextprev_image(FALSE);
  else if (k == GDK_BackSpace)
    cb_nextprev_image(TRUE);
  return FALSE;
}

static gint
cb_nextprev_image(gboolean do_prev)
{
  /* printf("nextprev.\n"); */
  if (do_prev)
    {
      if (img_idx == 0)
	return FALSE;
      img_idx--;
    }
  else
    {
      if (img_idx >= img_file_name_list->len-1)
	return FALSE;
      img_idx++;
    }

  img_name = (gchar*)g_ptr_array_index (img_file_name_list, img_idx);
  giv_load_image(img_name);
  cb_reset_image();

  return 0;
}

static gint
cb_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
  int button = event->button;
  gboolean is_signal_caught = FALSE;
    
  if (button == 1) {
  }
  else if (button == 2) {
  }
  else if (button == 3) {
    if (!(event->state & GDK_CONTROL_MASK)) {
      toggle_control_window();
      is_signal_caught= TRUE;
    }
  }
  if (is_signal_caught)
    gtk_signal_emit_stop_by_name(GTK_OBJECT(widget), "button_press_event");
    
  return FALSE;
}

static gint
cb_motion_event(GtkWidget *widget, GdkEventButton *event)
{
  GtkImageViewer *image_viewer = GTK_IMAGE_VIEWER(widget);
  int cx = event->x;
  int cy = event->y;
  double x,y;
  GString *info_label = g_string_new("");
  
  /* Don't update the coordinate if we are panning */
  if (gtk_image_viewer_get_is_panning(image_viewer))
    return FALSE;

  gtk_image_viewer_canv_coord_to_img_coord(GTK_IMAGE_VIEWER(image_viewer),
					   cx, cy, &x, &y);
  
  g_string_sprintf(info_label, " (%d, %d)", (int)giv_floor(x),(int)giv_floor(y));
  
  if (img_display)
    {
      int width = gdk_pixbuf_get_width(img_display);
      int height = gdk_pixbuf_get_height(img_display);
      int row_stride = gdk_pixbuf_get_rowstride(img_display);
      guint8 *buf = gdk_pixbuf_get_pixels(img_display);
      guint8 *rgb;

      if (x>=0 && x<width && y>=0 && y<height)
	{
	  rgb = &buf[((int)y)*row_stride+((int)x)*3];
	  
	  if (img_is_mono)
	    g_string_sprintfa(info_label,
			      " [%3d] = #%2X",
			      rgb[0],
			      (int)rgb[2]);
	  else 
	    g_string_sprintfa(info_label,
			      " [%3d %3d %3d] = #%6X",
			      rgb[0], rgb[1], rgb[2],
			      (int)(rgb[0]<<16) + (rgb[1]<<8) + rgb[2]);
	}
    }
  
  gtk_label_set(GTK_LABEL(w_info_label), info_label->str);
  g_string_free(info_label, TRUE);
  
  return FALSE;
}

static gboolean
giv_check_img_for_mono(GdkPixbuf *im)
{
  int pix_idx;
  int width = gdk_pixbuf_get_width(im);
  int height = gdk_pixbuf_get_height(im);
  guint8 *buf = gdk_pixbuf_get_pixels(im);

  for(pix_idx=0; pix_idx<width * height; pix_idx++) {
    if (buf[0] != buf[1]
	|| buf[0] != buf[2])
      return FALSE;
    buf+= 3;
  }
  return TRUE;
}

static void
img_flip_vertical(GdkPixbuf *im)
{
  int w = gdk_pixbuf_get_width(im);
  int h = gdk_pixbuf_get_height(im);
  guint8 *buf = gdk_pixbuf_get_pixels(im);
  int row_idx;
  int col_idx;

  for (row_idx=0; row_idx<h/2; row_idx++) {
    guint8 *ptr1 = buf+row_idx * w * 3;
    guint8 *ptr2 = buf+(h-row_idx-1) * w * 3;

    for (col_idx=0; col_idx< w; col_idx++) {
      guint8 tmp_r = *ptr1;
      guint8 tmp_g = *(ptr1+1);
      guint8 tmp_b = *(ptr1+2);
      *ptr1++ = *ptr2;
      *ptr1++ = *(ptr2+1);
      *ptr1++ = *(ptr2+2);
      *ptr2++ = tmp_r;
      *ptr2++ = tmp_g;
      *ptr2++ = tmp_b;
    }
  }
}

static void
img_flip_horizontal(GdkPixbuf *im)
{
  int w = gdk_pixbuf_get_width(im);
  int h = gdk_pixbuf_get_height(im);
  guint8*buf = gdk_pixbuf_get_pixels(im);
  int row_idx;
  int col_idx;
  int clr_idx;

    
  for (col_idx = 0; col_idx < w/2; col_idx++) {
    for (row_idx = 0; row_idx <h; row_idx++) {
      int l_idx = (row_idx * w + col_idx)*3;
      int r_idx = (row_idx * w + (w - col_idx - 1))*3;

      for (clr_idx=0; clr_idx<3; clr_idx++) {
	guint8 tmp = buf[l_idx+clr_idx];
	buf[l_idx+clr_idx] = buf[r_idx+clr_idx];
	buf[r_idx+clr_idx] = tmp;
      }
    }
  }
}

static void
giv_load_image(const char *new_img_name)
{
  GError *error = NULL;

#ifndef G_PLATFORM_WIN32  
  gchar *temp_name = NULL;

  if (strcmp(new_img_name, "-") == 0) {
    FILE *IMG;
    guint8 buf[2048];
    gint size;
	
    do_erase_img++;
    temp_name = g_strdup_printf("/tmp/giv%d", (int)getpid());
    IMG = fopen(temp_name, "w");

    while((size = fread(buf,1, sizeof(buf), stdin))) 
      fwrite(buf, 1, size, IMG);
	
    fclose(IMG);
  }
#endif
  
  img_org=gdk_pixbuf_new_from_file (new_img_name,
				    &error
				    );
#ifndef G_PLATFORM_WIN32
  if (temp_name)
    unlink(temp_name);
#endif
  
  if (!img_org && error != NULL)
    {
      fprintf(stderr, "No such image %s found!\n", new_img_name);
      return;
    }
  img_display = img_org;
  img_is_mono = giv_check_img_for_mono(img_display);
    
  switch (load_transformation) {
  case TRANSFORM_NONE: break;
  case TRANSFORM_ROT180:
    img_flip_vertical(img_display);
    img_flip_horizontal(img_display);
    break;
  case TRANSFORM_VFLIP:
    img_flip_vertical(img_display);
    break;
  case TRANSFORM_HFLIP:
    img_flip_horizontal(img_display);
    break;
  default:
    break;
    /* Ignore other transforms */
  }
    
  if (new_img_name != img_name)
    {
      if (img_name)
	free(img_name);
      img_name = strdup(new_img_name);
    }
  calc_histogram();

  shrink_wrap();
  if (image_viewer)
    gtk_image_viewer_set_image(GTK_IMAGE_VIEWER(image_viewer), img_display);

  if (w_window)
    {
      gtk_window_set_title(GTK_WINDOW(w_window), img_name);
      cb_reset_image();
    }
}

static void
giv_load_marks(const char *mark_file_name)
{
  free_mark_set_list(mark_set_list);

  g_ptr_array_add(mark_file_name_list, strdup(mark_file_name));
  mark_set_list = read_mark_set_list(mark_file_name_list,
                                     img_file_name_list);

  if (w_window)
    {
      gtk_window_set_title(GTK_WINDOW(w_window), img_name);
      cb_reset_image();
    }
}

/*======================================================================
//  Make the giv window exactly big enough for the contents.
//----------------------------------------------------------------------*/
static void
shrink_wrap()
{
  int new_width, new_height;
  gint s_width, s_height;
  gdouble scale_x, scale_y;

  if (!image_viewer)
    return;
    
  gtk_image_viewer_get_scale(image_viewer,
			     &scale_x, &scale_y
			     );


  if (!w_window || !img_display)
    return;

  s_width = gdk_screen_width ();
  s_height = gdk_screen_height ();
    
  new_width = gdk_pixbuf_get_width(img_display) * scale_x;
  new_height = gdk_pixbuf_get_height(img_display) * scale_y;

  if (new_width > 0.75*s_width)
    new_width = 0.75*s_width;
  if (new_height > 0.75*s_height)
    new_height = 0.75*s_height;

  gtk_widget_set_size_request(GTK_WIDGET(image_viewer), new_width, new_height);
  /*  gtk_widget_set_usize (GTK_WIDGET(image_viewer), new_width, new_height); */
}

/*======================================================================
//  create_widgets is responsible for creating the user inteface.
//----------------------------------------------------------------------*/
int create_widgets()
{
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *button_box;
  gint w,h;

  gdk_rgb_init ();
  gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
  gtk_widget_set_default_visual (gdk_rgb_get_visual ());
    
  /* Toplevel */
  w_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (w_window), "destroy",
		      GTK_SIGNAL_FUNC (cb_quit), NULL);
  gtk_container_border_width (GTK_CONTAINER (w_window), 3);

  /* Load the image specified as the first argument or create a grey
     image. */
  if (img_name) {
    giv_load_image(img_name);
    if (img_display)
      {
	w=gdk_pixbuf_get_width(img_display)*current_scale_x;
	h=gdk_pixbuf_get_height(img_display)*current_scale_y;

        /* Add some margin to get around windows bug */
	w+= 40;
	h+= 40;
      }
    else
      w = h = 100;
  }
  else
    {
      guchar *data;
      int width = 500;
      int height = 500;
      
      if (global_mark_max_x < width && global_mark_max_x > 100)
        width = (int)global_mark_max_x;
      if (global_mark_max_y < height && global_mark_max_y > 100)
        height = (int)global_mark_max_y;

      w = width*current_scale_x; h = height*current_scale_y;
      data = g_new(guchar, w * h*3);
      memset(data, 190, width * height*3);
      g_free(data);
      
      img_name = "marks view";
    }
  
  gtk_window_set_title(GTK_WINDOW(w_window), img_name);
  gtk_window_set_policy(GTK_WINDOW(w_window), TRUE, TRUE, TRUE);
    
  /* Suck the image's original width and height out of the Image structure */
  if (w>gdk_screen_width() * 0.75)
    w=gdk_screen_width() * 0.75;
  if (h>gdk_screen_width() * 0.75)
    h=gdk_screen_width() * 0.75;

  /* A vbox for the rest of the widgets */
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add (GTK_CONTAINER (w_window), vbox);

  /* My image drawing widget */
  image_viewer = GTK_IMAGE_VIEWER(gtk_image_viewer_new(img_display));
  gtk_widget_set_double_buffered(GTK_WIDGET(image_viewer), FALSE);
  if (!img_display)
    gtk_image_viewer_set_zoom_range(GTK_IMAGE_VIEWER(image_viewer),1.0e-6,1e6);

  gtk_widget_set_size_request(GTK_WIDGET(image_viewer), w, h);
  gtk_image_viewer_zoom_around_fixed_point(GTK_IMAGE_VIEWER(image_viewer),
					   current_scale_x,
					   current_scale_y,
					   w/2,h/2,w/2,h/2);
  shrink_wrap();

  /* Put image viewer in a scrolled window */
  {
    GtkWidget *scrolled_win;
    GtkAdjustment *hadjust, *vadjust;

    hadjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 0.1, 0.1, 0.5));
    gtk_image_viewer_set_hadjustment(GTK_IMAGE_VIEWER(image_viewer),
				     hadjust);
    
    vadjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 0.1, 0.1, 0.5));
    gtk_image_viewer_set_vadjustment(GTK_IMAGE_VIEWER(image_viewer),
				     vadjust);

    scrolled_win = gtk_scrolled_window_new(hadjust, vadjust);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
    
    gtk_container_add (GTK_CONTAINER (scrolled_win), GTK_WIDGET(image_viewer));
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(scrolled_win),
			TRUE, TRUE, 0);
  }
    
  /* events */
  gtk_signal_connect (GTK_OBJECT(image_viewer),     "key_press_event",
		      (GtkSignalFunc)		      cb_key_press_event
		      , NULL);
  gtk_signal_connect (GTK_OBJECT (image_viewer),    "button_press_event",
		      (GtkSignalFunc)		      cb_button_press_event,
		      NULL);
  gtk_signal_connect (GTK_OBJECT (image_viewer),    "motion_notify_event",
		      (GtkSignalFunc)		      cb_motion_event,
		      NULL);
  gtk_signal_connect (GTK_OBJECT(image_viewer),     "configure_event",
		      (GtkSignalFunc)		      cb_configure_event,
		      NULL);
  gtk_signal_connect (GTK_OBJECT(image_viewer),     "view_changed",
		      (GtkSignalFunc)		      draw_marks,
		      NULL);
  gtk_widget_set_events(GTK_WIDGET(image_viewer),
			GDK_EXPOSURE_MASK
			| GDK_STRUCTURE_MASK
			| GDK_PROPERTY_CHANGE_MASK
			| GDK_POINTER_MOTION_MASK 
			| GDK_BUTTON_PRESS_MASK
			| GDK_KEY_PRESS_MASK
			);
#if 0
  gtk_signal_connect (GTK_OBJECT (drawing_area),    "expose_event",
		      (GtkSignalFunc)		      cb_expose_event,
		      NULL);
#endif

  /* This is necessary in order to be able to focus the drawing area */
  GTK_WIDGET_SET_FLAGS (image_viewer, GTK_CAN_FOCUS);

  /* button box */
  button_box = gtk_hbox_new (0, 5);
  gtk_box_pack_start (GTK_BOX (vbox), button_box, FALSE, FALSE, 0);

  /* Quit button */
  button = gtk_button_new_with_label ("Quit");
  gtk_box_pack_start (GTK_BOX (button_box), button, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (cb_quit), NULL);

  /* Info box */
  w_info_label = gtk_label_new("");
  gtk_box_pack_start (GTK_BOX (button_box), w_info_label, FALSE, FALSE, 0);

  /* Show all the widgets */
  gtk_widget_show_all (w_window);

  /* Create widgets that are not shown */
  create_control_window();

  /* Misc setup */
  if (img_display)
     set_transfer_function(giv_current_transfer_function);
    
  return 0;
}

static void
create_marks_window()
{
  GtkWidget *vbox, *button;

  if (w_marks_window)
    gtk_widget_destroy(w_marks_window);
    
  w_marks_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_border_width (GTK_CONTAINER (w_marks_window), 3);

  /* button box */
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add (GTK_CONTAINER (w_marks_window), vbox);
  gtk_widget_show(vbox);


  /* Toggle marks button */
  button = gtk_button_new_with_label("Toggle marks");
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (cb_toggle_marks), NULL);
  gtk_widget_show(button);

  /* Load marks */
  button = gtk_button_new_with_label("Load marks");
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (cb_load_marks), NULL);
  gtk_widget_show(button);
}

static void
giv_print()
{
  /* Get name of file */
  const char *filename = gtk_entry_get_text(GTK_ENTRY(print_dialog.filename_entry));
  FILE *PS;

  if (!filename)
    return;
  PS = fopen(filename, "w");
  if (!PS)
    return;
  fprintf(stderr, "Creating file %s\n", filename);

  /* Print PROLOGUE */
  fprintf(PS,
	  "%%!\n"
	  );

  /* transformation matrix */
  fprintf(PS, "%d %d translate\n", (595-canvas_width)/2,
	  (842+canvas_height)/2);

  fprintf(PS, "1 -1 scale\n");
  /* Print bounding box */
  fprintf(PS, "0 0 moveto\n");
  fprintf(PS, "%d 0 lineto\n", canvas_width);
  fprintf(PS, "%d %d lineto\n", canvas_width, canvas_height);
  fprintf(PS, "0 %d lineto\n", canvas_height);
  fprintf(PS, "closepath clip stroke\n");

  /* Print IMAGE */

  /* Print MARKS */
  draw_marks_in_postscript(image_viewer,
			   PS);

  fprintf(PS, "showpage\n");
    
  fclose(PS);
}

static void
create_print_window()
{
  GSList *print_destination_group = NULL;
  GtkWidget *vbox, *hbox;
  GtkWidget *table1, *printer_name, *file_name, *radiobutton1, *radiobutton2;
  GtkWidget *button_print, *button_cancel;

  if (w_print_window)
    gtk_widget_destroy(w_print_window);

  w_print_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_border_width (GTK_CONTAINER (w_print_window), 3);

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add (GTK_CONTAINER (w_print_window), vbox);
  gtk_widget_show(vbox);
    
  table1 = gtk_table_new (2, 3, FALSE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "table1", table1,
			    (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox), table1, TRUE, TRUE, 0);

  printer_name = gtk_entry_new ();
  gtk_widget_ref (printer_name);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "Printer name", printer_name,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (printer_name);
  gtk_table_attach (GTK_TABLE (table1), printer_name, 1, 2, 1, 2,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);

  file_name = gtk_entry_new ();
  print_dialog.filename_entry = file_name;
  gtk_widget_ref (file_name);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "file_name", file_name,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (file_name);
  gtk_table_attach (GTK_TABLE (table1), file_name, 1, 2, 0, 1,
		    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);
    
  radiobutton1 = gtk_radio_button_new_with_label (print_destination_group, "File");
  print_destination_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton1));
  gtk_widget_ref (radiobutton1);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "radiobutton1", radiobutton1,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (radiobutton1);
  gtk_table_attach (GTK_TABLE (table1), radiobutton1, 0, 1, 0, 1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);

  radiobutton2 = gtk_radio_button_new_with_label (print_destination_group, "Printer");
  print_destination_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton2));
  gtk_widget_ref (radiobutton2);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "radiobutton2", radiobutton2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (radiobutton2);
  gtk_table_attach (GTK_TABLE (table1), radiobutton2, 0, 1, 1, 2,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (0), 0, 0);

#if 0
  button1 = gtk_button_new_with_label ("...");
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "button1", button1,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button1);
  gtk_table_attach (GTK_TABLE (table1), button1, 2, 3, 0, 1,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_ref (button2);
  gtk_object_set_data_full (GTK_OBJECT (w_print_window), "button2", button2,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button2);
  gtk_table_attach (GTK_TABLE (table1), button2, 2, 3, 1, 2,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
#endif

  /* print and quit buttons */
  hbox = gtk_hbox_new(FALSE,5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show(hbox);
    
  button_print = gtk_button_new_with_label("Print");
  gtk_box_pack_start (GTK_BOX (hbox), button_print, TRUE, TRUE, 0);
  gtk_widget_show(button_print);
  gtk_signal_connect (GTK_OBJECT (button_print),
		      "clicked", GTK_SIGNAL_FUNC(giv_print),
		      NULL);
    
  button_cancel = gtk_button_new_with_label("Cancel");
  gtk_box_pack_start (GTK_BOX (hbox), button_cancel, TRUE, TRUE, 0);
  gtk_widget_show(button_cancel);
  gtk_signal_connect (GTK_OBJECT (button_cancel),
		      "clicked", GTK_SIGNAL_FUNC(cb_toggle_print_window),
		      NULL);

  gtk_widget_show_all(w_print_window);

}

static void
giv_goto_point(GtkWidget *this)
{
  double x0 = atof(gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(this), "0"))))+0.5;
  double y0 = atof(gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(this), "1"))))+0.5;
  double zoom = atof(gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(this), "2"))));


  double old_canvas_x, old_canvas_y;
  double h = gtk_image_viewer_get_canvas_height(GTK_IMAGE_VIEWER(image_viewer));
  double w = gtk_image_viewer_get_canvas_width(GTK_IMAGE_VIEWER(image_viewer));
  gtk_image_viewer_img_coord_to_canv_coord(GTK_IMAGE_VIEWER(image_viewer),
                                           x0,  y0,
                                           /* output */
                                           &old_canvas_x,
                                           &old_canvas_y);
                                                              
  gtk_image_viewer_zoom_around_fixed_point(GTK_IMAGE_VIEWER(image_viewer),
					   zoom,
					   zoom,
					   old_canvas_x, old_canvas_y,
                                           w/2,h/2);
}

static void
create_goto_point_window()
{
  GtkWidget *vbox, *hbox;
  GtkWidget *table1;
  GtkWidget *button_goto, *button_cancel;
  gchar *fields[] = { "x:", "y:", "zoom:" };
  int num_fields = 3;
  int field_idx;

  if (w_goto_point_window)
    gtk_widget_destroy(w_goto_point_window);

  w_goto_point_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_border_width (GTK_CONTAINER (w_goto_point_window), 3);

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add (GTK_CONTAINER (w_goto_point_window), vbox);
  gtk_widget_show(vbox);
    
  table1 = gtk_table_new (2, 3, FALSE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (w_goto_point_window), "table1", table1,
			    (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox), table1, TRUE, TRUE, 0);
  
  button_goto = gtk_button_new_with_label("Goto");
  
  for (field_idx=0; field_idx<3; field_idx++) {
      GtkWidget *label = gtk_label_new(fields[field_idx]);
      GtkWidget *entry = gtk_entry_new();
      char field_name[16];
      
      gtk_table_attach (GTK_TABLE (table1), label, 0, 1, field_idx, field_idx+1,
                        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
      gtk_table_attach (GTK_TABLE (table1), entry, 1, 2, field_idx, field_idx+1,
                        (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                        (GtkAttachOptions) (0), 0, 0);
      
      /* Attach the entry to the button widget so that it may be retrieved
         when the goto button is pressed. */
      sprintf(field_name, "%d", field_idx);
      g_object_set_data(G_OBJECT(button_goto),
                        field_name,
                        entry);
  }
  
  /* goto and quit buttons */
  hbox = gtk_hbox_new(FALSE,5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show(hbox);
    
  gtk_box_pack_start (GTK_BOX (hbox), button_goto, TRUE, TRUE, 0);
  gtk_widget_show(button_goto);
  gtk_signal_connect (GTK_OBJECT (button_goto),
		      "clicked", GTK_SIGNAL_FUNC(giv_goto_point),
		      NULL);
    
  button_cancel = gtk_button_new_with_label("Cancel");
  gtk_box_pack_start (GTK_BOX (hbox), button_cancel, TRUE, TRUE, 0);
  gtk_widget_show(button_cancel);
  gtk_signal_connect (GTK_OBJECT (button_cancel),
		      "clicked", GTK_SIGNAL_FUNC(cb_toggle_goto_point_window),
		      NULL);

  gtk_widget_show_all(w_goto_point_window);

}

static void
create_button(GtkWidget *vbox,
	      const gchar *label,
	      gpointer func)
{
  /* Red only button */
  GtkWidget *button = gtk_button_new_with_label(label);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (func), NULL);
  gtk_widget_show(button);
}

static void
create_histogram_window()
{
  GtkWidget *vbox;

  if (w_histogram_window)
    gtk_widget_destroy(w_histogram_window);
    
  w_histogram_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_border_width (GTK_CONTAINER (w_histogram_window), 3);

  /* button box */
  vbox = gtk_vbox_new(0,0);
  gtk_container_add (GTK_CONTAINER (w_histogram_window), vbox);
  gtk_widget_show(vbox);

  /* Drawing area */
  histogram_drawing_area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(histogram_drawing_area), 256, histogram_height);
  gtk_box_pack_start (GTK_BOX (vbox), histogram_drawing_area, TRUE, TRUE, 0);

  /* Setup signals */
  gtk_signal_connect (GTK_OBJECT (histogram_drawing_area),    "expose_event",
		      (GtkSignalFunc) cb_histogram_expose_event,
		      NULL);
  gtk_signal_connect (GTK_OBJECT (histogram_drawing_area),    "configure_event",
		      (GtkSignalFunc) cb_configure_histogram_event,
		      NULL);
  gtk_signal_connect (GTK_OBJECT (histogram_drawing_area),    "button_press_event",
		      (GtkSignalFunc)		      cb_histogram_zoom,
		      NULL);
    
  gtk_widget_set_events(histogram_drawing_area,
			GDK_EXPOSURE_MASK
			| GDK_STRUCTURE_MASK
			| GDK_PROPERTY_CHANGE_MASK
			| GDK_BUTTON_PRESS_MASK
			);
    
  gtk_widget_show(histogram_drawing_area);
}

static void
create_control_window()
{
  GtkWidget *vbox;

  if (w_control_window)
    gtk_widget_destroy(w_control_window);
    
  w_control_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_border_width (GTK_CONTAINER (w_control_window), 3);

  /* button box */
  vbox = gtk_vbox_new (0, 0);
  gtk_container_add (GTK_CONTAINER (w_control_window), vbox);
  gtk_widget_show(vbox);

  create_button(vbox, "Reset", cb_reset_image);
  create_button(vbox, "Invert", cb_invert_image);
  create_button(vbox, "Normalize", cb_normalize_image);
  create_button(vbox, "Equalize", cb_equalize_image);
  create_button(vbox, "Histogram", cb_show_histogram);
  create_button(vbox, "Color", cb_color_image);
  create_button(vbox, "Red only", cb_red_only_image);
  create_button(vbox, "Green only", cb_green_only_image);
  create_button(vbox, "Blue only", cb_blue_only_image);
  create_button(vbox, "Load image", cb_load_image);
  create_button(vbox, "Marks", cb_toggle_marks_window);
  create_button(vbox, "Print", cb_toggle_print_window);
  create_button(vbox, "Goto Point", cb_toggle_goto_point_window);
}

/*======================================================================
//  Pixel algorithms callbacks.
//----------------------------------------------------------------------*/

static void
set_transfer_function(trans_func_t which_trans_func)
{
  int hist_idx, col_idx, pix_idx;
  guint8 *buf = gdk_pixbuf_get_pixels(img_display);
  gint width = gdk_pixbuf_get_width(img_display);
  gint height = gdk_pixbuf_get_height(img_display);
    
  switch(which_trans_func) {
  case TRANS_FUNC_RESET:
    for (col_idx = 0; col_idx<3; col_idx++) 
      for (hist_idx = 0; hist_idx < 256; hist_idx++)
	current_maps[col_idx][hist_idx] = hist_idx;
    break;
  case TRANS_FUNC_INVERT:
    for (col_idx = 0; col_idx<3; col_idx++) 
      for (hist_idx = 0; hist_idx < 256; hist_idx++)
	current_maps[col_idx][hist_idx] = 255-current_maps[col_idx][hist_idx];
    break;
  case TRANS_FUNC_LOW_CONTRAST:
    for (col_idx = 0; col_idx<3; col_idx++) 
      for (hist_idx = 0; hist_idx < 256; hist_idx++)
	current_maps[col_idx][hist_idx] = LOW_CONTRAST_LOW +
	  (LOW_CONTRAST_HIGH-LOW_CONTRAST_LOW)*hist_idx/256;
    break;
  case TRANS_FUNC_NORM:
    {
      guint8 min[3], max[3];
	    
      min[0] = min[1]=min[2] = 255;
      max[0] = max[1] = max[2] = 0;
      for (pix_idx=0; pix_idx<width*height; pix_idx++) {
	for (col_idx=0; col_idx<3;col_idx++) {
	  if (*buf < min[col_idx])
	    min[col_idx] = *buf;
	  else if (*buf > max[col_idx])
	    max[col_idx] = *buf;
	  buf++;
	}
      }
	    
      for (col_idx=0; col_idx<3; col_idx++) {
	for (hist_idx=0; hist_idx<256; hist_idx++) {
	  int map_value = 255 * (hist_idx-min[col_idx])
	    / (max[col_idx] - min[col_idx]);
	  if (map_value > 255)
	    map_value = 255;
	  else if (map_value < 0)
	    map_value = 0;
	  current_maps[col_idx][hist_idx] = map_value;
	}
      }
    }
    break;
  case TRANS_FUNC_EQ:
    {
      gint hist[3][256];
	    
      memset(hist, 0, sizeof(hist));

      for (pix_idx=0; pix_idx<width*height; pix_idx++) {
	for (col_idx=0; col_idx<3;col_idx++) {
	  hist[col_idx][*buf]++;
	  buf++;
	}
      }
	    
      for (col_idx=0; col_idx<3; col_idx++) {
	int accsum = 0;
	for (hist_idx=0; hist_idx<256; hist_idx++) {
	  current_maps[col_idx][hist_idx] = (accsum * 255) / (width * height);
	  accsum+= hist[col_idx][hist_idx];
	}
      }
    }
    break;
  case TRANS_FUNC_CURVE:
    break;
  }

  if (image_viewer)
  gtk_image_viewer_set_transfer_map(image_viewer,
				    current_maps[0],
				    current_maps[1],
				    current_maps[2] );
}

static void
cb_reset_image()
{
  if (!img_display)
    return;
  cb_color_image();
  set_transfer_function(TRANS_FUNC_RESET);
}

static void
cb_invert_image()
{
  if (!img_display)
    return;
  set_transfer_function(TRANS_FUNC_INVERT);
}

static void
cb_normalize_image()
{
  if (!img_display)
    return;
  set_transfer_function(TRANS_FUNC_NORM);
}

static void
cb_equalize_image()
{
  if (!img_display)
    return;
  set_transfer_function(TRANS_FUNC_EQ);
}

static void
cb_low_contrast_image()
{
  if (!img_display)
    return;
  set_transfer_function(TRANS_FUNC_LOW_CONTRAST);
}

static void
cb_show_histogram()
{
  if (!img_display)
    return;
  if (histogram_window_is_shown) {
    gtk_widget_destroy(w_histogram_window);
    w_histogram_window = NULL;
  }
  else {
    create_histogram_window();
    gtk_widget_show(w_histogram_window);
  }
  histogram_window_is_shown = !histogram_window_is_shown;
}

static void
cb_color_image()
{
  if (image_viewer == NULL)
    return;
  
  /* Free up the display image and set its pointer to the org image */
  if (img_org != img_display)
    {
      gdk_pixbuf_unref(img_display);
      img_display = img_org;
      gtk_image_viewer_set_image(image_viewer, img_display);
    }

}

static void
mono_only_image(int col_idx)
{
  gint w,h;
  int pix_idx;
  guint8 *p, *q;

  if (!img_org)
    return;
  w = gdk_pixbuf_get_width(img_org);
  h = gdk_pixbuf_get_height(img_org);
  
  if (img_org == img_display)
    {
      /* By copying I don't have to worry about strides, etc. It takes
	 a bit more time though. */
      img_display = gdk_pixbuf_copy(img_org);
    }
  
  p = gdk_pixbuf_get_pixels(img_org) + col_idx;
  q = gdk_pixbuf_get_pixels(img_display);
  for (pix_idx=0; pix_idx<w*h; pix_idx++)
    {
      *q++ = *p;
      *q++ = *p;
      *q++ = *p;
      
      p+= 3;
      
    }
  gtk_image_viewer_set_image(image_viewer, img_display);
}

static void
cb_red_only_image()
{
  mono_only_image(0);
}
     
static void
cb_green_only_image()
{
  mono_only_image(1);
}
     
static void
cb_blue_only_image()
{
  mono_only_image(2);
}
     
static void
cb_toggle_marks()
{
  do_show_marks = !do_show_marks;
  gtk_image_viewer_redraw(GTK_WIDGET(image_viewer));
}

/*======================================================================
//  Histogram functions.
//----------------------------------------------------------------------*/
static int
cb_histogram_zoom(GtkWidget *widget,
		  GdkEventButton *event)
{
  gint button = event->button;
  if (button == 1)
  histogram_scale *=1.4;
  else if (button == 2)
  histogram_scale = 1.0;
  else if (button == 3)
  histogram_scale /= 1.4;
  draw_histogram();
  return 1;
}

static void
calc_histogram()
{
  int pix_idx, col_idx;
  guint8 *buf = gdk_pixbuf_get_pixels(img_org);
  gint width = gdk_pixbuf_get_width(img_org);
  gint height = gdk_pixbuf_get_height(img_org);
    
  memset(hist, 0, sizeof(hist));
    
  for (pix_idx=0; pix_idx<width*height; pix_idx++) {
    for (col_idx=0; col_idx<3;col_idx++) {
      hist[col_idx][*buf]++;
      buf++;
    }
  }
}

static void
draw_histogram()
{
  int hist_idx, col_idx;
  gdouble hist_normalize;
  gint max_hist[3];
  GdkGC *gc = gdk_gc_new(histogram_drawing_area->window);
  GdkColor color = histogram_color;

  gc_set_attribs(gc,
		 &color,
		 1,
		 GDK_LINE_SOLID);
    
  for (col_idx=0; col_idx <3; col_idx++) {
    max_hist[col_idx] = 0;
    for (hist_idx=0; hist_idx<256; hist_idx++) {
      if (hist[col_idx][hist_idx]> max_hist[col_idx])
	max_hist[col_idx]= hist[col_idx][hist_idx];
    }
  }

  /* Draw the histogram, currently only for red*/
  if (max_hist[0] > 0)
    hist_normalize = 1.0*(histogram_height-10)/max_hist[0];
  else
    hist_normalize = 1;

  gdk_draw_rectangle(histogram_drawing_pixmap,
		     histogram_drawing_area->style->bg_gc[GTK_WIDGET_STATE (histogram_drawing_area)],
		     TRUE,
		     0,0,256,histogram_height);
  for (hist_idx=0; hist_idx<256; hist_idx++) {
    int y0 = histogram_height-5;
    int y1 = histogram_height-5-histogram_scale*hist_normalize * hist[0][hist_idx];
    gdk_draw_line(histogram_drawing_pixmap,
		  gc,
		  hist_idx, y0,
		  hist_idx, y1); 
  }
  g_free(gc);

  /* Make it updated */
  redraw_histogram(histogram_drawing_area);
}

/*======================================================================
//  Line drawing functionts.
//----------------------------------------------------------------------*/
static void
gc_set_attribs(GdkGC *gc,
	       GdkColor *color,
	       gint line_width,
	       gint line_style)
{
    
  /* I don't know if this is the proper way of doing it... */
  gdk_colormap_alloc_color (gdk_colormap_get_system(),
			    color,
			    FALSE,
			    TRUE);
  gdk_gc_set_foreground(gc, color);
  gdk_gc_set_line_attributes(gc, line_width, line_style, GDK_CAP_ROUND,
			     GDK_JOIN_ROUND);
}

static void
draw_one_mark(GdkWindow *drawable,
	      GdkGC *gc,
	      int x, int y,
	      int mark_type,
	      double size_x, double size_y)
{
  if ((x+size_x < 0 || x-size_x > canvas_width)
      && (y+size_y < 0 || y-size_y > canvas_height))
    return;
    
  if (mark_type == MARK_TYPE_CIRCLE || mark_type == MARK_TYPE_FCIRCLE)
    {
      gboolean do_fill = mark_type == MARK_TYPE_FCIRCLE;

      gdk_draw_arc(drawable,
		   gc,
		   do_fill,
		   x-size_x/2,y-size_y/2,
		   size_x, size_y,
		   0, 360*64);
    }
  else if (mark_type == MARK_TYPE_SQUARE || mark_type == MARK_TYPE_FSQUARE)
    {
      gboolean do_fill = mark_type == MARK_TYPE_FSQUARE;
      gdk_draw_rectangle(drawable,
			 gc,
			 do_fill,
			 x-size_x/2, y-size_y/2,
			 size_x, size_y);
    }
  else if (mark_type == MARK_TYPE_PIXEL) {
    gdk_draw_point(drawable,
		   gc,
		   x, y);
  }
  else
    g_message("Unknown mark type!");
}

static inline gboolean
line_hor_line_intersect(double x0, double y0, double x1, double y1,
			double line_x0, double line_x1, double line_y,
			/* output */
			double *x_cross, double *y_cross)
{
  if (y1 == y0) {
    *y_cross = x0;
    *x_cross = 0; /* Any x is a crossing */
    if (y1 == line_y)
      return TRUE;
    return FALSE;
  }

  *y_cross = line_y; /* Obviously! */
  *x_cross = x0 + (x1 - x0)*(line_y - y0)/(y1-y0);

  if (y1<y0) {
    double tmp = y0;
    y0=y1;
    y1=tmp;
  }

  return (*x_cross >= line_x0 && *x_cross <= line_x1 && *y_cross >= y0 && *y_cross <= y1);
}

static inline gboolean
line_ver_line_intersect(double x0, double y0, double x1, double y1,
			double line_y0, double line_y1, double line_x,
			/* output */
			double *x_cross, double *y_cross)
{
  if (x1 == x0) {
    *x_cross = x0;
    *y_cross = 0; /* Any y is a crossing */
    if (x1 == line_x)
      return TRUE;
    return FALSE;
  }


  *x_cross = line_x; /* Obviously! */
  *y_cross = y0 + (y1 - y0)*(line_x - x0)/(x1-x0);

  if (x1<x0) {
    double tmp = x0;
    x0=x1;
    x1=tmp;
  }
  return (*y_cross >= line_y0 && *y_cross <= line_y1 && *x_cross >= x0 && *x_cross <= x1);
}

static inline gboolean
clip_line_to_rectangle(double x0, double y0, double x1, double y1,
		       int rect_x0, int rect_y0, int rect_x1, int rect_y1,
		       /* output */
		       double *cx0, double *cy0, double *cx1, double *cy1)
{
  gboolean z0_inside, z1_inside;
  int num_crosses = 0;
  double cross_x[4], cross_y[4];
    
  /* Trivial tests if the point is outside the window on the same side*/
  if (x0 < rect_x0 && x1 < rect_x0)     return FALSE;
  if (y0 < rect_y0 && y1 < rect_y0)     return FALSE;
  if (x0 > rect_x1 && x1 > rect_x1)     return FALSE;
  if (y0 > rect_y1 && y1 > rect_y1)     return FALSE;

  /* Test if p1 is inside the window */
  z0_inside = (   x0 >= rect_x0 && x0 <= rect_x1
		  && y0 >= rect_y0 && y0 <= rect_y1);

  /* Test if p2 is inside the window */
  z1_inside = (   x1 >= rect_x0 && x1 <= rect_x1
		  && y1 >= rect_y0 && y1 <= rect_y1);

#ifdef DEBUG_CLIP
  printf("z0_inside z1_inside = %d %d  z0=(%d %d)  z1=(%d %d) rect=(%d %d %d %d)\n", z0_inside, z1_inside, (int)x0, (int)y0, (int)x1, (int)y1, (int)rect_x0, (int)rect_y0, (int)rect_x1, (int)rect_y1);
#endif
    
  /* Check if both are inside */
  if (z0_inside && z1_inside) {
    *cx0 = x0; *cy0 = y0;
    *cx1 = x1; *cy1 = y1;
    return TRUE;
  }

  /* Check for line intersection with the four edges */
  if (line_hor_line_intersect(x0, y0, x1, y1,
			      rect_x0, rect_x1, rect_y0,
			      &cross_x[num_crosses], &cross_y[num_crosses]))
    ++num_crosses;

  if (line_hor_line_intersect(x0, y0, x1, y1,
			      rect_x0, rect_x1, rect_y1,
			      &cross_x[num_crosses], &cross_y[num_crosses]))
    ++num_crosses;

  if (line_ver_line_intersect(x0, y0, x1, y1,
			      rect_y0, rect_y1, rect_x0, 
			      &cross_x[num_crosses], &cross_y[num_crosses]))
    ++num_crosses;

  if (line_ver_line_intersect(x0, y0, x1, y1,
			      rect_y0, rect_y1, rect_x1, 
			      &cross_x[num_crosses], &cross_y[num_crosses]))
    ++num_crosses;

#ifdef DEBUG_CLIP
  {
    int i;
    printf("num_crosses = %d\n", num_crosses);
    for (i=0; i<num_crosses; i++) {
      printf("   crossing[%d] = (%.2f %.2f)\n", i, cross_x[i], cross_y[i]);
    }
  }
#endif
    
  if (num_crosses == 0)
    return FALSE;
  else if (num_crosses == 1) {
    if (z0_inside) {
      *cx1 = cross_x[0];
      *cy1 = cross_y[0];
      *cx0 = x0;
      *cy0 = y0;
    } else {
      *cx0 = cross_x[0];
      *cy0 = cross_y[0];
      *cx1 = x1;
      *cy1 = y1;
    }
    return TRUE;
  }
  else {
    *cx0 = cross_x[0];
    *cy0 = cross_y[0];
    *cx1 = cross_x[1];
    *cy1 = cross_y[1];
    return TRUE;
  }
}

static void
draw_marks(GtkImageViewer *image_viewer)
{
  GdkWindow *drawing_area = GTK_WIDGET(image_viewer)->window;
  int set_idx;
  double scale_x, scale_y;
  GdkGC *gc = gdk_gc_new(drawing_area);

  if (gtk_image_viewer_get_is_panning(image_viewer))
    return;
    
  canvas_width = gtk_image_viewer_get_canvas_width(image_viewer);
  canvas_height = gtk_image_viewer_get_canvas_height(image_viewer);
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
    gdouble mark_size_x, mark_size_y;
    gint mark_type;
    GArray *segments;

    segments = g_array_new(FALSE, FALSE, sizeof(GdkSegment));
	
    gc_set_attribs(gc,
		   &mark_set->color,
		   mark_set->line_width,
		   mark_set->line_style);
    mark_size_x = mark_size_y = mark_set->mark_size;
    mark_type = mark_set->mark_type;
    if (mark_set->do_scale_marks) {
      mark_size_x *= scale_x;
      mark_size_y *= scale_y;
    }

    is_first_point = TRUE;
    for (p_idx=0; p_idx<mark_set->points->len; p_idx++) {
      point_t p = g_array_index(mark_set->points, point_t, p_idx);
      double x = p.data.point.x;
      double y = p.data.point.y;
      double cx, cy;
      int op = p.op;

      gtk_image_viewer_img_coord_to_canv_coord(image_viewer,
					       x,y,
					       &cx,&cy);
#ifdef DEBUG_CLIP
      printf("scale = %.0f  x y = %g %g   cx cy = %.2f %.2f\n", current_scale, x,y, cx, cy);
#endif

      if (mark_set->do_draw_lines && !is_first_point && op == OP_DRAW) {
	GdkSegment seg;
	double x1,y1,x2,y2;

	if (clip_line_to_rectangle(old_cx, old_cy, cx, cy,
				   0, 0, canvas_width, canvas_height,
				   &x1,&y1,&x2,&y2)) {
		
	  /* Must clip the area shown */
	  seg.x1 = (gint16)x1;
	  seg.y1 = (gint16)y1;
	  seg.x2 = (gint16)x2;
	  seg.y2 = (gint16)y2;
	  segments = g_array_append_val(segments, seg);
	}

#if 0
	draw_line(drawing_area,
		  gc,
		  old_cx, old_cy,
		  cx, cy);

#endif
      }
      if (mark_set->do_draw_marks) {
	/* Trivial clipping for marks */
	if (   cx+mark_size_x > 0
	       && cy+mark_size_y > 0
	       && cx-mark_size_x < canvas_width
	       && cy-mark_size_y < canvas_height)
	  draw_one_mark(drawing_area,
			gc,
			cx, cy,
			mark_type,
			mark_size_x,
			mark_size_y
			);
      }
      is_first_point = FALSE;
      old_cx = cx;
      old_cy = cy;
    }
    gdk_draw_segments(drawing_area, gc, &(g_array_index(segments,GdkSegment,0)), segments->len);
    g_array_free(segments, TRUE);

  }
  g_free(gc);
}

/*======================================================================
//  The draw_marks_in_postscript is getting its clipping and scaling
//  info straight from the widget. Probably this info should be
//  translated through some proxy...
//----------------------------------------------------------------------*/
static void
draw_marks_in_postscript(GtkImageViewer *widget,
			 FILE *PS)
{
  int set_idx;
  GtkImageViewer *image_viewer = GTK_IMAGE_VIEWER(widget);
  GdkColor current_color;
  double current_line_width = -1;
  double current_mark_size_x = -1;
  double current_mark_size_y = -1;
  double scale_x, scale_y;

  gtk_image_viewer_get_scale(image_viewer, &scale_x, &scale_y);
    
  /* Create some postscript macros */
  fprintf(PS,
	  "/sc { setrgbcolor } bind def\n"
	  "/lw { setlinewidth } bind def\n"
	  "/L { lineto } bind def\n"
	  "/S { stroke } bind def\n"
	  "/M /moveto load def\n"
	  "/ms { /marksize_y exch def marksize_x exch def } def\n"
	  "/mC { /y exch def /x exch def\n"
	  "       x marksize_x 2 div add y moveto\n"
	  "       x y marksize_x 2 div 0 360 arc stroke\n"
	  "    } bind def\n"
	  "/mFC { /y exch def /x exch def\n"
	  "       x marksize_x 2 div add y moveto\n"
	  "       x y marksize_x 2 div 0 360 arc fill stroke\n"
	  "    } bind def\n"
	  "/mS { /y exch def /x exch def\n"
	  "      x marksize_x 2 div sub  y marksize_y 2 div sub moveto\n"
	  "      marksize_x 0 rlineto\n"
	  "      0 marksize_y rlineto\n"
	  "      marksize_x neg 0 rlineto\n"
	  "      0 marksize_y neg rlineto\n"
	  "      stroke } bind def\n"
	  "/mFS { /y exch def /x exch def\n"
	  "      x marksize_x 2 div sub  y marksize_y 2 div sub moveto\n"
	  "      marksize_x 0 rlineto\n"
	  "      0 marksize_y rlineto\n"
	  "      marksize_x neg 0 rlineto\n"
	  "      0 marksize_y neg rlineto\n"
	  "      fill stroke } bind def\n"
	  "1 setlinejoin %% round endings and caps\n"
	  "1 setlinecap\n"
	  "newpath\n"
	  );
    
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

    /* Choose the color and line widths, etc*/
    if (set_idx == 0
	|| !color_eq(&current_color, &mark_set->color))
      {
	double red = 1.0 * mark_set->color.red / 0xffff;
	double green = 1.0 * mark_set->color.green / 0xffff;
	double blue = 1.0 * mark_set->color.blue / 0xffff;
	    
	fprintf(PS, "%.4g %.4g %.4g sc\n", red, green, blue);

	current_color = mark_set->color;
      }
	    
    if (current_line_width != mark_set->line_width)
      {
	fprintf(PS, "%g lw\n", 1.0*mark_set->line_width);
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
	fprintf(PS, "%g %g ms\n", mark_size_x, mark_size_y);
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
	  if (in_path)
	    fprintf(PS, "S\n");
	  in_path = FALSE;
	  fprintf(PS, "%.4g %.4g M\n", cx,cy);
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
		  fprintf(PS, "S\n");
		fprintf(PS, "%.4g %.4g M\n", x1,y1);
#endif
	      }
	    }
	    if (op == OP_DRAW) {
	      fprintf(PS, "%.4g %.4g M\n", x1,y1);
	      fprintf(PS, "%.4g %.4g L S\n", x2,y2);
	      in_path = TRUE;
	    }
	  }
	}
      }

      if (mark_set->do_draw_marks) {
	/* Trivial clipping for marks */
	if (   cx+mark_size_x > 0
	       && cy+mark_size_y > 0
	       && cx-mark_size_x < canvas_width
	       && cy-mark_size_y < canvas_height) {
	  if (mark_type == MARK_TYPE_CIRCLE)
	    fprintf(PS, "%.4g %.4g mC\n", cx, cy);
	  if (mark_type == MARK_TYPE_FCIRCLE)
	    fprintf(PS, "%.4g %.4g mFC\n", cx, cy);
	  else if (mark_type == MARK_TYPE_SQUARE)
	    fprintf(PS, "%.4g %.4g mS\n", cx, cy);
	  else if (mark_type == MARK_TYPE_FSQUARE)
	    fprintf(PS, "%.4g %.4g mFS\n", cx, cy);
	  else
	    g_message("Unknown mark type!\n");
	}
      }
      is_first_point = FALSE;
      old_cx = cx;
      old_cy = cy;
    }
    if (in_path)
      fprintf(PS, "S\n");
  }
}

static gboolean color_eq(GdkColor *color1, GdkColor *color2)
{
  return (color1->red == color2->red
	  && color1->green == color2->green
	  && color1->blue == color2->blue);
}

/* Define own version of floor as I can't use built in version with
   cygwin unless I want to use cygwin.dll! */
static double giv_floor(double x)
{
  if (x < 0)
    return -((int)(-x-1e-15+1));
  else
    return (int)x;
}

static void
set_last_directory_from_filename(const gchar *filename)
{
    gchar *dir_name;
    
    if (giv_last_directory)
        free(giv_last_directory);
    dir_name = g_path_get_dirname(filename);
    giv_last_directory = g_strdup_printf("%s/", dir_name);
    free(dir_name);
}

/*======================================================================
//  Make the window exactly fit the marks.
//----------------------------------------------------------------------*/
static void
fit_marks_in_window()
{
  // 500 is the default window width and height. It should be changed
  // to a parameter...
  double x_scale = 500 / (global_mark_max_x - global_mark_min_x)*0.7;
  double y_scale = 500 / (global_mark_max_y - global_mark_min_y)*0.7;
  double scale = x_scale;

  if (y_scale < scale)
    scale = y_scale;

  gtk_image_viewer_zoom_around_fixed_point(image_viewer,
                                           x_scale,
                                           y_scale,
                                           global_mark_min_x,
                                           global_mark_min_y,
                                           0,0);
}
