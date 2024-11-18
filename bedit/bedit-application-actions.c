#include <config.h>

#include "bedit-application-actions.h"

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "bedit-application.h"

/* --- File ----------------------------------------------------------------------------------------------- */

static void
bedit_application_actions_do_new(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_application_actions_do_close(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

/* --- Edit ----------------------------------------------------------------------------------------------- */

static void
bedit_application_actions_do_open_preferences(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

/* --- View ----------------------------------------------------------------------------------------------- */

static void
bedit_application_actions_do_increase_text_size(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_application_actions_do_decrease_text_size(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_application_actions_do_reset_text_size(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

/* --- Tools ---------------------------------------------------------------------------------------------- */

static void
bedit_application_actions_do_open_check_spelling(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_application_actions_do_open_set_language(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

/* --- Help ----------------------------------------------------------------------------------------------- */

static void
bedit_application_actions_do_open_help(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_application_actions_do_open_keyboard_shortcuts(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

static void
bedit_application_actions_do_open_about(GSimpleAction *action, GVariant *param, gpointer user_data) {
    BeditApplication *self = BEDIT_APPLICATION(user_data);

    g_return_if_fail(G_IS_SIMPLE_ACTION(action));
    g_return_if_fail(BEDIT_IS_APPLICATION(self));
    g_return_if_fail(param == NULL);
}

/* === Lifecycle ========================================================================================== */

void
bedit_application_actions_init_class(BeditApplicationClass *class) {}

void
bedit_application_actions_init_instance(BeditApplication *self) {
    GAction *action;
    GSettings *settings;

    settings = g_settings_new("com.bwhmather.Bedit2");

    // File.
    action = G_ACTION(g_simple_action_new("new", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_new), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("close", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_close), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    // Edit.
    action = G_ACTION(g_simple_action_new("preferences", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_open_preferences), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    // View.
    action = g_settings_create_action(settings, "show-toolbar");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = g_settings_create_action(settings, "show-menubar");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = g_settings_create_action(settings, "show-statusbar");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = g_settings_create_action(settings, "show-overview-map");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = g_settings_create_action(settings, "word-wrap");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("increase-text-size", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_increase_text_size), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("decrease-text-size", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_decrease_text_size), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("reset-text-size", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_reset_text_size), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = g_settings_create_action(settings, "highlight-mode");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    // Tools.
    action = G_ACTION(g_simple_action_new("open-check-spelling", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_open_check_spelling), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = g_settings_create_action(settings, "auto-check-spelling");
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("open-set-language", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_open_set_language), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("open-document-statistics", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_close), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    // Help.

    action = G_ACTION(g_simple_action_new("help", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_open_help), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("keyboard-shortcuts", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_open_keyboard_shortcuts), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    action = G_ACTION(g_simple_action_new("about", NULL));
    g_signal_connect(action, "activate", G_CALLBACK(bedit_application_actions_do_open_about), self);
    g_action_map_add_action(G_ACTION_MAP(self), action);

    g_clear_object(&settings);
}
