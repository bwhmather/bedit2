#pragma once

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BeditApplication BeditApplication;

#define BEDIT_TYPE_APPLICATION (bedit_application_get_type())
G_DECLARE_FINAL_TYPE(BeditApplication, bedit_application, BEDIT, APPLICATION, GtkApplication)

BeditApplication *
bedit_application_new(void);

G_END_DECLS
