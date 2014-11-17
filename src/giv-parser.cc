//======================================================================
//  GivParser.cc - A parser for the giv file format.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Nov  4 23:13:25 2007
//----------------------------------------------------------------------

#include <math.h>
#include "giv-parser.h"
#include "giv-data.h"
#include "GivStringArray.h"
#include "plis/plis.h"

using namespace plis;

enum
{
  STRING_NOP,
  STRING_DRAW,
  STRING_ELLIPSE,
  STRING_COMMENT,
  STRING_MOVE,
  STRING_QUIVER,
  STRING_TEXT,
  STRING_FONT,
  STRING_DASH,
  STRING_ARROW,
  STRING_CHANGE_COLOR,
  STRING_CHANGE_OUTLINE_COLOR,
  STRING_CHANGE_QUIVER_COLOR,
  STRING_CHANGE_SHADOW_COLOR,
  STRING_CHANGE_SHADOW_OFFSET,
  STRING_CHANGE_QUIVER_SCALE,
  STRING_CHANGE_QUIVER_HEAD,
  STRING_CHANGE_LINE_WIDTH,
  STRING_CHANGE_MARKS,
  STRING_CHANGE_NO_LINE,
  STRING_CHANGE_SCALE_MARKS,
  STRING_CHANGE_SCALE_FONTS,
  STRING_CHANGE_PANGO_MARKUP,
  STRING_CHANGE_MARK_SIZE,
  STRING_CHANGE_TEXT_SIZE,
  STRING_CHANGE_LINE,
  STRING_CHANGE_NO_MARK,
  STRING_CHANGE_POLYGON,
  STRING_BALLOON,
  STRING_IMAGE_REFERENCE,
  STRING_MARKS_REFERENCE,
  STRING_LOW_CONTRAST,
  STRING_VFLIP,
  STRING_VLOCK,
  STRING_NO_VFLIP,
  STRING_HFLIP,
  STRING_NO_HFLIP,
  STRING_PATH_NAME,
  STRING_SET_NAME,
  STRING_SET_XUNIT_TEXT,
  STRING_SET_YUNIT_TEXT,
  STRING_SET_LARGE_PIXELS,
  STRING_SET_TITLE,
  STRING_STYLE,
  STRING_DEF_STYLE,
  STRING_HIDE,
  STRING_IGNORE,
  STRING_TEXT_STYLE
};

#define COLOR_NONE 0xfffe

#define NCASE(s) if (!g_ascii_strcasecmp(s, S_))
#define CASE(s) if (!strcmp(s, S_))

static int color_parse(const char *giv_color_name,
                       // output
                       GivColor *color);

GivColor set_colors[] =
    { {0xffff, 0, 0, 0xffff},
      {0, 0xffff, 0, 0xffff},  
      {0, 0, 0xffff, 0xffff},
      {0xffff, 0xffff, 0, 0xffff},
      {0, 0xffff, 0xffff, 0xffff},
      {0xffff, 0, 0xffff, 0xffff} };
gint nmarks_colors = 6;

GivParser *giv_parser_new()
{
    GivParser *giv_parser = g_new0(GivParser, 1);
    giv_parser->cb_file_reference = NULL;
    giv_parser->quiver_scale = 1.0;
    giv_parser->style_hash = g_hash_table_new_full(g_str_hash,
                                                   g_str_equal,
                                                   (GDestroyNotify)g_free,
                                                   (GDestroyNotify)giv_string_array_free);
    giv_parser->giv_datasets = g_ptr_array_new();
    giv_parser_clear(giv_parser);
    return giv_parser;
}

void giv_parser_free(GivParser *gp)
{
    g_hash_table_destroy(gp->style_hash);
    giv_parser_clear(gp);
    g_ptr_array_free(gp->giv_datasets, TRUE);
    g_free(gp);
}

