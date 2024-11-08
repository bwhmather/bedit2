#include <config.h>

#include "bedit-searchbar.h"

#include <bricks.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

struct _BeditSearchbar {
    GtkWidget parent_instance;
};

G_DEFINE_TYPE(BeditSearchbar, bedit_searchbar, GTK_TYPE_WIDGET)

/* === Lifecycle ========================================================================================== */

static void
bedit_searchbar_constructed(GObject *gobject) {
    BeditSearchbar *self = BEDIT_SEARCHBAR(gobject);

    g_assert(BEDIT_IS_SEARCHBAR(self));

    G_OBJECT_CLASS(bedit_searchbar_parent_class)->constructed(gobject);
}

static void
bedit_searchbar_class_init(BeditSearchbarClass *class) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    g_type_ensure(BRK_TYPE_BUTTON_GROUP);

    gobject_class->constructed = bedit_searchbar_constructed;

    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/com/bwhmather/Bedit/ui/bedit-searchbar.ui"
    );
}

static void
bedit_searchbar_init(BeditSearchbar *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

/* === Public API ========================================================================================= */

BeditSearchbar *
bedit_searchbar_new(GtkApplication *application) {
    return g_object_new(BEDIT_TYPE_SEARCHBAR, NULL);
}
