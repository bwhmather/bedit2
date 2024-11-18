#include <config.h>

#include "bedit-application-shortcuts.h"

#include <glib-object.h>
#include <gtk/gtk.h>

#include "bedit-application.h"

void
bedit_application_shortcuts_init_instance(BeditApplication *self) {
    // File.
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "app.new", (char const *[]){"<Control>N", NULL});
    gtk_application_set_accels_for_action(GTK_APPLICATION(self), "win.new", (char const *[]){"<Control>T", NULL});
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
