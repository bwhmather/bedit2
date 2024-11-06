#pragma once

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BeditSearchbar BeditSearchbar;

#define BEDIT_TYPE_SEARCHBAR (bedit_searchbar_get_type())
G_DECLARE_FINAL_TYPE(BeditSearchbar, bedit_searchbar, BEDIT, SEARCHBAR, GtkWidget)

BeditSearchbar *
bedit_searchbar_new(GtkApplication *application);

G_END_DECLS
