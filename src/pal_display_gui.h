#ifndef PAL_DISPLAY_GUI_H
#define PAL_DISPLAY_GUI_H


#include <gtk/gtk.h>

#include "image_info.h"
#include "palette.h"


/*  pal_display

    displays the palette colours as a horizontal strip

    clicking and dragging anywhere within the strip of colours
    acts like a sliding scale.

    the strip rotates when colour cycling is activated.
*/


typedef struct
{
    GtkWidget*  drawing_area;

    image_info* img;

    int ptr_x;
    int org_x;

} pal_display;


pal_display*    pal_display_new(GtkWidget* container);
void            pal_display_free(pal_display*);
void            pal_display_update(pal_display*);



/* pal_affect
*/

typedef struct
{
    GtkWidget* offset;
    GtkWidget* stripe;
    GtkWidget* spread;

} pal_affect;


pal_affect* pal_affect_new(GtkWidget* container,    int* p_offset,
                                                    int* p_stripe,
                                                    int* p_spread );


#endif
