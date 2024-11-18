#include <config.h>

#include "bedit-window-actions.h"

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "bedit-document.h"
#include "bedit-window.h"

/* === Helpers ============================================================================================ */

static void
bedit_window_actions_cancellable_handle_window_destroy(GtkWidget *widget, gpointer user_data) {
    BeditWindow *self = BEDIT_WINDOW(widget);
    GCancellable *cancellable = G_CANCELLABLE(user_data);

    g_assert(BEDIT_IS_WINDOW(self));
    g_assert(G_IS_CANCELLABLE(cancellable));

    g_cancellable_cancel(cancellable);
}

static GCancellable *
bedit_window_actions_get_cancellable(BeditWindow *self) {
    GCancellable *cancellable;

    cancellable = g_cancellable_new();
    g_signal_connect_object(
        self,
        "destroy",
        G_CALLBACK(bedit_window_actions_cancellable_handle_window_destroy),
        cancellable,
        G_CONNECT_DEFAULT
    );

    return cancellable;
}

/* === File =============================================================================================== */

/* --- Window Open ---------------------------------------------------------------------------------------- */

static void
bedit_window_actions_handle_open_dialog_result(GObject *object, GAsyncResult *result, gpointer user_data);

static void
bedit_window_actions_handle_open_dialog_result(GObject *object, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *file_dialog = GTK_FILE_DIALOG(object);
    BeditWindow *self = BEDIT_WINDOW(user_data);
    GFile *file;
    BeditDocument *document;
    GError *error = NULL;

    file = gtk_file_dialog_open_finish(file_dialog, result, &error);
    g_return_if_fail(G_IS_FILE(file)); // TODO

    document = bedit_document_new_for_file(file);
    bedit_window_add_document(self, document);

    g_clear_object(&document);
    g_clear_object(&file);
}

static void
bedit_window_actions_do_open(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);
    GCancellable *cancellable;
    GtkFileDialog *file_dialog;

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);

    cancellable = bedit_window_actions_get_cancellable(self);

    file_dialog = gtk_file_dialog_new();

    gtk_file_dialog_open(
        file_dialog,
        GTK_WINDOW(self),
        cancellable,
        bedit_window_actions_handle_open_dialog_result,
        self
    );

    g_clear_object(&file_dialog);
    g_clear_object(&cancellable);
}

/* --- Document Save -------------------------------------------------------------------------------------- */

static void
bedit_window_actions_save_on_document_save_result(GObject *object, GAsyncResult *result, gpointer user_data);

static void
bedit_window_actions_save_begin(BeditWindow *self) {
    BeditDocument *document;

    g_assert(BEDIT_IS_WINDOW(self));

    document = bedit_window_get_active_document(self);
    g_assert(BEDIT_IS_DOCUMENT(document));

    bedit_document_save_async(
        document,
        NULL,
        bedit_window_actions_save_on_document_save_result,
        NULL
    );
}

static void
bedit_window_actions_save_on_document_save_result(GObject *object, GAsyncResult *result, gpointer user_data) {
    BeditDocument *document = BEDIT_DOCUMENT(object);
    gboolean success;
    GError *error;

    (void) user_data;

    g_return_if_fail(BEDIT_IS_DOCUMENT(document));

    success = bedit_document_save_finish(document, result, &error);
    if (!success) {
        // TODO show an error message.
        g_warning("Saving failed: %s", error->message);
        g_clear_pointer(&error, g_error_free);
        return;
    }
}

static void
bedit_window_actions_save_as_on_dialog_result(GObject *object, GAsyncResult *result, gpointer user_data);
static void
bedit_window_actions_save_as_on_document_save_as_result(GObject *object, GAsyncResult *result, gpointer user_data);

static void
bedit_window_actions_save_as_begin(BeditWindow *self) {
    BeditDocument *document;
    GCancellable *cancellable;
    GtkFileDialog *file_dialog;

    g_return_if_fail(BEDIT_IS_WINDOW(self));

    document = bedit_window_get_active_document(self);
    g_return_if_fail(BEDIT_IS_DOCUMENT(document));

    cancellable = bedit_window_actions_get_cancellable(self);

    file_dialog = gtk_file_dialog_new();

    gtk_file_dialog_save(
        file_dialog,
        GTK_WINDOW(self),
        cancellable,
        bedit_window_actions_save_as_on_dialog_result,
        g_object_ref(document)
    );

    g_clear_object(&file_dialog);
    g_clear_object(&cancellable);
}

static void
bedit_window_actions_save_as_on_dialog_result(GObject *object, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *file_dialog = GTK_FILE_DIALOG(object);
    BeditDocument *document = BEDIT_DOCUMENT(user_data);
    GError *error = NULL;
    GFile *file;

    g_assert(GTK_IS_FILE_DIALOG(object));
    g_assert(BEDIT_IS_DOCUMENT(document));

    file = gtk_file_dialog_save_finish(file_dialog, result, &error);
    if (file == NULL) {
        // TODO show an error message.
        g_warning("Save dialog error: %s", error->message);
        g_clear_pointer(&error, g_error_free);
        g_clear_object(&document);
        return;
    }

    bedit_document_save_as_async(
        document,
        file,
        NULL,
        bedit_window_actions_save_as_on_document_save_as_result,
        NULL
    );

    g_clear_object(&document);
    g_clear_object(&file);
}

