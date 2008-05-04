#ifndef _ui_disasm_list_h_included_
#define _ui_disasm_list_h_included_

#include <gtk/gtk.h>

/* Some boilerplate GObject defines. 'klass' is used
 *   instead of 'class', because 'class' is a C++ keyword */

#define TYPE_DISASM_LIST            (disasm_list_get_type ())
#define DISASM_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_DISASM_LIST, DisasmList))
#define DISASM_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_DISASM_LIST, DisasmListClass))
#define DISASM_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_DISASM_LIST))
#define DISASM_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_DISASM_LIST))
#define DISASM_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_DISASM_LIST, DisasmListClass))

/* The data columns that we export via the tree model interface */

enum
{
  DISASM_LIST_COL_RECORD = 0,
  DISASM_LIST_COL_NAME,
  DISASM_LIST_COL_YEAR_BORN,
  DISASM_LIST_N_COLUMNS,
} ;


typedef struct _DisasmList       DisasmList;
typedef struct _DisasmListClass  DisasmListClass;



/* DisasmList: this structure contains everything we need for our
 *             model implementation. You can add extra fields to
 *             this structure, e.g. hashtables to quickly lookup
 *             rows or whatever else you might need, but it is
 *             crucial that 'parent' is the first member of the
 *             structure.                                          */

struct _DisasmList
{
  GObject         parent;      /* this MUST be the first member */

  guint           startAddr;    /* number of rows that we have   */

  gint            stamp;       /* Random integer to check whether an iter belongs to our model */
};



/* DisasmListClass: more boilerplate GObject stuff */

struct _DisasmListClass
{
  GObjectClass parent_class;
};


GType             disasm_list_get_type (void);

DisasmList       *disasm_list_new (void);

void disasm_list_update (GtkTreeModel *tree_model, guint address);

gboolean disasm_list_get_iter (GtkTreeModel *tree_model,
		      GtkTreeIter  *iter, GtkTreePath  *path);

#endif /* _disasm_list_h_included_ */
