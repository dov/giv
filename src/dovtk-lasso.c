/** 
 * dovtk-lasso.c
 *
 * A solution for drawing overlays on a gtk widget.
 * 
 * This code is relased under the LGPL v2.0.
 *
 * Copyright Dov Grobgeld <dov.grobgeld@gmail.com> 2010
 * 
 */
#include "dovtk-lasso.h"

typedef struct {
    int num_rectangles;
    cairo_rectangle_t *rectangles;
} DovtkLassoRectangleList;

typedef struct {
    DovtkLasso parent;
    gulong expose_handler_id;
    GtkWidget *widget;
    DovtkLassoDrawing drawing_cb;
    DovtkLassoRectangleList *old_rect_list;
    gpointer user_data;
} DovtkLassoPrivate ;

static int lasso_cb_expose(GtkWidget      *widget,
                           GdkEventExpose *event,
                           gpointer        user_data);
static DovtkLassoRectangleList *dovtk_lasso_rectangle_list_new(int num_rectangles);
static void dovtk_lasso_rectangle_list_destroy(DovtkLassoRectangleList *rectangcle_list);

DovtkLasso *dovtk_lasso_create(GtkWidget *widget,
                               DovtkLassoDrawing drawing_cb,
                               gpointer user_data)
{
    DovtkLassoPrivate *selfp = g_new0(DovtkLassoPrivate, 1);
    
    // This binding doesn't work if the default expose handler
    // returns TRUE!
    selfp->expose_handler_id
        = g_signal_connect_after(widget,
                                 "expose-event",
                                 G_CALLBACK(lasso_cb_expose),
                                 selfp);
    selfp->widget = widget;
    selfp->drawing_cb = drawing_cb;
    selfp->old_rect_list = dovtk_lasso_rectangle_list_new(0);
    selfp->user_data = user_data;
    return (DovtkLasso*)selfp;
}

void dovtk_lasso_destroy(DovtkLasso *lasso)
{
    DovtkLassoPrivate *selfp = (DovtkLassoPrivate*)lasso;
    g_signal_handler_disconnect(selfp->widget,
                                selfp->expose_handler_id);
    // This gets rid of the overlay. Is this always needed?
    dovtk_lasso_update(lasso);
    
    dovtk_lasso_rectangle_list_destroy(selfp->old_rect_list);

    g_free(lasso);
}

void dovtk_lasso_clear_exprects(DovtkLasso *lasso)
{
    DovtkLassoPrivate *selfp = (DovtkLassoPrivate*)lasso;
    dovtk_lasso_rectangle_list_destroy(selfp->old_rect_list);
    selfp->old_rect_list = dovtk_lasso_rectangle_list_new(0);
}

static int lasso_cb_expose(GtkWidget      *widget,
                           GdkEventExpose *event,
                           gpointer        user_data)
{
    DovtkLassoPrivate *selfp = (DovtkLassoPrivate*)user_data;
    //    printf("dovtk-lasso.c: expose\n");

#if 0
    g_signal_handler_block(widget, selfp->expose_handler_id);
    int retval;
    g_signal_emit_by_name (widget, "expose-event", event, &retval);
    g_signal_handler_unblock(widget, selfp->expose_handler_id);
#endif

    cairo_t *cr;
    cr = gdk_cairo_create(widget->window);
    cairo_rectangle(cr, event->area.x, event->area.y,
                    event->area.width, event->area.height);
    cairo_clip(cr);

    selfp->drawing_cb(cr, DOVTK_LASSO_CONTEXT_PAINT, selfp->user_data);

    cairo_destroy(cr);

    return TRUE;
}

int a8_idx=0;

static DovtkLassoRectangleList *get_exprects_from_drawing(DovtkLassoPrivate *selfp)
{
    // Call drawing_cb to and use it to generate the rectangle list
    DovtkLassoRectangleList *rect_list = NULL;
    int scale_factor = 32;
    int low_res_width = (selfp->widget->allocation.width+scale_factor-1) / scale_factor;
    int low_res_height = (selfp->widget->allocation.height+scale_factor-1) / scale_factor;
    
    // This should be created in the creation of DovtkLasso
    cairo_t *cr = NULL;
    cairo_surface_t *surf = NULL;

    surf=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                    low_res_width,
                                    low_res_height);
    cr = cairo_create(surf);
    cairo_set_source_rgba(cr,0,0,0,0);
    cairo_rectangle(cr, 0,0,low_res_height,low_res_width);
    cairo_fill(cr);
    cairo_set_source_rgba(cr,0,0,0,1);

    cairo_scale(cr,1.0/scale_factor,1.0/scale_factor);
    selfp->drawing_cb(cr, TRUE, selfp->user_data);

    // Turn surf into a list of rectangles
    int row_idx, col_idx;

    // Allocate a lot of space
    rect_list = dovtk_lasso_rectangle_list_new(low_res_width*low_res_height);

    guint8 *buf = cairo_image_surface_get_data(surf);
    int rect_idx = 0;
    int row_stride = cairo_image_surface_get_stride(surf);
    for (row_idx=0; row_idx<low_res_height; row_idx++) {
        for (col_idx=0; col_idx<low_res_width; col_idx++) {
            // Check if the tile is "dirty" and then add it to the
            // rect list.
            if (*(buf + row_stride * row_idx + col_idx * 4+3) > 0) {
                cairo_rectangle_t *rect = &rect_list->rectangles[rect_idx++];
                rect->x = col_idx*scale_factor;
                rect->y = row_idx*scale_factor;
                rect->width = scale_factor;
                rect->height = scale_factor;
            }
        }
    }
    rect_list->num_rectangles = rect_idx;
    
    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    return rect_list;
}

