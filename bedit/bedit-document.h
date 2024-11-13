#pragma once

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BeditDocument BeditDocument;

#define BEDIT_TYPE_DOCUMENT (bedit_document_get_type())
G_DECLARE_FINAL_TYPE(BeditDocument, bedit_document, BEDIT, DOCUMENT, GtkWidget)

BeditDocument *
bedit_document_new(void);
BeditDocument *
bedit_document_new_for_file(GFile *file);

void
bedit_document_reload_async(BeditDocument *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean
bedit_document_reload_finish(BeditDocument *self, GAsyncResult *result, GError **error);

void
bedit_document_save_async(BeditDocument *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean
bedit_document_save_finish(BeditDocument *self, GAsyncResult *result, GError **error);

void
bedit_document_save_as_async(BeditDocument *self, GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean
bedit_document_save_as_finish(BeditDocument *self, GAsyncResult *result, GError **error);

char const *
bedit_document_get_title(BeditDocument *self);

GFile *
bedit_document_get_file(BeditDocument *self);

gboolean
bedit_document_get_modified(BeditDocument *self);
void
bedit_document_set_modified(BeditDocument *self, gboolean modified);

gboolean
bedit_document_get_loading(BeditDocument *self);

gboolean
bedit_document_get_saving(BeditDocument *self);

G_END_DECLS
