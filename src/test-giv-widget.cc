//======================================================================
//  test-giv-widget.c - Test the giv widget
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Apr  7 15:21:32 2009
//----------------------------------------------------------------------

#include "giv-widget.h"

/* 
 * A basic image viewer using the GtkImageViewer widget that shows
 * the pixel coordinate when moving over the image.
 */
#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>

static gint
cb_key_press_event(GtkWidget *widget, GdkEventKey *event)
{
  gint k = event->keyval;
  
  if (k == 'q')
      exit(0);

  return FALSE;
}

static gint
cb_motion_event(GtkWidget *widget,
                GdkEventButton *event,
                gpointer user_data)
{
    gchar pixel_info[64];
    GtkWidget *label = GTK_WIDGET(user_data);
    double img_x = 0, img_y = 0;

    gtk_image_viewer_canv_coord_to_img_coord(GTK_IMAGE_VIEWER(widget),
                                             event->x, event->y,
                                             &img_x, &img_y);
    
    g_snprintf(pixel_info, sizeof(pixel_info), "(%d,%d)", 
               (int)floor(img_x),
               (int)floor(img_y));
    gtk_label_set_text(GTK_LABEL(label), pixel_info);

    return 0;
}

int 
main (int argc, char *argv[])
{
  GtkWidget *window, *vbox, *image_viewer, *scrolled_win, *label;
  GdkPixbuf *img;
  int width, height;
  
  gtk_init (&argc, &argv);

  if (argc < 2)
    {
      printf("Need name of image!\n");
      exit(0);
    }
  else
    {
      GError *error = NULL;
      img = gdk_pixbuf_new_from_file (argv[1], &error);
    }
    
  width = gdk_pixbuf_get_width (img);
  height = gdk_pixbuf_get_height (img);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, FALSE);
  gtk_window_set_title (GTK_WINDOW (window), argv[1]);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_exit), NULL);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  scrolled_win = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);

  gtk_box_pack_start (GTK_BOX (vbox), scrolled_win,
                      TRUE, TRUE, 0);
  gtk_widget_show(scrolled_win);

  image_viewer = giv_widget_new(img); 
  gtk_widget_set_size_request (image_viewer, width<500?width:500,
                               height<500?height:500); 
  g_signal_connect (window, "key_press_event",
                    G_CALLBACK(cb_key_press_event), NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), image_viewer);
  gtk_widget_show (image_viewer);

  label = gtk_label_new("");
  gtk_box_pack_start (GTK_BOX (vbox),
                      label, FALSE, FALSE, 0);
  gtk_widget_show(label);
  
  g_signal_connect (image_viewer, "motion_notify_event",
                    G_CALLBACK(cb_motion_event), label);

  gtk_widget_show_all (window);

  giv_widget_add_giv_from_string(GIV_WIDGET(image_viewer),
                                 "$color red\n"
                                 "$marks fcircle\n"
                                 "$balloon balloon!\n"
                                 "128 128\n"
                                 "\n"
                                 "$path arrow both \n"
                                 "$arrow both  \n"
                                 "$color pink \n"
                                 "$nomark\n"
                                 "$lw 2 \n"
                                 "128 128\n" 
                                 "256 256 \n"
                                 "\n"
                                 );

  giv_widget_add_giv_from_file(GIV_WIDGET(image_viewer),
                               "/home/dov/pictures/maja.giv");
  
  gtk_main ();
  
  return 0;
}
