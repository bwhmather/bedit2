#include <config.h>

#include "bedit-document.h"

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

struct _BeditDocument {
    GtkWidget parent_instance;

    GtkSourceView *source_view;
    GtkSourceBuffer *source_buffer;

    gchar *title;
    GFile *file;

    gboolean modified : 1;
    gboolean loading : 1;
    gboolean saving : 1;
};

G_DEFINE_TYPE(BeditDocument, bedit_document, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    PROP_TITLE,
    PROP_FILE,
    PROP_MODIFIED,
    PROP_LOADING,
    PROP_SAVING,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* --- Loading -------------------------------------------------------------------------------------------- */

static void
bedit_document_on_source_file_loader_load_progress(goffset current_num_bytes, goffset total_num_bytes, gpointer user_data);

static void
bedit_document_on_file_query_info_notify(GObject *object, GAsyncResult *result, gpointer user_data);

static void
bedit_document_on_source_file_loader_load_notify(GObject *object, GAsyncResult *result, gpointer user_data);

static void
bedit_document_do_load(BeditDocument *self, GTask *task) {
    GtkSourceFile *source_file;
    GtkSourceFileLoader *source_loader;

    g_assert(BEDIT_IS_DOCUMENT(self));
    g_assert(G_IS_TASK(task));
    g_return_if_fail(self->loading == FALSE);
    g_return_if_fail(self->saving == FALSE);
    g_return_if_fail(G_IS_FILE(self->file));

    source_file = gtk_source_file_new();
    gtk_source_file_set_location(source_file, self->file);

    source_loader = gtk_source_file_loader_new(self->source_buffer, source_file);

    gtk_source_file_loader_load_async(
        source_loader,
        G_PRIORITY_LOW,
        g_task_get_cancellable(task),
        bedit_document_on_source_file_loader_load_progress,
        self,
        NULL,
        bedit_document_on_source_file_loader_load_notify,
        g_object_ref(task)
    );

    g_object_unref(source_loader);
    g_object_unref(source_file);

    self->loading = TRUE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_LOADING]);
}

static void
bedit_document_on_source_file_loader_load_progress(goffset current_num_bytes, goffset total_num_bytes, gpointer user_data) {
    (void) current_num_bytes;
    (void) total_num_bytes;
    (void) user_data;

    // TODO
}

static void
bedit_document_on_source_file_loader_load_notify(GObject *object, GAsyncResult *result, gpointer user_data) {
    GtkSourceFileLoader *source_loader = GTK_SOURCE_FILE_LOADER(object);
    GTask *task = user_data;
    GError *error = NULL;
    BeditDocument *self;
    gboolean success;
    GFile *file;

    g_assert(GTK_SOURCE_IS_FILE_LOADER(source_loader));
    g_assert(G_IS_ASYNC_RESULT(result));
    g_assert(G_IS_TASK(task));
    g_assert(self->loading == TRUE);
    g_assert(self->saving == FALSE);

    self = g_task_get_source_object(task);
    g_assert(BEDIT_IS_DOCUMENT(self));

    success = gtk_source_file_loader_load_finish(source_loader, result, &error);
    if (!success) {
        g_warning("Failed to load file: %s", error->message);
        g_task_return_error(task, g_steal_pointer(&error));
        return;
    }

    file = bedit_document_get_file(self);

    g_file_query_info_async(
        file,
        G_FILE_ATTRIBUTE_STANDARD_SIZE "," G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," G_FILE_ATTRIBUTE_FILESYSTEM_READONLY,
        G_FILE_QUERY_INFO_NONE,
        G_PRIORITY_DEFAULT,
        g_task_get_cancellable(task),
        bedit_document_on_file_query_info_notify,
        g_object_ref(task)
    );

    g_object_unref(task);
}

static void
bedit_document_on_file_query_info_notify(GObject *object, GAsyncResult *result, gpointer user_data) {
    GFile *file = G_FILE(object);
    GTask *task = G_TASK(user_data);
    BeditDocument *self;
    GFileInfo *info;
    GError *error = NULL;
    goffset size;
    gchar const *content_type;
    gboolean readonly;

    g_assert(G_IS_FILE(file));
    g_assert(G_IS_ASYNC_RESULT(result));
    g_assert(G_IS_TASK(task));
    g_assert(self->loading == TRUE);
    g_assert(self->saving == FALSE);

    self = g_task_get_source_object(task);
    g_assert(BEDIT_IS_DOCUMENT(self));

    info = g_file_query_info_finish(file, result, &error);
    if (info == NULL) {
        g_warning("Failed to read file attributes: %s", error->message);
        g_task_return_error(task, g_steal_pointer(&error));
        return;
    }

    size = g_file_info_get_size(info);
    content_type = g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
    readonly = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);

    (void) size; // TODO
    (void) content_type; // TODO
    (void) readonly; // TODO

    self->loading = FALSE;
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_LOADING]);
}

/* === Lifecycle ========================================================================================== */

static void
bedit_document_constructed(GObject *gobject) {
    BeditDocument *self = BEDIT_DOCUMENT(gobject);

    g_assert(BEDIT_IS_DOCUMENT(self));

    G_OBJECT_CLASS(bedit_document_parent_class)->constructed(gobject);
}

