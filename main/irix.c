#include <string.h>
#include <unistd.h>
#include "irix.h"

#ifdef __sgi

/**
 * We dont have getopt_long on irix, so here is a poor and
 * partial implementation
 */
int getopt_long(int argc, const char **argv, const char *optstring,
	const struct option *longopts, int *longindex)
{
    int i;
    static int index = -1;

    if (index >= argc)
	return -1;
    index++;
    for (i = 0; longopts[i].name; i++)
    {
	if (argv[index])
	if (strlen(argv[index]) > 2)
	{
            if (!strcmp(longopts[i].name, argv[index] + 2))
            {
		*longindex = index;
		if (index < argc - 1)
		{
		    optarg = argv[index + 1];
		    index++;
		}
		*longindex = index;
		optind = index + 1;
	        return longopts[i].val;
	    }
	}else if ((strlen(argv[index]) == 2) && (argv[index][0] == '-'))
	    return argv[index][1];
    }
    return 0;
}

/**
 * Current nekoware GTK2+ is 2.10, but gtk_widget_set_tooltip_text
 * is >= 2.12. Should be able to remove this when we'll upgrade.
 */
//#ifdef USE_GTK
#ifndef NO_GUI
void gtk_widget_set_tooltip_text(GtkWidget* widget, const gchar* sText)
{
    static GtkTooltips* tooltips;
    if(!tooltips)
	tooltips = gtk_tooltips_new();
    gtk_tooltips_set_tip(tooltips, widget, sText, NULL);
}
#endif // NO_GUI
//#endif // USE_GTK

#endif // __sgi

