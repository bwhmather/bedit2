#include <config.h>

#include "bedit-window-actions.h"

#include <bricks.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "bedit-window.h"

/* --- File ----------------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_open(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_save(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_save_as(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_revert(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_print_preview(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_print(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_close_tab(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_close_window(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Edit ----------------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_undo(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_redo(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_cut(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_copy(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_paste(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_delete(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_duplicate(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_select_all(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_comment(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_uncomment(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_insert_date_time(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_sort_lines(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_join_lines(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

/* --- Search --------------------------------------------------------------------------------------------- */

static void
bedit_window_actions_do_find(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_find_next(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_find_previous(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_replace(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_replace_all(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

    g_return_if_fail(BEDIT_IS_WINDOW(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_window_actions_do_go_to_line(GtkWidget *widget, char const *action_name, GVariant *param) {
    BeditWindow *self = BEDIT_WINDOW(widget);

    (void)action_name;

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
