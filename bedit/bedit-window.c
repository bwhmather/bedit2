#include <config.h>

#include <bricks.h>

#include "bedit-window.h"

struct _BeditWindow {
    GtkApplicationWindow parent_instance;

    BrkTabView *tab_view;
};

G_DEFINE_TYPE(BeditWindow, bedit_window, GTK_TYPE_APPLICATION_WINDOW)

/* === Lifecycle ========================================================================================== */

static void
bedit_window_constructed(GObject *gobject) {
    BeditWindow *self = BEDIT_WINDOW(gobject);
    GtkWidget *label;

    g_assert(BEDIT_IS_WINDOW(self));

    G_OBJECT_CLASS(bedit_window_parent_class)->constructed(gobject);

    label = gtk_label_new("First Tab");
    brk_tab_view_append(self->tab_view, label);

    label = gtk_label_new("Second Tab");
    brk_tab_view_append(self->tab_view, label);
}

static void
bedit_window_class_init(BeditWindowClass *class) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    g_type_ensure(BRK_TYPE_TAB_BAR);
    g_type_ensure(BRK_TYPE_TAB_VIEW);

    gobject_class->constructed = bedit_window_constructed;

    gtk_widget_class_set_template_from_resource(widget_class, "/com/bwhmather/Bedit/ui/bedit-window.ui");
    gtk_widget_class_bind_template_child(widget_class, BeditWindow, tab_view);
}

static void
bedit_window_init(BeditWindow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

/* === Public API ========================================================================================= */

BeditWindow *
bedit_window_new(GtkApplication *application) {
    return g_object_new(
        BEDIT_TYPE_WINDOW,
        "application", application,
        NULL
    );
}
