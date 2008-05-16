//
// TASinput Configuration Screen for GTK+2.xx
// written by NMN
// --
// I may have took some 'tips' from Glade. Sue me.
//

#include <gtk/gtk.h>

#define GTKHUOBJECT(component,widget,name) \
g_object_set_data_full (G_OBJECT (component), name, \
gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GTKHUOBJECT_NO_REF(component,widget,name) \
g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget* create_ControllerEdit (void);
GtkWidget* create_MacroEdit (void);
GtkWidget* create_EventEdit (void);
bool configure();

