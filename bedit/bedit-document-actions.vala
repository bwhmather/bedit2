public sealed class Bedit.DocumentActions : GLib.Object, GLib.ActionGroup {

    public Bedit.Document document { get; construct; }
    public Bedit.Window window { get { return document.root as Bedit.Window; } }

    private GLib.SimpleActionGroup actions = new GLib.SimpleActionGroup();

    /* === Actions ======================================================================================== */

    /* --- Saving Documents ------------------------------------------------------------------------------- */

    private async void
    do_save_async() throws Error {
        var file = this.document.file;
        if (file == null) {
            var file_dialog = new Gtk.FileDialog();
            try {
                file = yield file_dialog.save(this.window, null);
            } catch (Gtk.DialogError.DISMISSED err) {
                return;
            }
        }

        yield this.document.save_async(file);
    }

    private void
    on_save() {
        this.do_save_async.begin((_, res) => {
            try {
                this.do_save_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    private async void
    do_save_as_async() throws Error {
        GLib.File file;
        var file_dialog = new Gtk.FileDialog();
        try {
            file = yield file_dialog.save(this.window, null);
        } catch (Gtk.DialogError.DISMISSED err) {
            return;
        }

        yield this.document.save_async(file);
    }

    private void
    on_save_as() {
        this.do_save_as_async.begin((_, res) => {
            try {
                this.do_save_as_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    private void
    on_revert() {
    }

    /* --- Printing Documents ----------------------------------------------------------------------------- */

    private void
    on_print() {
    }

    private void
    on_print_preview() {
    }

    /* --- Closing Windows and Tabs ----------------------------------------------------------------------- */

    private void
    on_close() {
        this.document.request_close_async.begin((_, res) => {
            try {
                this.document.request_close_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    /* --- Edit History ----------------------------------------------------------------------------------- */

    private void
    on_undo() {
        this.document.undo();
    }

    private void
    on_redo() {
        this.document.redo();
    }

    /* --- Clipboard -------------------------------------------------------------------------------------- */

    private void
    on_cut() {
    }

    private void
    on_copy() {
    }

    private void
    on_paste() {
    }

    /* --- Selection -------------------------------------------------------------------------------------- */

    private void
    on_select_all() {
    }

    /* --- Commenting and Uncommenting -------------------------------------------------------------------- */

    private void
    on_comment() {
    }

    private void
    on_uncomment() {
    }

    /* --- Insert Date and Time --------------------------------------------------------------------------- */

    private void
    on_insert_date_and_time() {
    }

    /* --- Line Operations -------------------------------------------------------------------------------- */

    private void
    on_sort_lines() {
    }

    private void
    on_join_lines() {
    }

    private void
    on_delete_line() {
    }

    private void
    on_duplicate_line() {
    }

    /* --- Navigate to Line ------------------------------------------------------------------------------- */

    private void
    on_show_go_to_line() {
    }

    /* === Lifecycle ====================================================================================== */

    const GLib.ActionEntry[] action_entries = {
        {"save", on_save},
        {"save-as", on_save_as},
        {"revert", on_revert},
        {"print-preview", on_print_preview},
        {"print", on_print},
        {"close", on_close},
        {"undo", on_undo},
        {"redo", on_redo},
        {"cut", on_cut},
        {"copy", on_copy},
        {"paste", on_paste},
        {"select-all", on_select_all},
        {"comment", on_comment},
        {"uncomment", on_uncomment},
        {"insert-date-and-time", on_insert_date_and_time},
        {"sort-lines", on_sort_lines},
        {"join-lines", on_join_lines},
        {"delete", on_delete_line},
        {"duplicate", on_duplicate_line},
        {"show-go-to-line", on_show_go_to_line},
    };

    private void
    update_action(string name, bool enabled) {
        var action = this.actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    update_actions() {
        bool busy = this.document.saving || this.document.loading;
        bool has_file = this.document.file != null;
        bool can_undo = this.document.can_undo;
        bool can_redo = this.document.can_redo;

        update_action("save", !busy && has_file);
        update_action("save-as", !busy);
        update_action("revert", !busy);
        update_action("print-preview", !busy);
        update_action("print", !busy);
        update_action("close", !busy);
        update_action("undo", !busy && can_undo);
        update_action("redo", !busy && can_redo);
        update_action("cut", !busy);
        update_action("copy", !busy);
        update_action("paste", !busy);
        update_action("select-all", !busy);
        update_action("comment", !busy);
        update_action("uncomment", !busy);
        update_action("insert-date-and-time", !busy);
        update_action("sort-lines", !busy);
        update_action("join-lines", !busy);
        update_action("delete", !busy);
        update_action("duplicate", !busy);
        update_action("show-go-to-line", !busy);
    }

    construct {
        this.actions.action_added.connect((_, action_name) => {
            this.action_added(action_name);
        });
        this.actions.action_enabled_changed.connect((_, action_name, enabled) => {
            this.action_enabled_changed(action_name, enabled);
        });
        this.actions.action_removed.connect((_, action_name) => {
            this.action_removed(action_name);
        });
        this.actions.action_state_changed.connect((_, action_name, state) => {
            this.action_state_changed(action_name, state);
        });
        this.actions.add_action_entries(action_entries,this);

        this.document.notify["can-undo"].connect((sb, pspec) => {
            this.update_actions();
        });
        this.document.notify["can-redo"].connect((sb, pspec) => {
            this.update_actions();
        });
        this.document.notify["file"].connect((sf, pspec) => {
            this.update_actions();
        });
        this.document.notify["loading"].connect((d, pspec) => {
            this.update_actions();
        });
        this.document.notify["saving"].connect((d, pspec) => {
            this.update_actions();
        });

        this.update_actions();
    }

    public
    DocumentActions(Bedit.Document document) {
        Object(
            document: document
        );
    }

    /* === Action Group Interface ========================================================================= */

    public void
    activate_action(string action_name, Variant? parameter) {
        this.actions.activate_action(action_name, parameter);
    }

    public void
    change_action_state(string action_name, Variant value) {
        this.actions.change_action_state(action_name, value);
    }

    public override bool
    get_action_enabled(string action_name) {
        return this.actions.get_action_enabled(action_name);
    }

    public override unowned VariantType?
    get_action_parameter_type(string action_name) {
        return this.actions.get_action_parameter_type(action_name);
    }

    public override Variant?
    get_action_state(string action_name) {
        return this.actions.get_action_state(action_name);
    }

    public override Variant?
    get_action_state_hint(string action_name) {
        return this.actions.get_action_state_hint(action_name);
    }

    public override bool
    has_action(string action_name) {
        return this.actions.has_action(action_name);
    }

    public string[]
    list_actions() {
        return this.actions.list_actions();
    }

    public override bool
    query_action(
        string action_name,
        out bool enabled,
        out unowned VariantType parameter_type,
        out unowned VariantType state_type,
        out Variant state_hint,
        out Variant state
    ) {
        return this.actions.query_action(
            action_name,
            out enabled,
            out parameter_type,
            out state_type,
            out state_hint,
            out state
        );
    }
}
