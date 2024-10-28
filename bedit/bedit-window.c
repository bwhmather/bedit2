#include <config.h>

#include "bedit-window.h"

#include <bricks.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "bedit-document.h"
#include "bedit-window-actions.h"

struct _BeditWindow {
    GtkApplicationWindow parent_instance;

    BrkTabView *tab_view;
};

G_DEFINE_TYPE(BeditWindow, bedit_window, GTK_TYPE_APPLICATION_WINDOW)

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

    g_type_ensure(BEDIT_TYPE_DOCUMENT);

    gobject_class->constructed = bedit_window_constructed;

    bedit_window_actions_init_class(class);

    gtk_widget_class_set_template_from_resource(widget_class, "/com/bwhmather/Bedit/ui/bedit-window.ui");
    gtk_widget_class_bind_template_child(widget_class, BeditWindow, tab_view);
}

static void
bedit_window_init(BeditWindow *self) {
    bedit_window_actions_init_instance(self);
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
