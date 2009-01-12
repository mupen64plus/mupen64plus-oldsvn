#ifndef __IRIX_H__
#define __IRIX_H__

#ifdef __sgi
#ifndef NO_GUI
#include <gtk/gtk.h>
#endif // NO_GUI

/* we don't have getopt_long on irix */
struct option
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
};
#define no_argument 0
#define required_argument 1

int getopt_long(int, const char **, const char *,
        const struct option *, int *);
//#ifdef USE_GTK
#ifndef NO_GUI
void gtk_widget_set_tooltip_text(GtkWidget*, const gchar*);
#endif // NO_GUI
//#endif // USE_GTK

#endif // __sgi

#endif /* __IRIX_H__ */