static void
bedit_document_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    BeditDocument *self = BEDIT_DOCUMENT(object);

    switch (prop_id) {
    case PROP_TITLE:
        g_value_set_string(value, bedit_document_get_title(self));
        break;

    case PROP_FILE:
        g_value_set_object(value, bedit_document_get_file(self));
        break;

    case PROP_MODIFIED:
        g_value_set_boolean(value, bedit_document_get_modified(self));
        break;

    case PROP_LOADING:
        g_value_set_boolean(value, bedit_document_get_loading(self));
        break;

    case PROP_SAVING:
        g_value_set_boolean(value, bedit_document_get_saving(self));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
bedit_document_set_property(GObject *object, guint prop_id, GValue const *value, GParamSpec *pspec) {
    BeditDocument *self = BEDIT_DOCUMENT(object);

    switch (prop_id) {

    case PROP_FILE:
        g_set_object(&self->file, g_value_get_object(value));
        break;

    case PROP_MODIFIED:
        bedit_document_set_modified(self, g_value_get_boolean(value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
bedit_document_class_init(BeditDocumentClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    g_type_ensure(GTK_SOURCE_TYPE_VIEW);

    object_class->constructed = bedit_document_constructed;
    object_class->get_property = bedit_document_get_property;
    object_class->set_property = bedit_document_set_property;

    properties[PROP_TITLE] = g_param_spec_string(
        "title", "Title", "Human readable string that identifies the document",
        NULL,
        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
    );

    properties[PROP_FILE] = g_param_spec_object(
        "file", "File", "The document's underlying file (if any)",
        G_TYPE_FILE,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
    );

    properties[PROP_MODIFIED] = g_param_spec_boolean(
        "modified", "Modified", "Whether the document contains changes that have not been saved",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
    );

    properties[PROP_LOADING] = g_param_spec_boolean(
        "loading", "Loading", "Whether the document is currently loading from disk",
        FALSE,
        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
    );

    properties[PROP_SAVING] = g_param_spec_boolean(
        "saving", "Saving", "Whether the document is currently in the process of being saved to disk",
        FALSE,
        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
    );

    g_object_class_install_properties(object_class, N_PROPS, properties);

    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/com/bwhmather/Bedit/ui/bedit-document.ui"
    );
    gtk_widget_class_bind_template_child(widget_class, BeditDocument, source_view);
}

static void
bedit_document_init(BeditDocument *self) {
    GtkSourceBuffer *source_buffer;

    gtk_widget_init_template(GTK_WIDGET(self));

    source_buffer = GTK_SOURCE_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->source_view)));
    g_assert(GTK_SOURCE_IS_BUFFER(source_buffer));
    self->source_buffer = g_object_ref(source_buffer);
}

/* === Public API ========================================================================================= */

BeditDocument *
bedit_document_new(void) {
    return g_object_new(BEDIT_TYPE_DOCUMENT, NULL);
}

BeditDocument *
bedit_document_new_for_file(GFile *file) {
    return g_object_new(
        BEDIT_TYPE_DOCUMENT,
        "file", file,
        NULL
    );
}

void
bedit_document_reload_async(BeditDocument *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *task;

    g_return_if_fail(BEDIT_IS_DOCUMENT(self));
    g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));

    task = g_task_new(self, cancellable, callback, user_data);
    g_task_set_source_tag(task, bedit_document_reload_async);

    bedit_document_do_load(self, task);
}

gboolean
bedit_document_reload_finish(BeditDocument *self, GAsyncResult *result, GError **error) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), FALSE);
    g_return_val_if_fail(G_IS_TASK(result), FALSE);
    g_return_val_if_fail(g_task_get_source_tag(G_TASK(result)) == bedit_document_reload_async, FALSE);

    return g_task_propagate_boolean(G_TASK(result), error);
}

void
bedit_document_save_async(BeditDocument *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
    GTask *task;

    g_return_if_fail(BEDIT_IS_DOCUMENT(self));
    g_return_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable));

    task = g_task_new(self, cancellable, callback, user_data);
    g_task_set_source_tag(task, bedit_document_save_async);
}

gboolean
bedit_document_save_finish(BeditDocument *self, GAsyncResult *result, GError **error) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), FALSE);
    g_return_val_if_fail(G_IS_TASK(result), FALSE);
    g_return_val_if_fail(g_task_get_source_tag(G_TASK(result)) == bedit_document_save_async, FALSE);

    return g_task_propagate_boolean(G_TASK(result), error);
}

void
bedit_document_save_as_async(BeditDocument *self, GFile *file, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);

gboolean
bedit_document_save_as_finish(BeditDocument *self, GAsyncResult *result, GError **error);

gchar const *
bedit_document_get_title(BeditDocument *self) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), NULL);
    return self->title;
}

GFile *
bedit_document_get_file(BeditDocument *self) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), NULL);
    return self->file;
}

gboolean
bedit_document_get_modified(BeditDocument *self) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), FALSE);
    return self->modified;
}

void
bedit_document_set_modified(BeditDocument *self, gboolean modified) {
    g_return_if_fail(BEDIT_IS_DOCUMENT(self));

    if (self->modified == modified) {
        return;
    }
    self->modified = modified;

    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MODIFIED]);
}

gboolean
bedit_document_get_loading(BeditDocument *self) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), FALSE);
    return self->loading;
}

gboolean
bedit_document_get_saving(BeditDocument *self) {
    g_return_val_if_fail(BEDIT_IS_DOCUMENT(self), FALSE);
    return self->saving;
}
