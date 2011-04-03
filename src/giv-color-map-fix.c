/*======================================================================
 * This is giv-colormap with use of gregex replaced with givregex.
 *
 * It should be automatically generated from giv-color-map.c.
 */

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <float.h>
#include <math.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <cairo.h>
#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include "givregex.h"

#define TYPE_GIV_COLOR_MAP (giv_color_map_get_type ())
#define GIV_COLOR_MAP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_GIV_COLOR_MAP, GivColorMap))
#define GIV_COLOR_MAP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GIV_COLOR_MAP, GivColorMapClass))
#define IS_GIV_COLOR_MAP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_GIV_COLOR_MAP))
#define IS_GIV_COLOR_MAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GIV_COLOR_MAP))
#define GIV_COLOR_MAP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GIV_COLOR_MAP, GivColorMapClass))

typedef struct _GivColorMap GivColorMap;
typedef struct _GivColorMapClass GivColorMapClass;
typedef struct _GivColorMapPrivate GivColorMapPrivate;

/*  This is a widget that displays the current colormap that is used.
----------------------------------------------------------------------*/
struct _GivColorMap {
	GtkDrawingArea parent_instance;
	GivColorMapPrivate * priv;
	gint* colormap;
	gint colormap_length1;
	double min;
	double max;
	gboolean is_horizontal;
};

struct _GivColorMapClass {
	GtkDrawingAreaClass parent_class;
};


static gpointer giv_color_map_parent_class = NULL;

GType giv_color_map_get_type (void);
enum  {
	GIV_COLOR_MAP_DUMMY_PROPERTY
};
void giv_color_map_set_default_map (GivColorMap* self);
void giv_color_map_redraw (GivColorMap* self);
void giv_color_map_set_rgb_colormap (GivColorMap* self, guint8* colormap, int colormap_length1);
void giv_color_map_show_text (GivColorMap* self, cairo_t* cr, const char* text, PangoAlignment alignment);
void giv_color_map_cshow (GivColorMap* self, cairo_t* cr, const char* text);
void giv_color_map_lshow (GivColorMap* self, cairo_t* cr, const char* text);
void giv_color_map_rshow (GivColorMap* self, cairo_t* cr, const char* text);
void giv_color_map_set_min_max (GivColorMap* self, double min, double max);
static char* giv_color_map_double_to_markup (GivColorMap* self, double v);
static gboolean giv_color_map_real_expose_event (GtkWidget* base, const GdkEventExpose* event);
GivColorMap* giv_color_map_new (void);
GivColorMap* giv_color_map_construct (GType object_type);
static GObject * giv_color_map_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static void giv_color_map_finalize (GObject* obj);
static int _vala_strcmp0 (const char * str1, const char * str2);



void giv_color_map_set_default_map (GivColorMap* self) {
	g_return_if_fail (self != NULL);
	{
		gint i;
		/* Default colormap*/
		i = 0;
		{
			gboolean _tmp0_;
			_tmp0_ = TRUE;
			while (TRUE) {
				if (!_tmp0_) {
					i++;
				}
				_tmp0_ = FALSE;
				if (!(i < 256)) {
					break;
				}
				self->colormap[(i * 3) + 0] = i;
				self->colormap[(i * 3) + 1] = i;
				self->colormap[(i * 3) + 2] = i;
			}
		}
	}
}


void giv_color_map_set_rgb_colormap (GivColorMap* self, guint8* colormap, int colormap_length1) {
	g_return_if_fail (self != NULL);
	if (colormap == NULL) {
		giv_color_map_set_default_map (self);
	} else {
		{
			gint i;
			i = 0;
			{
				gboolean _tmp0_;
				_tmp0_ = TRUE;
				while (TRUE) {
					if (!_tmp0_) {
						i++;
					}
					_tmp0_ = FALSE;
					if (!(i < (256 * 3))) {
						break;
					}
					self->colormap[i] = (gint) colormap[i];
				}
			}
		}
	}
	giv_color_map_redraw (self);
}