int
giv_parser_parse_file(GivParser *gp,
                      const char *filename)
{
    int ret = 0;

    giv_dataset_t *marks = NULL;
    gboolean is_new_set;
    int num_sets = gp->giv_datasets->len;
    int linenum = 0;
    
    FILE *GIV;

    GIV = fopen(filename, "rb");
    if (!GIV)
	return -1;

    is_new_set = TRUE;
    gboolean empty_line = false;
    while(!empty_line) {
	char S_[256];
	int len;
	
	linenum++;
	fgets(S_, sizeof(S_), GIV);
	len = strlen(S_);

        // Skip damaged sections with all NULLS
        if (len==0)
            empty_line = true;

	// Get rid of CR and LF at end of line
        int org_len = len;
	while (len>0 && (S_[len-1] == '\r' || S_[len-1] == '\n')) {
	    S_[len-1] = 0;
	    len--;
	}

        // Get out if we didn't get a \r or \n at the end of the line!
        if (org_len == len)
            break;
	
	if (is_new_set || marks==NULL) {
	    marks = new_giv_dataset(num_sets);
	    marks->color = set_colors[num_sets % nmarks_colors];
	    marks->file_name = g_strdup(filename);
            g_ptr_array_add(gp->giv_datasets, marks);
	    
	    is_new_set = FALSE;
	    num_sets++;
	}
	
	if (len == 0) {
	    if (marks && ((GArray*)marks->points)->len > 0)
		is_new_set++;
	    continue;
	}

        giv_parser_giv_marks_data_add_line(gp, marks, S_, filename, linenum);

        if (feof(GIV))
            break;
    }
    fclose(GIV);

    /* Get rid of empty data sets */
    if (marks && marks->points->len == 0) {
        g_ptr_array_remove_index(gp->giv_datasets, gp->giv_datasets->len-1);
	free_giv_data_set(marks);
	marks = NULL;
    }
    
    return ret;
}

// Parse a string - should return the index of the first dataset in the
// array. This can be used for erasing afterwards.
int
giv_parser_parse_string(GivParser *gp,
                        const char *giv_string)
{
    int ret = 0;

    giv_dataset_t *marks = NULL;
    gboolean is_new_set = TRUE;
    int num_sets = gp->giv_datasets->len;

    const char *p = giv_string;
    int giv_string_len = strlen(giv_string);
    int line_num = 0;
    while(p < giv_string + giv_string_len) {
        line_num++;
        const char *line_start = p;
        while(*p!= '\n' && p < giv_string + giv_string_len) {
            p++;
        }
        char *S_ = g_strndup(line_start, p-line_start);
	int len = p-line_start;
        p++;
	
	if (is_new_set || marks==NULL) {
	    marks = new_giv_dataset(num_sets);
	    marks->color = set_colors[num_sets % nmarks_colors];
	    marks->file_name = g_strdup("string");
            g_ptr_array_add(gp->giv_datasets, marks);
	    
	    is_new_set = FALSE;
	    num_sets++;
	}
	
	if (len == 0) {
	    if (marks && ((GArray*)marks->points)->len > 0)
		is_new_set++;
	}
        else
            giv_parser_giv_marks_data_add_line(gp, marks, S_, "string", line_num);
        g_free(S_);
    }

    /* Get rid of empty data sets */
    if (marks && marks->points->len == 0) {
        g_ptr_array_remove_index(gp->giv_datasets, gp->giv_datasets->len-1);
	free_giv_data_set(marks);
	marks = NULL;
    }
    
    return ret;
}


