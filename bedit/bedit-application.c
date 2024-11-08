#include <config.h>

#include "bedit-application.h"

#include <bricks.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "bedit-application-actions.h"
#include "bedit-window.h"

struct _BeditApplication {
    GtkApplication parent_instance;
};

G_DEFINE_TYPE(BeditApplication, bedit_application, GTK_TYPE_APPLICATION)

/* === Lifecycle ========================================================================================== */

static void
bedit_application_startup(GApplication *application) {
    G_APPLICATION_CLASS(bedit_application_parent_class)->startup(application);

    brk_init();
}

static int
bedit_application_handle_local_options(GApplication *application, GVariantDict *options) {
    BeditApplication *self = BEDIT_APPLICATION(application);
    gboolean print_version = FALSE;
    gboolean standalone = FALSE;
    GApplicationFlags flags;

    g_assert(BEDIT_IS_APPLICATION(self));

    g_variant_dict_lookup(options, "version", "b", &print_version);
    if (print_version) {
        g_fprintf(stdout, "%s - Version %s", BEDIT_NAME, BEDIT_VERSION);
    }

    g_variant_dict_lookup(options, "standalone", "b", &standalone);
    if (standalone) {
        flags = g_application_get_flags(G_APPLICATION(self));
        flags |= G_APPLICATION_NON_UNIQUE;
        g_application_set_flags(G_APPLICATION(self), flags);
        g_variant_dict_remove(options, "standalone");
    }

    return -1;
}

static int
bedit_application_command_line(GApplication *application, GApplicationCommandLine *cmdline) {
    BeditApplication *self = BEDIT_APPLICATION(application);
    BeditWindow *window;

    g_assert(BEDIT_IS_APPLICATION(self));

    window = bedit_window_new(GTK_APPLICATION(self));
    g_object_ref(window);
    gtk_window_present(GTK_WINDOW(window));

    return 0;
}

static void
bedit_application_activate(GApplication *application) {
    BeditApplication *self = BEDIT_APPLICATION(application);
    BeditWindow *window;

    g_assert(BEDIT_IS_APPLICATION(self));

    // TODO
    window = bedit_window_new(GTK_APPLICATION(self));
    g_object_ref(window);
    gtk_window_present(GTK_WINDOW(window));
}

static void
bedit_application_constructed(GObject *gobject) {
    BeditApplication *self = BEDIT_APPLICATION(gobject);
    GApplicationFlags flags;

    g_assert(BEDIT_IS_APPLICATION(self));

    G_OBJECT_CLASS(bedit_application_parent_class)->constructed(gobject);

    flags = g_application_get_flags(G_APPLICATION(self));
    flags |= G_APPLICATION_HANDLES_COMMAND_LINE;
    g_application_set_flags(G_APPLICATION(self), flags);

    g_application_add_main_option(
        G_APPLICATION(self), "version", 0, 0, G_OPTION_ARG_NONE, "Print the application's version", NULL
    );
    g_application_add_main_option(
        G_APPLICATION(self), "standalone", 0, 0, G_OPTION_ARG_NONE, "Always create a new instance", NULL
    );
    g_application_add_main_option(
        G_APPLICATION(self), G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, "Files to open", "FILES"
    );
}

static void
bedit_application_dispose(GObject *gobject) {
    BeditApplication *self = BEDIT_APPLICATION(gobject);

    g_assert(BEDIT_IS_APPLICATION(self));

    G_OBJECT_CLASS(bedit_application_parent_class)->dispose(gobject);
}

static void
bedit_application_finalize(GObject *gobject) {
    BeditApplication *self = BEDIT_APPLICATION(gobject);

    g_assert(BEDIT_IS_APPLICATION(self));

    G_OBJECT_CLASS(bedit_application_parent_class)->finalize(gobject);
}

static void
bedit_application_class_init(BeditApplicationClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    GApplicationClass *application_class = G_APPLICATION_CLASS(class);

    g_type_ensure(BEDIT_TYPE_WINDOW);

    object_class->constructed = bedit_application_constructed;
    object_class->dispose = bedit_application_dispose;
    object_class->finalize = bedit_application_finalize;

    application_class->startup = bedit_application_startup;
    application_class->handle_local_options = bedit_application_handle_local_options;
    application_class->command_line = bedit_application_command_line;
    application_class->activate = bedit_application_activate;

    bedit_application_actions_init_class(class);
}

static void
bedit_application_init(BeditApplication *self) {
    bedit_application_actions_init_instance(self);

    // File.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.new", (char const *[]){"<Control>N", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.open", (char const *[]){"<Control>O", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.save", (char const *[]){"<Control>S", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.save-as", (char const *[]){"<Control><Shift>S", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.revert", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.print-preview", (char const *[]){"<Control><Shift>P", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.print", (char const *[]){"<Control>P", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.close", (char const *[]){"<Control>W", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.close", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.quit", (char const *[]){NULL});

    // Edit.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.undo", (char const *[]){"<Control>Z", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.redo", (char const *[]){"<Control><Shift>Z", "<Control>Y", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.cut", (char const *[]){"<Control>X", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.copy", (char const *[]){"<Control>C", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.paste", (char const *[]){"<Control>P", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.delete", (char const *[]){"<Control>D", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.duplicate", (char const *[]){"<Control><Shift>D", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.select-all", (char const *[]){"<Control>A", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.comment", (char const *[]){"<Control>slash", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.uncomment", (char const *[]){"<Control><Shift>slash", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.insert-date-and-time", (char const *[]){"F5", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.sort-lines", (char const *[]){"F9", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "doc.join-lines", (char const *[]){"<Control>J", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.preferences", (char const *[]){NULL});

    // View.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.show-toolbar", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.show-menubar", (char const *[]){"F10", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.show-statusbar", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.show-overview-maps", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.fullscreen", (char const *[]){"F11", "<Control>F", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.word-wrap", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.increase-text-size", (char const *[]){"<Control>plus", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.decrease-text-size", (char const *[]){"<Control>minus", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.reset-text-size", (char const *[]){"<Control>0", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.highlight-mode", (char const *[]){NULL});

    // Search.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.find", (char const *[]){"<Control>F", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.find-next", (char const *[]){"<Control>G", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.find-previous", (char const *[]){"<Control><Shift>G", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.replace", (char const *[]){"<Control>R", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.replace-all", (char const *[]){"<Control><Alt>R", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.go-to-line", (char const *[]){"<Control>I", NULL});

    // Tools.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.open-check-spelling", (char const *[]){"<Shift>F7", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.auto-check-spelling", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.open-set-language", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.open-document-statistics", (char const *[]){NULL});

    // Help.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.help", (char const *[]){"F1", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.keyboard-shortcuts", (char const *[]){NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.about", (char const *[]){NULL});
}

/* === Public API ========================================================================================= */

BeditApplication *
bedit_application_new(void) {
    return g_object_new(
        BEDIT_TYPE_APPLICATION,
        "application-id", "com.bwhmather.Bedit",
        "resource-base-path", "/com/bwhmather/Bedit",
        NULL
    );
}