void giv_color_map_redraw (GivColorMap* self) {
	g_return_if_fail (self != NULL);
	gtk_widget_queue_draw_area ((GtkWidget*) self, 0, 0, ((GtkWidget*) self)->allocation.width, ((GtkWidget*) self)->allocation.height);
}


void giv_color_map_show_text (GivColorMap* self, cairo_t* cr, const char* text, PangoAlignment alignment) {
	PangoRectangle ink_rect = {0};
	PangoRectangle logical_rect = {0};
	PangoFontDescription* font_description;
	PangoLayout* layout;
	g_return_if_fail (self != NULL);
	g_return_if_fail (cr != NULL);
	g_return_if_fail (text != NULL);
	font_description = pango_font_description_new ();
	pango_font_description_set_family (font_description, "Sans ");
	pango_font_description_set_size (font_description, (gint) (9 * PANGO_SCALE));
	layout = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (layout, font_description);
	pango_layout_set_markup (layout, text, -1);
	pango_layout_get_extents (layout, &ink_rect, &logical_rect);
	if (alignment == PANGO_ALIGN_CENTER) {
		cairo_rel_move_to (cr, ((-logical_rect.width) / 2.0) / PANGO_SCALE, ((-logical_rect.height) / 2.0) / PANGO_SCALE);
	} else {
		if (alignment == PANGO_ALIGN_RIGHT) {
			cairo_rel_move_to (cr, (double) ((-logical_rect.width) / PANGO_SCALE), ((-logical_rect.height) / 2.0) / PANGO_SCALE);
		} else {
			cairo_rel_move_to (cr, (double) 0, ((-logical_rect.height) / 2.0) / PANGO_SCALE);
		}
	}
	pango_cairo_show_layout (cr, layout);
	(font_description == NULL) ? NULL : (font_description = (pango_font_description_free (font_description), NULL));
	(layout == NULL) ? NULL : (layout = (g_object_unref (layout), NULL));
}


void giv_color_map_cshow (GivColorMap* self, cairo_t* cr, const char* text) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (cr != NULL);
	g_return_if_fail (text != NULL);
	giv_color_map_show_text (self, cr, text, PANGO_ALIGN_CENTER);
}


void giv_color_map_lshow (GivColorMap* self, cairo_t* cr, const char* text) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (cr != NULL);
	g_return_if_fail (text != NULL);
	giv_color_map_show_text (self, cr, text, PANGO_ALIGN_LEFT);
}


void giv_color_map_rshow (GivColorMap* self, cairo_t* cr, const char* text) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (cr != NULL);
	g_return_if_fail (text != NULL);
	giv_color_map_show_text (self, cr, text, PANGO_ALIGN_RIGHT);
}


void giv_color_map_set_min_max (GivColorMap* self, double min, double max) {
	g_return_if_fail (self != NULL);
	self->min = min;
	self->max = max;
}