/*======================================================================
//
// General string handling in C. These functions should be replaced
// by something in glib.
//
//----------------------------------------------------------------------*/
static int
string_count_words (const char *string)
{
  int nwords = 0;
  const char *p = string;
  int in_word = 0;
  while (*p) {
      if (!in_word) {
	  if ((*p != ' ') && (*p != '\n') && (*p != '\t'))
              in_word = 1;
      }
      else if (in_word) {
	  if ((*p == ' ') || (*p == '\n') || (*p == '\t')) {
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

char *
string_strdup_word (const char *string, int idx)
{
  const char *p = string;
  int in_word = 0;
  int word_count = -1;
  int nchr = 1;
  char *word = NULL;
  const char *word_start = string;

  /* printf("p = 0x%x\n", p); */
  while (*p)
    {
      /* printf("%c\n", *p); fflush(stdout); */
      if (!in_word)
	{
	  if (*p != ' ' && *p != '\n' && *p != '\t')
	    {
	      in_word = 1;
	      word_count++;
	      nchr = 1;
	      word_start = p;
	    }
	}
      else if (in_word)
	{
	  if (*p == ' ' || *p == '\n' || *p == '\t')
	    {
	      if (idx == word_count)
		break;
	      in_word = 0;
	    }
	}
      nchr++;
      p++;
    }
  if (in_word)
    {
      word = g_new (char, nchr);
      strncpy (word, word_start, nchr - 1);
      word[nchr - 1] = 0;
    }
  return word;
}

static char *
string_strdup_rest (const char *string, int idx, bool parse_escape)
{
  const char *p = string;
  int in_word = 0;
  int word_count = -1;
  int nchr = 1;
  char *word = NULL;
  const char *word_start = string;

  /* printf("p = 0x%x\n", p); */
  while (*p)
    {
      char ch = *p;

      /* printf("%c\n", *p); fflush(stdout); */
      if (!in_word)
	{
	  if (ch != ' ' && ch != '\n' && ch != '\t')
	    {
	      in_word = 1;
	      word_count++;
	      nchr = 1;
	      word_start = p;
	    }
	}
      else if (in_word)
	{
	  if (ch == ' ' || ch == '\n' || ch == '\t')
	    {
	      if (idx == word_count)
		break;
	      in_word = 0;
	    }
	}
      nchr++;
      p++;
    }

  if (in_word)
    {
      nchr = strlen (word_start);
      word = g_new (char, nchr + 1);
      strncpy (word, word_start, nchr);
      word[nchr] = 0;
      if (parse_escape)
        {
          gchar *word_copy = g_strdup(word);
          char *p = word_copy;
          char *pd = word;
          while(*p)
            {
              char ch = *p;
              if (ch == '\\')
                {
                  p++;
                  if (!*p)
                    break;
                  if (*p=='n')
                    ch = '\n';
                  else if (*p == '\\')
                    ch = '\\';
                  else
                    {
                      // ignore the escape character
                      ch = '\\';
                      p--;
                    }
                }
              *pd++ = ch;
              p++;
            }
          *pd=0;
          g_free(word_copy);
        }
    }
  return word;
}

int
string_to_atoi (const char *string, int idx)
{
  char *word = string_strdup_word (string, idx);
  int value = atoi (word);
  g_free (word);

  return value;
}

gdouble
string_to_atof (const char *string, int idx)
{
  char *word = string_strdup_word (string, idx);
  gdouble value = atof (word);
  g_free (word);

  return value;
}


/*======================================================================
//  Classify a string.
//----------------------------------------------------------------------
*/
static gint
parse_string (const char *string, const char *fn, gint linenum)
{
  gint type = -1;
  gchar first_char = string[0];
  gchar *first_word;

  /* Shortcut for speeding up drawing */
  if (first_char >= '0' && first_char <= '9')
    return STRING_DRAW;

  first_word = string_strdup_word (string, 0);

  if (first_char == '#')
    type = STRING_COMMENT;
  else if (first_char == '$')
    {
      char *S_ = first_word;
      NCASE ("$lw")
      {
	type = STRING_CHANGE_LINE_WIDTH;
      }
      NCASE("$balloon")
      {
          type = STRING_BALLOON;
      }
      NCASE ("$color")
      {
	type = STRING_CHANGE_COLOR;
      }
      NCASE ("$outline_color")
      {
	type = STRING_CHANGE_OUTLINE_COLOR;
      }
      NCASE ("$quiver_color")
      {
	type = STRING_CHANGE_QUIVER_COLOR;
      }
      NCASE ("$shadow_color")
      {
	type = STRING_CHANGE_SHADOW_COLOR;
      }
      NCASE ("$shadow_offset")
      {
	type = STRING_CHANGE_SHADOW_OFFSET;
      }
      NCASE ("$quiver_scale")
      {
	type = STRING_CHANGE_QUIVER_SCALE;
      }
      NCASE ("$marks")
      {
	type = STRING_CHANGE_MARKS;
      }
      NCASE ("$noline")
      {
	type = STRING_CHANGE_NO_LINE;
      }
      NCASE ("$scale_marks")
      {
	type = STRING_CHANGE_SCALE_MARKS;
      }
      NCASE ("$scale_font")
      {
          type = STRING_CHANGE_SCALE_FONTS;
      }
      NCASE ("$pango_markup")
      {
	type = STRING_CHANGE_PANGO_MARKUP;
      }
      NCASE ("$path")
      {
	type = STRING_PATH_NAME;
      }
      NCASE ("$mark_size")
      {
	type = STRING_CHANGE_MARK_SIZE;
      }
      NCASE ("$text_size")
      {
	type = STRING_CHANGE_TEXT_SIZE;
      }
      NCASE ("$font")
      {
          type = STRING_FONT;
      }
      NCASE ("$text_style")
      {
          type = STRING_TEXT_STYLE;
      }
      NCASE ("$nomark")
      {
	type = STRING_CHANGE_NO_MARK;
      }
      NCASE ("$line")
      {
	type = STRING_CHANGE_LINE;
      }
      NCASE ("$image")
      {
	type = STRING_IMAGE_REFERENCE;
      }
      NCASE ("$polygon")
      {
	type = STRING_CHANGE_POLYGON;
      }
      NCASE ("$marks_file")
      {
	type = STRING_MARKS_REFERENCE;
      }
      NCASE ("$low_contrast")
      {
	type = STRING_LOW_CONTRAST;
      }
      NCASE("$vflip")
        {
          type = STRING_VFLIP;
        }
      NCASE("$vlock")
        {
          type = STRING_VLOCK;
        }
      NCASE("$novflip")
        {
          type = STRING_NO_VFLIP;
        }
      NCASE("$hflip")
        {
          type = STRING_HFLIP;
        }
      NCASE("$nohflip")
        {
          type = STRING_NO_HFLIP;
        }
      NCASE("$linedash")
        {
          type = STRING_DASH;
        }
      NCASE("$arrow") {
          type = STRING_ARROW;
      }
      NCASE("$def_style")
	{
	  type = STRING_DEF_STYLE;
	}
      NCASE("$style")
	{
	  type = STRING_STYLE;
	}
      NCASE ("$title")
      {
	type = STRING_SET_TITLE;
      }
      NCASE ("$name")
      {
	type = STRING_SET_NAME;
      }
      NCASE ("$hide")
      {
	type = STRING_HIDE;
      }
#if 0
      if (type == -1)
	{
	  fprintf (stderr, "Unknown parameter %s in file %s line %d!\n", S_,
		   fn, linenum);
	}
#endif
    }
  else if (((first_char >= 'a' && first_char <= 'z')
	    || (first_char >= 'A' && first_char <= 'Z'))
	   && first_word[strlen (first_word) - 1] == ':')
    {
      char *S_ = first_word;

      while(1) /* So that we may break */
	{
	  NCASE("Title:") { type = STRING_SET_TITLE; break;}
	  NCASE("TitleText:") { type = STRING_SET_TITLE; break;}
	  NCASE("XUnitText:") { type = STRING_SET_XUNIT_TEXT; break; }
	  NCASE("YUnitText:") { type = STRING_SET_YUNIT_TEXT; break; }
	  NCASE("LargePixels:") { type = STRING_SET_LARGE_PIXELS; break; }

	  printf("Unsupported keyword=%s\n", S_);
	  // TBD - Recognize more xgraph keywords...
	  type = STRING_NOP;

	  break;
	}

    }
  else if (first_char == 'M' || first_char == 'm')
    {
      type = STRING_MOVE;
    }
  else if (first_char == 'E' || first_char == 'e')
    {
      type = STRING_ELLIPSE;
    }
  else if (first_char == 'Q' || first_char == 'q')
      type = STRING_QUIVER;
  else if (first_char == 'T' || first_char == 't')
    {
      type = STRING_TEXT;
    }
  else if (first_char == '"')
    {
      type = STRING_SET_NAME;
    }
  else
    {
      type = STRING_DRAW;
    }

  g_free (first_word);

  return type;
}

gint
giv_parse_mark_type (const char *S_, const gchar * fn, gint linenum)
{
  NCASE ("circle") return MARK_TYPE_CIRCLE;
  NCASE ("fcircle") return MARK_TYPE_FCIRCLE;
  NCASE ("square") return MARK_TYPE_SQUARE;
  NCASE ("fsquare") return MARK_TYPE_FSQUARE;
  NCASE ("pixel") return MARK_TYPE_PIXEL;
  fprintf (stderr, "Unknown mark %s in file %s line %d\n", S_, fn, linenum);
  return MARK_TYPE_CIRCLE;
}


int
giv_parser_giv_marks_data_add_line(GivParser *gp,
                                   giv_dataset_t *marks,
                                   const char *line,
                                   const char *filename,
                                   int linenum)
{
    int type;
    const char *S_ = line; 
    char dummy[256];
    point_t p;
    // Only take marks size into account if we are scaling the marks!
    double ms2 = 0.5*marks->mark_size * marks->do_scale_marks; 

    /* Parse the line */
    type = parse_string(line, filename, linenum);
    switch (type) {
    case STRING_COMMENT:
        break;
    case STRING_DRAW:
    case STRING_MOVE:
    case STRING_ELLIPSE:
    case STRING_QUIVER:
        if (type == STRING_DRAW) {
            if (sscanf(S_, "%lf %lf", &p.x, &p.y)==2) {
                if (marks->points->len == 0)
                    p.op = OP_MOVE;
                else
                    p.op = OP_DRAW;
            }
        }
        else if (type == STRING_ELLIPSE) {
            double x,y,xsize, ysize, angle;
            sscanf(S_, "%s %lf %lf %lf %lf %lf", dummy, &x, &y, &xsize,&ysize,&angle);
            p.op = OP_ELLIPSE;
            p.x = x;
            p.y = y;
            g_array_append_val(marks->points, p);
            p.op = OP_CONT;
            p.x = xsize;
            p.y = ysize;
            g_array_append_val(marks->points, p);
            p.op = OP_CONT;
            p.x = angle;
            g_array_append_val(marks->points, p);

            // Assign p to x and y for min and max calculations.
            p.x = x;
            p.y = y;
        }
        else {
            if (sscanf(S_, "%s %lf %lf", dummy, &p.x, &p.y)==3) {
                if (type == STRING_QUIVER) {
                    p.op = OP_QUIVER;
                    marks->has_quiver = TRUE;
                }
                else
                    p.op = OP_MOVE;
            }
        }

        /* Find marks bounding box */
        if (p.x < gp->global_mark_min_x)
            gp->global_mark_min_x = p.x - ms2;
        if (p.x > gp->global_mark_max_x)
            gp->global_mark_max_x = p.x + ms2;
        if (p.y < gp->global_mark_min_y)
            gp->global_mark_min_y = p.y - ms2;
        if (p.y > gp->global_mark_max_y)
            gp->global_mark_max_y = p.y + ms2;
        g_array_append_val(marks->points, p);
        break;
    case STRING_TEXT:
        {
            text_mark_t *tm = (text_mark_t*)g_new(text_mark_t, 1);
            sscanf(S_, "%s %lf %lf", dummy, &p.x, &p.y);
            tm->text_align = 1;
            if (S_[1] >= '0' && S_[1] <= '9') 
                tm->text_align = atoi(&S_[1]);
            tm->string = string_strdup_rest(S_, 3, TRUE);
            p.op = OP_TEXT;
            p.text_object = tm;
            g_array_append_val(marks->points, p);
        }
        break;
    case STRING_CHANGE_LINE_WIDTH:
        marks->line_width = string_to_atof(S_, 1);
        break;
    case STRING_BALLOON:
        {
            char *s = string_strdup_rest(S_,1,TRUE);
            if (!marks->balloon_string)
                marks->balloon_string = g_string_new(s);
            else {
                g_string_append(marks->balloon_string, "\n");
                g_string_append(marks->balloon_string, s);
            }
            g_free(s);
            
            break;
        }
    case STRING_IMAGE_REFERENCE:
        {
            char *image_filename = string_strdup_word(S_, 1);

            if (gp->cb_file_reference)
                (*(gp->cb_file_reference))(image_filename, gp->cb_file_reference_data);
            // In contrast to other references image commands are carried out immediately!
            // salfw_viewer_load_file(image_filename, 0);
                  
            g_free(image_filename);

            break;
        }
    case STRING_VFLIP:
        if (gp->cb_set_orientation)
            (*(gp->cb_set_orientation))(GIV_PARSER_ORIENTATION_UNDEF, GIV_PARSER_ORIENTATION_FLIP, gp->cb_set_orientation_data);
        break;
    case STRING_VLOCK:
        if (gp->cb_set_vlock)
            (*(gp->cb_set_vlock))(1,gp->cb_set_vlock_data);
        break;
    case STRING_NO_VFLIP:
        if (gp->cb_set_orientation)
            (*(gp->cb_set_orientation))(GIV_PARSER_ORIENTATION_UNDEF, GIV_PARSER_ORIENTATION_UNFLIP, gp->cb_set_orientation_data);
        break;
    case STRING_HFLIP:
        if (gp->cb_set_orientation)
            (*(gp->cb_set_orientation))(GIV_PARSER_ORIENTATION_FLIP, GIV_PARSER_ORIENTATION_UNDEF, gp->cb_set_orientation_data);
        break;
    case STRING_NO_HFLIP:
        if (gp->cb_set_orientation)
            (*(gp->cb_set_orientation))(GIV_PARSER_ORIENTATION_UNFLIP, GIV_PARSER_ORIENTATION_UNDEF, gp->cb_set_orientation_data);
        break;
    case STRING_MARKS_REFERENCE:
        {
#if 0
            char *marks_filename = string_strdup_word(S_, 1);
                
            // Todo: Make image relative to the marks list
            g_ptr_array_add(mark_file_name_list, marks_filename);
                
#endif
            break;
        }
    case STRING_LOW_CONTRAST:
        {
#if 0
            giv_current_transfer_function = TRANS_FUNC_LOW_CONTRAST;
#endif
            break;
        }
    case STRING_DASH:
        {
            // Get string and split it on commas
            char *p;
            char *dash_string = string_strdup_word(S_,1);
            int i, num_dashes;
            
            // Replace comma with spaces for easier parsing
            p = dash_string;
            while((p=g_strstr_len(p, strlen(dash_string), ","))) 
                *p = ' ';
            
            num_dashes = string_count_words(dash_string);
            marks->num_dashes = num_dashes;
            marks->dashes = g_new0(double, num_dashes);
            for (i=0; i<num_dashes; i++)
                marks->dashes[i] = string_to_atof(dash_string, i);
            g_free(dash_string);
            
            break;
        }
    case STRING_ARROW:
        {
            char *arrow_string = string_strdup_word(S_,1);
            if (arrow_string == NULL)
                marks->arrow_type = ARROW_TYPE_END;
            if (strcmp(arrow_string, "start")==0)
                marks->arrow_type = ARROW_TYPE_START;
            else if (strcmp(arrow_string, "both")==0)
                marks->arrow_type = ARROW_TYPE_BOTH;
            else 
                marks->arrow_type = ARROW_TYPE_END;
            if (arrow_string)
                free(arrow_string);
            break;
        }
    case STRING_CHANGE_NO_LINE:
        marks->do_draw_lines = FALSE;
        break;
    case STRING_CHANGE_POLYGON:
        marks->do_draw_polygon = TRUE;
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
    case STRING_CHANGE_TEXT_SIZE:
        marks->text_size = string_to_atof(S_, 1);
        break;
    case STRING_FONT:
        if (marks->font_name)
            g_free(marks->font_name);
        marks->font_name = string_strdup_rest(S_, 1, TRUE);
        break;
    case STRING_CHANGE_COLOR:
        {
            char *color_name = string_strdup_word(S_, 1);
            GivColor color = {0,0,0,0};
            if ((slip)color_name == "none")
                marks->color.alpha = COLOR_NONE;
            else {
                if (color_parse(color_name,&color))
                    marks->color = color;
            }
            g_free(color_name);
            break;
        }
    case STRING_CHANGE_OUTLINE_COLOR:
        {
            char *color_name = string_strdup_word(S_, 1);
            GivColor color = {0,0,0,0};
            if ((slip)color_name == "none")
                marks->outline_color.alpha = COLOR_NONE;
            else {
                if (color_parse(color_name,&color))
                    marks->outline_color = color;
            }
            marks->do_draw_polygon_outline = TRUE;
            g_free(color_name);
            break;
        }
    case STRING_CHANGE_QUIVER_COLOR:
        {
            char *color_name = string_strdup_word(S_, 1);
            GivColor color = {0,0,0,0};
            if ((slip)color_name == "none")
                marks->quiver_color.alpha = COLOR_NONE;
            else {
                if (color_parse(color_name,&color))
                    marks->quiver_color = color;
            }
            g_free(color_name);
            break;
        }
    case STRING_CHANGE_SHADOW_COLOR:
        {
            char *color_name = string_strdup_word(S_, 1);
            GivColor color = {0,0,0,0};
            if ((slip)color_name == "none")
                marks->shadow_color.alpha = COLOR_NONE;
            else {
                if (color_parse(color_name,&color))
                    marks->shadow_color = color;
            }
            g_free(color_name);
            break;
        }
    case STRING_CHANGE_SHADOW_OFFSET:
        {
            char *arg1 = string_strdup_word(S_, 1);
            char *arg2 = string_strdup_word(S_, 2);
            marks->shadow_offset_x = atof(arg1);
            marks->shadow_offset_y = atof(arg2);
            g_free(arg1);
            g_free(arg2);
        }
    case STRING_CHANGE_QUIVER_HEAD:
        {
            char *arg = string_strdup_word(S_, 1);
            if ((slip)arg == "none")
                marks->quiver_head = false;
            else {
                marks->quiver_head = true;
            }
            g_free(arg);
            break;
        }
    case STRING_CHANGE_QUIVER_SCALE:
        marks->quiver_scale = string_to_atof(S_, 1);

        // remember last value
        gp->quiver_scale = marks->quiver_scale;
        break;
    case STRING_CHANGE_MARKS:
        {
            char *mark_name = string_strdup_word(S_, 1);
            marks->do_draw_marks = TRUE;
	      
            marks->mark_type = giv_parse_mark_type(mark_name, filename, linenum);
		
            g_free(mark_name);
            break;
        }
    case STRING_CHANGE_SCALE_MARKS:
        if (string_count_words(S_) == 1)
            marks->do_scale_marks = 1;
        else
            marks->do_scale_marks = string_to_atoi(S_, 1);
        break;
    case STRING_CHANGE_SCALE_FONTS:
        if (string_count_words(S_) == 1)
            marks->do_scale_fonts = 1;
        else
            marks->do_scale_fonts = string_to_atoi(S_, 1);
        break;
    case STRING_CHANGE_PANGO_MARKUP:
        if (string_count_words(S_) == 1)
            marks->do_pango_markup = 1;
        else
            marks->do_pango_markup = string_to_atoi(S_, 1);
        break;
    case STRING_PATH_NAME:
        if (marks->path_name)
            g_free(marks->path_name);
        marks->path_name = string_strdup_rest(S_, 1,FALSE);
        break;
    case STRING_DEF_STYLE:
        {
            char *style_name = string_strdup_word(S_, 1);
            char *style_string = string_strdup_rest(S_, 2,FALSE);
            giv_parser_giv_style_add_string(gp, style_name, style_string);
            g_free(style_name);
            g_free(style_string);
        }
        break;
    case STRING_STYLE:
        {
            char *style_name = string_strdup_word(S_, 1);
            giv_parser_giv_set_props_from_style(gp, marks, style_name);
            g_free(style_name);
            break;
        }
    case STRING_HIDE:
        {
            marks->is_visible = false;
            break;
        }
    case STRING_TEXT_STYLE:
        {
          char *text_style = string_strdup_word(S_,1);
          if (strcmp(text_style, "shadow") == 0)
            marks->text_style = TEXT_STYLE_DROP_SHADOW;
          g_free(text_style);
          break;
        }
    default:
        ;
#if 0
    default:
        printf("Unknown string: %s\n", S_);
#endif
    }
    return 0;
}

void
giv_parser_giv_style_add_string(GivParser *gp,
                                const char* style_name,
                                const char* style_string)
{
    llip matches;

    if (slip(style_string).m("^(\\S+)\\s+(.*)", matches)) {
        // Search if the same property is already defined, then overwrite it.
        slip key = matches[1];
        GPtrArray *string_array = (GPtrArray*)g_hash_table_lookup(gp->style_hash, key);

        // If the key doesn't exist, insert it into the hash
        if (string_array == NULL) {
            string_array = g_ptr_array_new();

            g_ptr_array_add(string_array,
                            g_strdup(style_string));
            
            g_hash_table_insert(gp->style_hash,
                                g_strdup(style_name),
                                string_array);
            return;
        }

        // Check if the key exist
        int key_index = giv_string_array_find(string_array, key);
        if (key_index < 0) {
            g_ptr_array_add(string_array,
                            g_strdup(style_string));
        }
        else {
            giv_string_array_replace(string_array,
                                     key_index,
                                     g_strdup(style_string));
        }
    }
}

void
giv_parser_giv_set_props_from_style(GivParser *gp,
                                    giv_dataset_t *marks,
                                    const char *style_name)
{
    GPtrArray *style_array = (GPtrArray*)g_hash_table_lookup(gp->style_hash, style_name);
    
    if (!style_array) 
        return;

    int prop_idx;

    /* Loop over all style properties and parse them */
    for (prop_idx=0; prop_idx < (int)style_array->len; prop_idx++) {
        slip style_string = (char*)g_ptr_array_index(style_array, prop_idx);
        llip args = style_string.split();

        slip S_ = args.shift();
        S_.tr("A-Z","a-z"); // lower case

        CASE("lw") {
            marks->line_width = args.shift().atof();
            continue;
        }
        CASE("color") {
            slip color_name = args.shift();
            GivColor color = {0,0,0,0};
            if (color_name == "none")
                marks->color.alpha = COLOR_NONE;
            else {
                if (color_parse(color_name.c_str(),&color))
                    marks->color = color;
            }
            continue;
        }
        CASE("outline_color") {
            slip color_name = args.shift();
            GivColor color = {0,0,0,0};
            if (color_parse(color_name,&color))
                marks->outline_color = color;
            marks->do_draw_polygon_outline = TRUE;
            continue;
        }
        CASE("marks") {
            slip mark_name = args.shift();
            marks->do_draw_marks = TRUE;
	  
            marks->mark_type = giv_parse_mark_type(mark_name, style_name, prop_idx);
	  
            continue;
        }
        CASE("noline") {
            marks->do_draw_lines = FALSE;
            continue;
        }
        CASE("line") {
            marks->do_draw_lines = TRUE;
            continue;
        }
        CASE("scale_marks") {
            if (args.length() > 0)
                marks->do_scale_marks = args[1].atoi();
            else 
                marks->do_scale_marks = 1;
            continue;
        }
        CASE("mark_size") {
            if (args.length() > 0)
                marks->mark_size = args[1].atof();
            continue;
        }
        CASE("nomark") {
            marks->do_draw_marks = FALSE;
            continue;
        }
        CASE("polygon") {
            marks->do_draw_polygon = TRUE;
            continue;
        }
        CASE("arrow") {
            if (args.length() < 1
                || args[1] == "end") {
                marks->arrow_type = ARROW_TYPE_END;
            }
            else if (args[1] == "start") 
                marks->arrow_type = ARROW_TYPE_START;
            else if (args[1] == "both")
                marks->arrow_type = ARROW_TYPE_BOTH;
        }
        printf("Unknown option %s!\n", (const char*)S_);
    }
}

void
giv_parser_set_reference_callback(GivParser *gp,
                                  giv_parser_file_reference_t fr,
                                  gpointer user_data)
{
    gp->cb_file_reference = fr;
    gp->cb_file_reference_data = user_data;
}

void
giv_parser_set_orientation_callback(GivParser *gp,
                                    giv_parser_set_orientation_t cb,
                                    gpointer user_data)
{
    gp->cb_set_orientation = cb;
    gp->cb_set_orientation_data = user_data;
}

void
giv_parser_set_vlock_callback(GivParser *gp,
                              giv_parser_set_vlock_t cb,
                              gpointer user_data)
{
    gp->cb_set_vlock = cb;
    gp->cb_set_vlock_data = user_data;
}

void
giv_parser_clear(GivParser *gp)
{
    for (int i=0; i<(int)gp->giv_datasets->len; i++)
        free_giv_data_set((giv_dataset_t*)g_ptr_array_index(gp->giv_datasets, i));
    g_ptr_array_free(gp->giv_datasets, TRUE);
    gp->giv_datasets = g_ptr_array_new();

    gp->global_mark_max_x = gp->global_mark_max_y = -1e30; // -HUGE
    gp->global_mark_min_x = gp->global_mark_min_y = 1e30; // HUGE
}

void
giv_parser_get_data_bbox(GivParser *gp,
                         double *min_x, double* min_y,
                         double *max_x, double* max_y)
{
    *min_x = gp->global_mark_min_x;
    *min_y = gp->global_mark_min_y;
    *max_x = gp->global_mark_max_x;
    *max_y = gp->global_mark_max_y;
}

// Propagate quiver scale
void
giv_parser_set_quiver_scale(GivParser *gp,
                            double quiver_scale)
{
    for (int i=0; i<(int)gp->giv_datasets->len; i++) {
        giv_dataset_t *dataset = (giv_dataset_t*)g_ptr_array_index(gp->giv_datasets, i);
        dataset->quiver_scale = quiver_scale;
    }
}

static
int color_parse(const char *giv_color_name,
                 // output
                 GivColor *color)
{
    int ret = 1;
    const char *slash_pos = strstr(giv_color_name, "/");

    if (slash_pos) {
        GdkColor gcolor;
        char *color_name = g_strndup(giv_color_name,
                                     slash_pos-giv_color_name);
        const char *alpha_string = slash_pos+1;
        gdk_color_parse(color_name,
                        // output
                        &gcolor);
        color->red = gcolor.red;
        color->green = gcolor.green;
        color->blue = gcolor.blue;

        // The following syntaxes for alpha are recognized
        // .../0.5
        // .../0xff
        // .../0xffff

        if (alpha_string[0] == '0'
            && alpha_string[1] == 'x') {
            if (strlen(alpha_string) == 4) {
                color->alpha = 0xff * ((alpha_string[2]<<8) + alpha_string[3]);
            }
            else if (strlen(alpha_string) == 6) {
                color->alpha = (alpha_string[2] << 24)
                    +(alpha_string[3] << 16)
                    +(alpha_string[4] << 8)
                    +(alpha_string[5]);
            }
            else
                color->alpha = 0xffff;
        }
        else if (strstr(alpha_string, ".")  != NULL) {
            color->alpha = (guint16)(0xffff * atof(alpha_string));
        }
        else {
            color->alpha = 255 * atoi(alpha_string);
        }
        g_free(color_name);
    }
    else {
        GdkColor gcolor;
        gdk_color_parse(giv_color_name,
                        // output
                        &gcolor);
        color->red = gcolor.red;
        color->green = gcolor.green;
        color->blue = gcolor.blue;
        color->alpha = 0xffff;
    }
    
    return ret;
}

GPtrArray *giv_parser_get_giv_datasets(GivParser *gp)
{
    return gp->giv_datasets;
}

int giv_parser_count_marks(GivParser *gp,
                           double x0, double y0,
                           double x1, double y1)
{
    double min_x = x0;
    double max_x = x1;
    if (min_x > max_x) {
        double tmp = min_x;
        min_x = max_x;
        max_x = tmp;
    }
    double min_y = y0;
    double max_y = y1;
    if (min_y > max_y) {
        double tmp = min_y;
        min_y = max_y;
        max_y = tmp;
    }
        
        
    int num_marks = 0;
    for (int i=0; i<(int)gp->giv_datasets->len; i++) {
        giv_dataset_t *dataset = (giv_dataset_t*)g_ptr_array_index(gp->giv_datasets, i);

        if (!dataset->is_visible)
            continue;
        for (int p_idx=0; p_idx<(int)dataset->points->len; p_idx++) {
            point_t p = g_array_index(dataset->points, point_t, p_idx);

            if (p.x >= min_x && p.x <= max_x
                && p.y >= min_y && p.y <= max_y)
                num_marks++;
        }
    }

    return num_marks;
}
    
