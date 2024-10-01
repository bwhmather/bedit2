#include <config.h>

#include "bedit-window.h"

#include <bricks.h>

struct _BeditWindow {
    GtkApplicationWindow parent_instance;

    BrkTabView *tab_view;
};

G_DEFINE_TYPE(BeditWindow, bedit_window, GTK_TYPE_APPLICATION_WINDOW)

/* === Actions ============================================================================================ */

static void
bedit_window_do_open(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_close(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_toggle_fullscreen(GSimpleAction *action, GVariant *state, gpointer user_data) {}

static void
bedit_window_do_find(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_find_next(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_find_previous(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_replace(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_replace_all(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static void
bedit_window_do_go_to_line(GSimpleAction *action, GVariant *parameter, gpointer user_data) {}

static GActionEntry win_entries[] = {
    {"open", bedit_window_do_open},
    {"close", bedit_window_do_close},
    {"fullscreen", NULL, NULL, "false", bedit_window_do_toggle_fullscreen},
    {"find", bedit_window_do_find},
    {"find-next", bedit_window_do_find_next},
    {"find-previous", bedit_window_do_find_previous},
    {"replace", bedit_window_do_replace},
    {"replace-all", bedit_window_do_replace_all},
    {"go-to-line", bedit_window_do_go_to_line},
};

/* === Lifecycle ========================================================================================== */

static void
bedit_window_constructed(GObject *gobject) {
    BeditWindow *self = BEDIT_WINDOW(gobject);

    g_assert(BEDIT_IS_WINDOW(self));

    G_OBJECT_CLASS(bedit_window_parent_class)->constructed(gobject);
}

static void
bedit_window_class_init(BeditWindowClass *class) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    g_type_ensure(BRK_TYPE_TAB_BAR);
    g_type_ensure(BRK_TYPE_TAB_VIEW);
    g_type_ensure(BRK_TYPE_TOOLBAR_VIEW);
    g_type_ensure(BRK_TYPE_TOOLBAR);

    gobject_class->constructed = bedit_window_constructed;

    gtk_widget_class_set_template_from_resource(
        widget_class, "/com/bwhmather/Bedit/ui/bedit-window.ui"
    );
    gtk_widget_class_bind_template_child(widget_class, BeditWindow, tab_view);
}

static void
bedit_window_init(BeditWindow *self) {
    g_action_map_add_action_entries(
        G_ACTION_MAP(self), win_entries, G_N_ELEMENTS(win_entries), self
    );

    gtk_widget_init_template(GTK_WIDGET(self));
}

/* === Public API ========================================================================================= */

BeditWindow *
bedit_window_new(GtkApplication *application) {
    return g_object_new(
        BEDIT_TYPE_WINDOW,
        "application", application,
        "show-menubar", TRUE,
        NULL
    );
}