static DovtkLassoRectangleList *rect_cat(DovtkLassoRectangleList *rect1,
                                         DovtkLassoRectangleList *rect2)
{
    int num_rects1 = rect1->num_rectangles;
    int num_rects2 = rect2->num_rectangles;
    DovtkLassoRectangleList *rect_list
        = dovtk_lasso_rectangle_list_new(num_rects1+num_rects2);
    int i;
    for (i=0; i<num_rects1; i++) 
        rect_list->rectangles[i] = rect1->rectangles[i];
    for (i=0; i<num_rects2; i++)
        rect_list->rectangles[num_rects1 + i] = rect2->rectangles[i];
    return rect_list;
}

void dovtk_lasso_update(DovtkLasso *lasso)
{
    DovtkLassoPrivate *selfp = (DovtkLassoPrivate*)lasso;

    // Call drawing_cb to and use it to generate the rectangle list
    DovtkLassoRectangleList *rect_list = get_exprects_from_drawing(selfp);

    // Build a list of expose rectangles from the old and the new lists.
    // Better done as a linked list.
    DovtkLassoRectangleList *expose_rect_list
        = rect_cat(selfp->old_rect_list, rect_list);

    // Expose the old and the new list of rectangles!
    int i;
    for (i=0; i<expose_rect_list->num_rectangles; i++) {
        // Shortcut
        cairo_rectangle_t *lasso_rect = &expose_rect_list->rectangles[i];
        
        GdkRectangle rect;
        rect.x = lasso_rect->x;
        rect.y = lasso_rect->y;
        rect.width = lasso_rect->width;
        rect.height = lasso_rect->height;
#if 0
        printf("Invalidate region (%d,%d,%d,%d).\n",
               rect.x,rect.y,rect.width,rect.height);
#endif
        gdk_window_invalidate_rect(selfp->widget->window,
                                   &rect,
                                   TRUE);
    }
    dovtk_lasso_rectangle_list_destroy(expose_rect_list);

    dovtk_lasso_rectangle_list_destroy(selfp->old_rect_list);
    selfp->old_rect_list = rect_list;
}

DovtkLassoRectangleList *dovtk_lasso_rectangle_list_new(int num_rectangles)
{
    DovtkLassoRectangleList *rectangle_list = g_new0(DovtkLassoRectangleList, 1);
    rectangle_list->num_rectangles = num_rectangles;
    rectangle_list->rectangles = g_new0(cairo_rectangle_t, num_rectangles);
    return rectangle_list;
}

void dovtk_lasso_rectangle_list_destroy(DovtkLassoRectangleList *rectangle_list)
{
    g_free(rectangle_list->rectangles);
    g_free(rectangle_list);
}

int dovtk_lasso_get_label_for_pixel(DovtkLasso *lasso,
                                    int col_idx, int row_idx)
{
    DovtkLassoPrivate *selfp = (DovtkLassoPrivate*)lasso;
    cairo_t *cr = NULL;
    cairo_surface_t *surf = NULL;

    surf=cairo_image_surface_create(CAIRO_FORMAT_RGB24,1,1);
    cr = cairo_create(surf);
    cairo_translate(cr,-col_idx,-row_idx);
    
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

    selfp->drawing_cb(cr, DOVTK_LASSO_CONTEXT_LABEL, selfp->user_data);

    guint8 *buf = cairo_image_surface_get_data(surf);
    int label = buf[2]+256*buf[1]+256*256*buf[0];
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
    return label;
}

/** 
 * The label is encoded in the RGB image by its color.
 * 
 * @param lasso 
 * @param cr 
 * @param label 
 */
void dovtk_lasso_set_color_label(DovtkLasso *lasso,
                                cairo_t *cr,
                                int label)
{
    double rr = 1.0*label/255;
    double gg = (1.0*(label>>8))/255;
    double bb = (1.0*(label>>16))/255;

    cairo_set_source_rgb(cr,rr,gg,bb);
}

/** 
 * Use the drawing routine to add more rects list without exposing
 * the drawing.
 * 
 * @param lasso 
 */
void dovtk_lasso_add_exprects_from_drawing_cb(DovtkLasso *lasso)
{
    DovtkLassoPrivate *selfp = (DovtkLassoPrivate*)lasso;
    DovtkLassoRectangleList *rect_list = get_exprects_from_drawing(selfp);
    DovtkLassoRectangleList *new_rect_list = rect_cat(selfp->old_rect_list,
                                                      rect_list);
    dovtk_lasso_rectangle_list_destroy(selfp->old_rect_list);
    selfp->old_rect_list = new_rect_list;
}
