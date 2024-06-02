#include <config.h>

#include <gio/gio.h>
#include <glib-object.h>
#include <gtksourceview/gtksource.h>

#include "bedit-application.h"

int main(int argc, char **argv) {
    BeditApplication *app;
    int status;

    gtk_source_init();

    app = bedit_application_new();
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