static char* giv_color_map_double_to_markup (GivColorMap* self, double v) {
	char* result;
	GError * _inner_error_;
	gboolean _tmp0_;
	g_return_val_if_fail (self != NULL, NULL);
	_inner_error_ = NULL;
	_tmp0_ = FALSE;
	if (v > 100) {
		_tmp0_ = v < 10000;
	} else {
		_tmp0_ = FALSE;
	}
	if (_tmp0_) {
		result = g_strdup_printf ("%.0f", v);
		return result;
	}
	{
		char* format;
		char* s;
		char* s1;
		gint state;
		GivMatchInfo* match_info;
		GivRegex* regex;
		GivMatchInfo* _tmp3_;
		gboolean _tmp2_;
		GivMatchInfo* _tmp1_;
		format = g_strdup ("%.3g");
		s = g_strdup_printf (format, v);
		s1 = NULL;
		/* states: 0 before string
		         1 in number*/
		state = 0;
		/* Sorry Ron. You'll have to comment out this section
		 to support old CentOs compilation
		 Cleanup string and turn it into 10^ notation.*/
		match_info = NULL;
		regex = giv_regex_new ("(.*?)e([\\+\\-])0+(\\d+)", 0, 0, &_inner_error_);
		if (_inner_error_ != NULL) {
			format = (g_free (format), NULL);
			s = (g_free (s), NULL);
			s1 = (g_free (s1), NULL);
			(match_info == NULL) ? NULL : (match_info = (giv_match_info_free (match_info), NULL));
			if (_inner_error_->domain == GIV_REGEX_ERROR) {
				goto __catch0_giv_regex_error;
			}
			goto __finally0;
		}
		_tmp3_ = NULL;
		_tmp1_ = NULL;
		if ((_tmp2_ = giv_regex_match (regex, s, (GivRegexMatchFlags) 0, &_tmp1_), match_info = (_tmp3_ = _tmp1_, (match_info == NULL) ? NULL : (match_info = (giv_match_info_free (match_info), NULL)), _tmp3_), _tmp2_)) {
			char* s_sign;
			char* _tmp10_;
			char* _tmp9_;
			char* _tmp8_;
			char* _tmp7_;
			char* _tmp6_;
			char* _tmp5_;
			s_sign = giv_match_info_fetch (match_info, 2);
			if (_vala_strcmp0 (s_sign, "+") == 0) {
				char* _tmp4_;
				_tmp4_ = NULL;
				s_sign = (_tmp4_ = g_strdup (""), s_sign = (g_free (s_sign), NULL), _tmp4_);
			}
			_tmp10_ = NULL;
			_tmp9_ = NULL;
			_tmp8_ = NULL;
			_tmp7_ = NULL;
			_tmp6_ = NULL;
			_tmp5_ = NULL;
			s = (_tmp10_ = g_strconcat (_tmp9_ = g_strconcat (_tmp7_ = g_strconcat (_tmp6_ = g_strconcat (_tmp5_ = giv_match_info_fetch (match_info, 1), "&#8901;10<sup>", NULL), s_sign, NULL), _tmp8_ = giv_match_info_fetch (match_info, 3), NULL), "</sup>", NULL), s = (g_free (s), NULL), _tmp10_);
			_tmp9_ = (g_free (_tmp9_), NULL);
			_tmp8_ = (g_free (_tmp8_), NULL);
			_tmp7_ = (g_free (_tmp7_), NULL);
			_tmp6_ = (g_free (_tmp6_), NULL);
			_tmp5_ = (g_free (_tmp5_), NULL);
			s_sign = (g_free (s_sign), NULL);
		}
		/* Ron - end of uncommenting */
		result = s;
		format = (g_free (format), NULL);
		s1 = (g_free (s1), NULL);
		(match_info == NULL) ? NULL : (match_info = (giv_match_info_free (match_info), NULL));
		(regex == NULL) ? NULL : (regex = (giv_regex_unref (regex), NULL));
		return result;
	}
	goto __finally0;
	__catch0_giv_regex_error:
	{
		GError * e;
		e = _inner_error_;
		_inner_error_ = NULL;
		{
			(e == NULL) ? NULL : (e = (g_error_free (e), NULL));
		}
	}
	__finally0:
	if (_inner_error_ != NULL) {
		g_critical ("file %s: line %d: uncaught error: %s", __FILE__, __LINE__, _inner_error_->message);
		g_clear_error (&_inner_error_);
		return NULL;
	}
	result = g_strdup ("");
	return result;
}


static char* double_to_string (double self) {
	char* result;
	const char* _tmp1_;
	gint _tmp0__length1;
	gchar* _tmp0_;
	char* _tmp2_;
	_tmp1_ = NULL;
	_tmp0_ = NULL;
	_tmp2_ = NULL;
	result = (_tmp2_ = (_tmp1_ = g_ascii_dtostr ((_tmp0_ = g_new0 (gchar, G_ASCII_DTOSTR_BUF_SIZE), _tmp0__length1 = G_ASCII_DTOSTR_BUF_SIZE, _tmp0_), G_ASCII_DTOSTR_BUF_SIZE, self), (_tmp1_ == NULL) ? NULL : g_strdup (_tmp1_)), _tmp0_ = (g_free (_tmp0_), NULL), _tmp2_);
	return result;
}


