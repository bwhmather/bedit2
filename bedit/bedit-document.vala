[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {

    [GtkChild]
    private unowned GtkSource.View source_view;
    private unowned GtkSource.Buffer source_buffer;
    private GtkSource.File source_file;

    public string title { get; private set; }
    public unowned GLib.File? file { get; construct; }
    public bool modified { get; }

    public bool loading { get; private set; }
    public bool saving { get; private set; }

    private GLib.SimpleActionGroup actions = new GLib.SimpleActionGroup();
    public unowned GLib.ActionGroup action_group { get { return actions; } }

    /* === File Actions =================================================================================== */

    /* --- Saving Documents ------------------------------------------------------------------------------- */

    private async void
    do_save() throws Error {
        yield this.save(null);
    }

    private async void
    do_save_as() throws Error {
        var file_dialog = new Gtk.FileDialog();
        var file = yield file_dialog.save(this.root as Gtk.Window, null);
        yield this.save_as(file, null);
    }

    private void
    on_save() {
        if (this.file == null) {
            this.do_save_as.begin((_, res) => {
                try {
                    this.do_save_as.end(res);
                } catch (Error err) {
                    warning("Error: %s\n", err.message);
                }
            });
        } else {
            this.do_save.begin((_, res) => {
                try {
                    this.do_save.end(res);
                } catch (Error err) {
                    warning("Error: %s\n", err.message);
                }
            });
        }
    }

    private void
    on_save_as() {
        this.do_save_as.begin((_, res) => {
            try {
                this.do_save_as.end(res);
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
    }

    /* === Edit Actions =================================================================================== */

    /* --- Edit History ----------------------------------------------------------------------------------- */
    private void
    on_undo() {
        this.source_buffer.undo();
    }

    private void
    on_redo() {
        this.source_buffer.redo();
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

    /* === Search Actions ================================================================================= */

    /* --- Navigate to Line ------------------------------------------------------------------------------- */

    private void
    on_show_go_to_line() {
    }

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
        bool busy = this.saving || this.loading;
        bool can_undo = this.source_buffer.can_undo;
        bool can_redo = this.source_buffer.can_redo;

        update_action("save", !busy);
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

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.actions.add_action_entries(action_entries,this);

        this.source_buffer = source_view.get_buffer() as GtkSource.Buffer;
        this.source_buffer.notify["can-undo"].connect((sb, pspec) => {
            this.update_actions();
        });
        this.source_buffer.notify["can-redo"].connect((sb, pspec) => {
            this.update_actions();
        });

        this.source_file = new GtkSource.File();
        this.source_file.set_location(file);
        this.source_file.notify["location"].connect((sf, pspec) => {
            this.file = this.source_file.location;
            this.title = file.get_basename();
        });

        this.notify["loading"].connect((d, pspec) => {
            this.update_actions();
        });
        this.notify["saving"].connect((d, pspec) => {
            this.update_actions();
        });

        this.update_actions();

        if (file != null) {
            reload_async.begin(null);
        }
    }

    public Document.for_file(GLib.File file) {
        Object(file: file);
    }

    public async bool
    reload_async(GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        loading = true;

        var source_loader = new GtkSource.FileLoader(source_buffer, source_file);
        yield source_loader.load_async(Priority.LOW, cancellable, null);

        loading = false;
        return true;
    }

    public async bool
    save(GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        saving = true;

        var source_saver = new GtkSource.FileSaver(source_buffer, source_file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, cancellable, null);

        saving = false;
        return true;
    }

    public async bool
    save_as(GLib.File file, GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        saving = true;

        var source_saver = new GtkSource.FileSaver.with_target(source_buffer, source_file, file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, cancellable, null);

        saving = false;
        return true;
    }
}
