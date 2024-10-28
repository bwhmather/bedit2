#pragma once

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BeditDocument BeditDocument;

#define BEDIT_TYPE_DOCUMENT (bedit_document_get_type())
G_DECLARE_FINAL_TYPE(BeditDocument, bedit_document, BEDIT, DOCUMENT, GtkWidget)

BeditDocument *
bedit_document_new(GtkApplication *application);

G_END_DECLS