static gboolean giv_color_map_real_expose_event (GtkWidget* base, const GdkEventExpose* event) {
	GivColorMap * self;
	gboolean result;
	cairo_t* cr;
	gint w;
	gint h;
	self = (GivColorMap*) base;
	cr = gdk_cairo_create ((GdkDrawable*) ((GtkWidget*) self)->window);
	w = ((GtkWidget*) self)->allocation.width;
	h = ((GtkWidget*) self)->allocation.height;
	/* Set clipping area to minimize redrawing*/
	cairo_rectangle (cr, (double) (*event).area.x, (double) (*event).area.y, (double) (*event).area.width, (double) (*event).area.height);
	cairo_clip (cr);
	if (!self->is_horizontal) {
		gint lmargin;
		gint rmargin;
		gint tbmargin;
		gint grad_span;
		double line_width;
		gint line_length;
		gint text_pad;
		gint label_pos_x;
		char* _tmp1_;
		char* _tmp2_;
		/* Draw 256 with colors from start to end*/
		lmargin = 60;
		rmargin = 5;
		tbmargin = 10;
		grad_span = h - (2 * tbmargin);
		line_width = (1.0 * grad_span) / 256;
		line_length = (w - lmargin) - rmargin;
		text_pad = 3;
		cairo_set_line_width (cr, ceil (line_width + 0.5));
		{
			gint i;
			i = 0;
			{
				gboolean _tmp0_;
				_tmp0_ = TRUE;
				while (TRUE) {
					double rr;
					double gg;
					double bb;
					if (!_tmp0_) {
						i++;
					}
					_tmp0_ = FALSE;
					if (!(i < 256)) {
						break;
					}
					/* TBD - use color table lookup*/
					rr = (1.0 / 255) * self->colormap[((255 - i) * 3) + 0];
					gg = (1.0 / 255) * self->colormap[((255 - i) * 3) + 1];
					bb = (1.0 / 255) * self->colormap[((255 - i) * 3) + 2];
					cairo_set_source_rgb (cr, rr, gg, bb);
					cairo_move_to (cr, (double) lmargin, tbmargin + (i * line_width));
					cairo_line_to (cr, (double) (lmargin + line_length), tbmargin + (i * line_width));
					cairo_stroke (cr);
				}
			}
		}
		/* Print labels*/
		label_pos_x = lmargin - text_pad;
		cairo_set_source_rgb (cr, (double) 0, (double) 0, (double) 0);
		cairo_move_to (cr, (double) label_pos_x, (double) tbmargin);
		_tmp1_ = NULL;
		giv_color_map_rshow (self, cr, _tmp1_ = giv_color_map_double_to_markup (self, self->max));
		_tmp1_ = (g_free (_tmp1_), NULL);
		cairo_move_to (cr, (double) label_pos_x, (double) (tbmargin + grad_span));
		_tmp2_ = NULL;
		giv_color_map_rshow (self, cr, _tmp2_ = giv_color_map_double_to_markup (self, self->min));
		_tmp2_ = (g_free (_tmp2_), NULL);
	} else {
		gint margin;
		gint grad_width;
		double line_width;
		gint line_height;
		char* _tmp4_;
		char* _tmp5_;
		/* Draw 256 with colors from start to end*/
		margin = 20;
		grad_width = w - (2 * margin);
		line_width = (1.0 * grad_width) / 256;
		line_height = h - (3 * margin);
		cairo_set_line_width (cr, ceil (line_width + 0.5));
		{
			gint i;
			i = 0;
			{
				gboolean _tmp3_;
				_tmp3_ = TRUE;
				while (TRUE) {
					double rr;
					double gg;
					double bb;
					if (!_tmp3_) {
						i++;
					}
					_tmp3_ = FALSE;
					if (!(i < 256)) {
						break;
					}
					/* TBD - use color table lookup*/
					rr = (1.0 / 255) * self->colormap[(i * 3) + 0];
					gg = (1.0 / 255) * self->colormap[(i * 3) + 1];
					bb = (1.0 / 255) * self->colormap[(i * 3) + 2];
					cairo_set_source_rgb (cr, rr, gg, bb);
					cairo_move_to (cr, margin + (i * line_width), (double) margin);
					cairo_line_to (cr, margin + (i * line_width), (double) (margin + line_height));
					cairo_stroke (cr);
				}
			}
		}
		/* Print labels*/
		cairo_set_source_rgb (cr, (double) 0, (double) 0, (double) 0);
		cairo_move_to (cr, (double) margin, (1.1 * margin) + line_height);
		_tmp4_ = NULL;
		giv_color_map_cshow (self, cr, _tmp4_ = double_to_string (self->min));
		_tmp4_ = (g_free (_tmp4_), NULL);
		cairo_move_to (cr, (double) (margin + grad_width), (1.1 * margin) + line_height);
		_tmp5_ = NULL;
		giv_color_map_cshow (self, cr, _tmp5_ = double_to_string (self->max));
		_tmp5_ = (g_free (_tmp5_), NULL);
	}
	result = TRUE;
	(cr == NULL) ? NULL : (cr = (cairo_destroy (cr), NULL));
	return result;
}


