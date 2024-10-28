#include <config.h>

#include "bedit-document.h"

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

struct _BeditDocument {
    GtkWidget parent_instance;

    GtkSourceView *source_view;
};

G_DEFINE_TYPE(BeditDocument, bedit_document, GTK_TYPE_WIDGET)

/* === Lifecycle ========================================================================================== */

static void
bedit_document_constructed(GObject *gobject) {
    BeditDocument *self = BEDIT_DOCUMENT(gobject);

    g_assert(BEDIT_IS_DOCUMENT(self));

    G_OBJECT_CLASS(bedit_document_parent_class)->constructed(gobject);
}

static void
bedit_document_class_init(BeditDocumentClass *class) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    g_type_ensure(GTK_SOURCE_TYPE_VIEW);

    gobject_class->constructed = bedit_document_constructed;

    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/com/bwhmather/Bedit/ui/bedit-document.ui"
    );
    gtk_widget_class_bind_template_child(widget_class, BeditDocument, source_view);
}

static void
bedit_document_init(BeditDocument *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

/* === Public API ========================================================================================= */

BeditDocument *
bedit_document_new(GtkApplication *application) {
    return g_object_new(BEDIT_TYPE_DOCUMENT, NULL);
}
