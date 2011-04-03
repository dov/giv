//======================================================================
//  test-giv-win.cc - This will be the new giv
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Jul  8 19:32:57 2009
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "gtk/gtk.h"
#include "giv-win.h"

#define CASE(s) if (!strcmp(s, S_))

int main(int argc, char **argv)
{
    GtkWidget *giv;
    gtk_init(&argc, &argv);

    giv = giv_win_new(argc, argv);

    gtk_widget_show(giv);

    gtk_main();

    exit(0);
    return(0);
}