/*  This is a widget that displays the current colormap that is used.
----------------------------------------------------------------------*/
GivColorMap* giv_color_map_construct (GType object_type) {
	GivColorMap * self;
	self = g_object_newv (object_type, 0, NULL);
	return self;
}


GivColorMap* giv_color_map_new (void) {
	return giv_color_map_construct (TYPE_GIV_COLOR_MAP);
}


static GObject * giv_color_map_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GivColorMapClass * klass;
	GObjectClass * parent_class;
	GivColorMap * self;
	klass = GIV_COLOR_MAP_CLASS (g_type_class_peek (TYPE_GIV_COLOR_MAP));
	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = GIV_COLOR_MAP (obj);
	{
		gint _tmp0_;
		_tmp0_ = 256 * 3;
		self->colormap = g_renew (gint, self->colormap, 256 * 3);
		(_tmp0_ > self->colormap_length1) ? memset (self->colormap + self->colormap_length1, 0, sizeof (gint) * (_tmp0_ - self->colormap_length1)) : NULL;
		self->colormap_length1 = _tmp0_;
		giv_color_map_set_default_map (self);
		gtk_widget_set_size_request ((GtkWidget*) self, 80, 300);
		self->min = (double) 0;
		self->max = 255e-5;
		self->is_horizontal = FALSE;
	}
	return obj;
}


static void giv_color_map_class_init (GivColorMapClass * klass) {
	giv_color_map_parent_class = g_type_class_peek_parent (klass);
	GTK_WIDGET_CLASS (klass)->expose_event = giv_color_map_real_expose_event;
	G_OBJECT_CLASS (klass)->constructor = giv_color_map_constructor;
	G_OBJECT_CLASS (klass)->finalize = giv_color_map_finalize;
}


static void giv_color_map_instance_init (GivColorMap * self) {
}


static void giv_color_map_finalize (GObject* obj) {
	GivColorMap * self;
	self = GIV_COLOR_MAP (obj);
	self->colormap = (g_free (self->colormap), NULL);
	G_OBJECT_CLASS (giv_color_map_parent_class)->finalize (obj);
}


GType giv_color_map_get_type (void) {
	static GType giv_color_map_type_id = 0;
	if (giv_color_map_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (GivColorMapClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) giv_color_map_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (GivColorMap), 0, (GInstanceInitFunc) giv_color_map_instance_init, NULL };
		giv_color_map_type_id = g_type_register_static (GTK_TYPE_DRAWING_AREA, "GivColorMap", &g_define_type_info, 0);
	}
	return giv_color_map_type_id;
}


static int _vala_strcmp0 (const char * str1, const char * str2) {
	if (str1 == NULL) {
		return -(str1 != str2);
	}
	if (str2 == NULL) {
		return str1 != str2;
	}
	return strcmp (str1, str2);
}




