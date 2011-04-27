/** 
 * dovtk-lasso.h
 *
 * A solution for drawing overlays on a gtk widget.
 * 
 * This code is relased under the LGPL v2.0.
 *
 * Copyright Dov Grobgeld <dov.grobgeld@gmail.com> 2010
 * 
 */
#ifndef DOVTK_H
#define DOVTK_H

#include <gtk/gtk.h>

/** 
 * The drawing callback is used for three purposes:
 *
 *  - Drawing a mask
 *  - Drawing a label image (mapping of pixel to label). Used for
 *    picking.
 *  - Drawing the graphics
 * 
 * @param DovtkLassoDrawing 
 * 
 * @return 
 */
typedef enum {
  DOVTK_LASSO_CONTEXT_PAINT,
  DOVTK_LASSO_CONTEXT_MASK,
  DOVTK_LASSO_CONTEXT_LABEL
} DovtkLassoContext;
  
/**
 * Opaque handle for the lasso
 * 
 */
typedef struct {
} DovtkLasso;

/** 
 * Callback function for the lasso that paints the overlap. If
 * context == DOVTK_LASSO_CONTEXT_MASK, then the drawing alpha channel
 * of the drawing will be used to determine whether the redraw
 * that patch. Typically lines will be drawn thicker when mask is
 * on in order to make sure that the corresponding patch is dirty.
 * 
 * @param DovtkLassoDrawing 
 * 
 * @return 
 */
typedef void (*DovtkLassoDrawing)(cairo_t *cr,
                                  DovtkLassoContext Context,
                                  gpointer user_data);


/**
 * Create a new lasso structure.
 * 
 */
DovtkLasso *dovtk_lasso_create(GtkWidget *widget,
                               DovtkLassoDrawing drawing_cb,
                               gpointer user_data);

/** 
 * Called when the coordinates of the lasso were changed.
 * 
 * @param lasso 
 */
void dovtk_lasso_update(DovtkLasso *lasso);

/** 
 * Destroys a DovtkLasso object.
 * 
 * @param lasso 
 */
void dovtk_lasso_destroy(DovtkLasso *lasso);

/** 
 * Discards all the exprects.
 * 
 * @param lasso 
 */
void dovtk_lasso_clear_exprects(DovtkLasso *lasso);

/** 
 * Create exprects from the current callback
 * 
 * @param lasso 
 */
void dovtk_lasso_add_exprects_from_drawing_cb(DovtkLasso *lasso);

/**
 * Get label for a pixel according to the current drawing routine.
 */
int dovtk_lasso_get_label_for_pixel(DovtkLasso *lasso,
                                    int col_idx, int row_idx);

/**
 * Set the color corresponding to a label.
 */
void dovtk_lasso_set_color_label(DovtkLasso *lasso,
                                 cairo_t *cr,
                                 int label);

#endif /* DOVTK */
