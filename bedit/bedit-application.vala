class Bedit.Application : Gtk.Application {
    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit2");

    private void
    action_new() {
        var window = new Bedit.Window(this);
        window.present();
    }

    private void
    action_close() {
    }

    private void
    action_preferences() {
    }

    private void
    action_increase_text_size() {
    }

    private void
    action_decrease_text_size() {
    }

    private void
    action_reset_text_size() {
    }

    private void
    action_open_check_spelling() {
    }

    private void
    action_open_set_language() {
    }

    private void
    action_open_document_statistics() {
    }

    private void
    action_help() {
    }

    private void
    action_keyboard_shortcuts() {
    }

    private void
    action_about() {
    }

    const GLib.ActionEntry[] action_entries = {
        {"new", action_new},
        {"close", action_close},
        {"preferences", action_preferences},
        {"increase-text-size", action_increase_text_size},
        {"decrease-text-size", action_decrease_text_size},
        {"reset-text-size", action_reset_text_size},
        {"open-check-spelling", action_open_check_spelling},
        {"open-set-language", action_open_set_language},
        {"open-document-statistics", action_open_document_statistics},
        {"help", action_help},
        {"keyboard-shortcuts", action_keyboard_shortcuts},
        {"about", action_about},
    };

    public override void
    startup() {
        base.startup();
        Brk.init();
        GtkSource.init();
    }

    public override int
    handle_local_options(GLib.VariantDict options) {
        bool print_version = false;
        options.lookup("version", "b", print_version);
        if (print_version) {
            warning("TODO");
            return 0;
        }

        bool standalone = false;
        options.lookup("standalone", "b", standalone);
        if (standalone) {
            this.flags |= NON_UNIQUE;
            options.remove("standalone");
        }

        return -1;
    }

    public override int
    command_line(GLib.ApplicationCommandLine cmdline) {
        var window = new Bedit.Window(this);
        window.present();

        return 0;
    }

    public override void
    activate() {
        var window = new Bedit.Window(this);
        window.present();
    }

    class construct {
        typeof (Bedit.Window).ensure();
    }

    construct {
        this.flags |= HANDLES_COMMAND_LINE;

        this.add_main_option("version", 0, 0, NONE, "Print the application's version", null);
        this.add_main_option("standalone", 0, 0, NONE, "Always start a new instance", null);
        this.add_main_option(GLib.OPTION_REMAINING, 0, 0, STRING_ARRAY, "Files to open", "FILES");

        this.add_action_entries(action_entries, this);

        this.add_action(settings.create_action("show-toolbar"));
        this.add_action(settings.create_action("show-menubar"));
        this.add_action(settings.create_action("show-statusbar"));
        this.add_action(settings.create_action("show-overview-map"));
        this.add_action(settings.create_action("highlight-selection"));
        this.add_action(settings.create_action("highlight-current-line"));
        this.add_action(settings.create_action("show-line-numbers"));
        this.add_action(settings.create_action("word-wrap"));
        this.add_action(settings.create_action("auto-check-spelling"));

        // File.
        this.set_accels_for_action("app.new", {"<Control>N"});
        this.set_accels_for_action("win.new", {"<Control>T"});
        this.set_accels_for_action("win.open", {"<Control>O"});
        this.set_accels_for_action("doc.save", {"<Control>S"});
        this.set_accels_for_action("doc.save-as", {"<Control><Shift>S"});
        this.set_accels_for_action("doc.revert", {});
        this.set_accels_for_action("doc.print-preview", {"<Control><Shift>P"});
        this.set_accels_for_action("doc.print", {"<Control>P"});
        this.set_accels_for_action("doc.close", {"<Control>W"});
        this.set_accels_for_action("win.close", {});
        this.set_accels_for_action("app.quit", {});

        // Edit.
        this.set_accels_for_action("doc.undo", {"<Control>Z"});
        this.set_accels_for_action("doc.redo", {"<Control><Shift>Z", "<Control>Y"});
        this.set_accels_for_action("clipboard.cut", {"<Control>X"});
        this.set_accels_for_action("clipboard.copy", {"<Control>C"});
        this.set_accels_for_action("clipboard.paste", {"<Control>P"});
        this.set_accels_for_action("doc.delete", {"<Control>D"});
        this.set_accels_for_action("doc.duplicate", {"<Control><Shift>D"});
        this.set_accels_for_action("selection.select-all", {"<Control>A"});
        this.set_accels_for_action("doc.comment", {"<Control>slash"});
        this.set_accels_for_action("doc.uncomment", {"<Control><Shift>slash"});
        this.set_accels_for_action("doc.insert-date-and-time", {"F5"});
        this.set_accels_for_action("doc.sort-lines", {"F9"});
        this.set_accels_for_action("doc.join-lines", {"<Control>J"});
        this.set_accels_for_action("app.preferences", {});

        // View.
        this.set_accels_for_action("app.show-toolbar", {});
        this.set_accels_for_action("app.show-menubar", {"F10"});
        this.set_accels_for_action("app.show-statusbar", {});
        this.set_accels_for_action("app.show-overview-maps", {});
        this.set_accels_for_action("win.fullscreen", {"F11", "<Control>F"});
        this.set_accels_for_action("app.word-wrap", {});
        this.set_accels_for_action("app.increase-text-size", {"<Control>plus"});
        this.set_accels_for_action("app.decrease-text-size", {"<Control>minus"});
        this.set_accels_for_action("app.reset-text-size", {"<Control>0"});
        this.set_accels_for_action("app.highlight-selection", {});
        this.set_accels_for_action("app.highlight-current-line", {});
        this.set_accels_for_action("app.show-line-numbers", {});

        // Search.
        this.set_accels_for_action("search.find", {"<Control>F"});
        this.set_accels_for_action("search.find-next", {"<Control>G"});
        this.set_accels_for_action("search.find-previous", {"<Control><Shift>G"});
        this.set_accels_for_action("search.find-and-replace", {"<Control>H"});
        this.set_accels_for_action("search.replace", {"<Control>R"});
        this.set_accels_for_action("search.replace-all", {"<Control><Alt>R"});
        this.set_accels_for_action("doc.show-go-to-line", {"<Control>I"});

        // Tools.
        this.set_accels_for_action("app.open-check-spelling", {"<Shift>F7"});
        this.set_accels_for_action("app.auto-check-spelling", {});
        this.set_accels_for_action("app.open-set-language", {});
        this.set_accels_for_action("app.open-document-statistics", {});

        // Help.
        this.set_accels_for_action("app.help", {"F1"});
        this.set_accels_for_action("app.keyboard-shortcuts", {});
        this.set_accels_for_action("app.about", {});
    }

    public Application() {
        Object(
            application_id: "com.bwhmather.Bedit",
            resource_base_path: "/com/bwhmather/Bedit"
        );
    }

    public static int
    main(string[] args) {
        var app = new Bedit.Application();
        return app.run(args);
    }
}
