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
 * Opaque handle for the lasso
 * 
 */
typedef struct {
} DovtkLasso;

/** 
 * Callback function for the lasso that paints the overlap. If
 * do_mask is true, then the drawing alpha channel of the drawing
 * will be used to determine whether the redraw that patch.
 * Typically lines will be drawn thicker when mask is on in
 * order to make sure that the corresponding patch is dirty.
 * 
 * @param DovtkLassoDrawing 
 * 
 * @return 
 */
typedef void (*DovtkLassoDrawing)(cairo_t *cr,
                                  gboolean do_mask,
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

#endif /* DOVTK */
