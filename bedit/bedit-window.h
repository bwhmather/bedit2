#pragma once

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BeditWindow BeditWindow;

#define BEDIT_TYPE_WINDOW (bedit_window_get_type())
G_DECLARE_FINAL_TYPE(BeditWindow, bedit_window, BEDIT, WINDOW, GtkApplicationWindow)

BeditWindow *
bedit_window_new(GtkApplication *application);

G_END_DECLS