static void
bedit_window_actions_save_as_on_document_save_as_result(GObject *object, GAsyncResult *result, gpointer user_data) {
    BeditDocument *document = BEDIT_DOCUMENT(object);
    gboolean success;
    GError *error;

    (void) user_data;

    g_return_if_fail(BEDIT_IS_DOCUMENT(document));

    success = bedit_document_save_as_finish(document, result, &error);
    if (!success) {
        // TODO show an error message.
        g_warning("Saving failed: %s", error->message);
        g_clear_pointer(&error, g_error_free);
        return;
    }
}

static void
bedit_window_actions_do_save(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);
    BeditDocument *document;
    GFile *file;

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);

    document = bedit_window_get_active_document(self);
    g_return_if_fail(BEDIT_IS_DOCUMENT(document));

    file = bedit_document_get_file(document);

    if (file == NULL) {
        bedit_window_actions_save_as_begin(self);
    } else {
        bedit_window_actions_save_begin(self);
    }
}

static void
bedit_window_actions_do_save_as(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);

    bedit_window_actions_save_as_begin(self);
}

/* --- Document Revert ------------------------------------------------------------------------------------ */

static void
bedit_window_actions_do_revert(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}
/* --- Document Print Preview ----------------------------------------------------------------------------- */

static void
bedit_window_actions_do_print_preview(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Print ------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_print(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Close ------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_close_tab(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Window Close --------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_close_window(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* === Edit =============================================================================================== */

/* --- Document Undo -------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_undo(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Redo -------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_redo(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Cut --------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_cut(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Copy -------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_copy(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Paste ------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_paste(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Delete Line ------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_delete(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Duplicate Line ---------------------------------------------------------------------------- */

static void
bedit_window_actions_do_duplicate(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Select All -------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_select_all(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Comment ----------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_comment(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Uncomment --------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_uncomment(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Insert Date and Time ---------------------------------------------------------------------- */

static void
bedit_window_actions_do_insert_date_time(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Sort Lines -------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_sort_lines(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Join Lines -------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_join_lines(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* === Search ============================================================================================= */

/* --- Window Find ---------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_find(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Window Find Next ----------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_find_next(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Window Find Previous ------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_find_previous(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Window Replace ------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_replace(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}
/* --- Window Replace All --------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_replace_all(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Document Go To Line -------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_go_to_line(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void) action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* === Lifecycle ========================================================================================== */

void
bedit_window_actions_init_class(BeditWindowClass *class) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    // File.
    gtk_widget_class_install_action(widget_class, "win.open", NULL, bedit_window_actions_do_open);
    gtk_widget_class_install_action(widget_class, "doc.save", NULL, bedit_window_actions_do_save);
    gtk_widget_class_install_action(widget_class, "doc.save-as", NULL, bedit_window_actions_do_save_as);
    gtk_widget_class_install_action(widget_class, "doc.revert", NULL, bedit_window_actions_do_revert);
    gtk_widget_class_install_action(widget_class, "doc.print-preview", NULL, bedit_window_actions_do_print_preview);
    gtk_widget_class_install_action(widget_class, "doc.print", NULL, bedit_window_actions_do_print);
    gtk_widget_class_install_action(widget_class, "doc.close", NULL, bedit_window_actions_do_close_tab);
    gtk_widget_class_install_action(widget_class, "win.close", NULL, bedit_window_actions_do_close_window);

    // Edit.
    gtk_widget_class_install_action(widget_class, "doc.undo", NULL, bedit_window_actions_do_undo);
    gtk_widget_class_install_action(widget_class, "doc.redo", NULL, bedit_window_actions_do_redo);
    gtk_widget_class_install_action(widget_class, "doc.cut", NULL, bedit_window_actions_do_cut);
    gtk_widget_class_install_action(widget_class, "doc.copy", NULL, bedit_window_actions_do_copy);
    gtk_widget_class_install_action(widget_class, "doc.paste", NULL, bedit_window_actions_do_paste);
    gtk_widget_class_install_action(widget_class, "doc.delete", NULL, bedit_window_actions_do_delete);
    gtk_widget_class_install_action(widget_class, "doc.duplicate", NULL, bedit_window_actions_do_duplicate);
    gtk_widget_class_install_action(widget_class, "doc.select-all", NULL, bedit_window_actions_do_select_all);
    gtk_widget_class_install_action(widget_class, "doc.comment", NULL, bedit_window_actions_do_comment);
    gtk_widget_class_install_action(widget_class, "doc.uncomment", NULL, bedit_window_actions_do_uncomment);
    gtk_widget_class_install_action(widget_class, "doc.insert-date-and-time", NULL, bedit_window_actions_do_insert_date_time);
    gtk_widget_class_install_action(widget_class, "doc.sort-lines", NULL, bedit_window_actions_do_sort_lines);
    gtk_widget_class_install_action(widget_class, "doc.join-lines", NULL, bedit_window_actions_do_join_lines);

    // View.
    // ...

    // Search.
    gtk_widget_class_install_action(widget_class, "win.find", NULL, bedit_window_actions_do_find);
    gtk_widget_class_install_action(widget_class, "win.find-next", NULL, bedit_window_actions_do_find_next);
    gtk_widget_class_install_action(widget_class, "win.find-previous", NULL, bedit_window_actions_do_find_previous);
    gtk_widget_class_install_action(widget_class, "win.replace", NULL, bedit_window_actions_do_replace);
    gtk_widget_class_install_action(widget_class, "win.replace-all", NULL, bedit_window_actions_do_replace_all);
    gtk_widget_class_install_action(widget_class, "win.show-go-to-line", NULL, bedit_window_actions_do_go_to_line);
}

void
bedit_window_actions_init_instance(BeditWindow *self) {}
