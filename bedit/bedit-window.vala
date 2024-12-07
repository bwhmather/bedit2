[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-window.ui")]
public sealed class Bedit.Window : Gtk.ApplicationWindow {
    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned Brk.TabView tab_view;

    public Bedit.Document? active_document { get; private set; }

    /* === Document Actions =============================================================================== */

    private GLib.SimpleActionGroup document_actions = new GLib.SimpleActionGroup();

    /* --- Saving Documents ------------------------------------------------------------------------------- */

    private async bool
    do_save_async() throws Error {
        var file = this.active_document.file;
        if (file == null) {
            var file_dialog = new Gtk.FileDialog();
            try {
                file = yield file_dialog.save(this, null);
            } catch (Gtk.DialogError.DISMISSED err) {
                return false;
            }
        }

        yield this.active_document.save_async(file);

        return true;
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

    private async bool
    do_save_as_async() throws Error {
        GLib.File file;
        var file_dialog = new Gtk.FileDialog();
        try {
            file = yield file_dialog.save(this, null);
        } catch (Gtk.DialogError.DISMISSED err) {
            return false;
        }

        yield this.active_document.save_async(file);

        return true;
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

    /* --- Closing Tabs ----------------------------------------------------------------------------------- */


    private void
    on_close() {
    }

    /* --- Edit History ----------------------------------------------------------------------------------- */

    private void
    on_undo() {
        this.active_document.undo();
    }

    private void
    on_redo() {
        this.active_document.redo();
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

    /* --- Document Action State -------------------------------------------------------------------------- */

    const GLib.ActionEntry[] document_action_entries = {
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
    document_actions_set_action_enabled(string name, bool enabled) {
        var action = this.document_actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    document_actions_update() {
        bool exists = this.active_document != null;
        bool busy = exists && (this.active_document.saving || this.active_document.loading);
        bool has_file = exists && this.active_document.file != null;
        bool can_undo = exists && this.active_document.can_undo;
        bool can_redo = exists && this.active_document.can_redo;

        document_actions_set_action_enabled("save", exists && !busy && has_file);
        document_actions_set_action_enabled("save-as", exists && !busy);
        document_actions_set_action_enabled("revert", exists && !busy);
        document_actions_set_action_enabled("print-preview", exists && !busy);
        document_actions_set_action_enabled("print", exists && !busy);
        document_actions_set_action_enabled("close", exists && !busy);
        document_actions_set_action_enabled("undo", exists && !busy && can_undo);
        document_actions_set_action_enabled("redo", exists && !busy && can_redo);
        document_actions_set_action_enabled("cut", exists && !busy);
        document_actions_set_action_enabled("copy", exists && !busy);
        document_actions_set_action_enabled("paste", exists && !busy);
        document_actions_set_action_enabled("select-all", exists && !busy);
        document_actions_set_action_enabled("comment", exists && !busy);
        document_actions_set_action_enabled("uncomment", exists && !busy);
        document_actions_set_action_enabled("insert-date-and-time", exists && !busy);
        document_actions_set_action_enabled("sort-lines", exists && !busy);
        document_actions_set_action_enabled("join-lines", exists && !busy);
        document_actions_set_action_enabled("delete", exists && !busy);
        document_actions_set_action_enabled("duplicate", exists && !busy);
        document_actions_set_action_enabled("show-go-to-line", exists && !busy);
    }

    private void
    document_actions_update_on_notify(string name) {
        ulong handle = 0;
        Bedit.Document? current;

        if (this.active_document != null) {
            handle = this.active_document.notify[name].connect((d, pspec) => { this.document_actions_update(); });
        }
        current = this.active_document;

        this.notify["active-document"].connect((da, pspec) => {
            if (current != null) {
                current.disconnect(handle);
            }
            if (this.active_document != null) {
                handle = this.active_document.notify[name].connect((d, pspec) => { this.document_actions_update(); });
            }
            current = this.active_document;
        });
    }

    private void
    document_actions_init() {
        this.document_actions.add_action_entries(document_action_entries,this);
        this.insert_action_group("doc", this.document_actions);

        this.document_actions_update_on_notify("can-undo");
        this.document_actions_update_on_notify("can-redo");
        this.document_actions_update_on_notify("file");
        this.document_actions_update_on_notify("loading");
        this.document_actions_update_on_notify("saving");

        this.notify["active-document"].connect((da, pspec) => {
            this.document_actions_update();
        });

        this.document_actions_update();
    }

    /* === Window Actions ================================================================================= */

    /* --- Creating New Documents and Opening Existing Ones ----------------------------------------------- */

    private void
    on_new() {
        var document = new Bedit.Document();
        this.add_document(document);
    }

    private async void
    do_open() throws Error {
        var file_dialog = new Gtk.FileDialog();
        var file = yield file_dialog.open(this, this.cancellable);

        var document = new Bedit.Document.for_file(file);
        this.add_document(document);
    }

    private void
    on_open() {
        this.do_open.begin((_, res) => {
            try {
                this.do_open.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    /* --- Closing Windows and Tabs ----------------------------------------------------------------------- */

    private void
    on_close_window() {
        this.close_request();
    }

    /* --- Find ------------------------------------------------------------------------------------------- */

    private void
    on_find() {
    }

    private void
    on_find_next() {
    }

    private void
    on_find_previous() {
    }

    /* --- Replace ---------------------------------------------------------------------------------------- */

    private void
    on_replace() {
    }

    private void
    on_replace_all() {
    }

    /* --- Window Action State ---------------------------------------------------------------------------- */

    const GLib.ActionEntry[] window_action_entries = {
        {"new", on_new},
        {"open", on_open},
        {"close", on_close_window},
        {"find", on_find},
        {"find-next", on_find_next},
        {"find-previous", on_find_previous},
        {"replace", on_replace},
        {"replace-all", on_replace_all},
    };

    private void
    window_actions_init() {
        this.add_action_entries(window_action_entries, this);
    }

    /* === Lifecycle ====================================================================================== */

    private void
    on_tab_view_selected_tab_changed(GLib.Object _, GLib.ParamSpec pspec) {
        var page = this.tab_view.selected_page;
        if (page == null) {
            this.active_document = null;
        } else {
            this.active_document = page.child as Bedit.Document;
        }
    }

    private async bool
    confirm_close_async(Bedit.Document document) throws Error {
        if (document.loading) {
            return true;
        }

        var handle = document.notify["saving"].connect((d, pspec) => { confirm_close_async.callback(); });
        while (this.active_document.saving) {
            yield;
        }
        document.disconnect(handle);

//        if (!document.modified) {
//          return true;
//    }


        var action = yield Bedit.CloseConfirmationDialog.run_async(this.cancellable, this, document);
        switch (action) {
        case CANCEL:
            return false;
        case DISCARD:
            return true;
        case SAVE:
            break;
        }

        //if (!active_document.modified) {
        //    return true;
        // }

        // TODO block;
        // TODO return true;


        return yield this.do_save_async();
    }

    private bool
    on_window_close_request(Gtk.Window window) {

        return true;
    }

    private bool
    on_tab_view_close_page_request(Brk.TabView view, Brk.TabPage page) {
        var document = page.child as Bedit.Document;
        this.confirm_close_async.begin(document, (_, res) => {
            try {
                var should_close = this.confirm_close_async.end(res);
                view.close_page_finish(page, should_close);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
        return Gdk.EVENT_STOP;
    }

    class construct {
        typeof (Brk.TabBar).ensure();
        typeof (Brk.TabView).ensure();
        typeof (Brk.ToolbarView).ensure();
        typeof (Brk.Toolbar).ensure();
        typeof (Bedit.Document).ensure();
        typeof (Bedit.Searchbar).ensure();
    }

    construct {
        tab_view.notify["selected-page"].connect(on_tab_view_selected_tab_changed);
        tab_view.close_page.connect(on_tab_view_close_page_request);

        this.close_request.connect(on_window_close_request);
        var w = (this as Gtk.Widget);
        w.destroy.connect((w) => {
            cancellable.cancel();
        });

        this.window_actions_init();
        this.document_actions_init();
    }

    public Window(Gtk.Application application) {
        Object(
            application: application,
            show_menubar: true
        );
    }

    public void
    add_document(Bedit.Document document) {
        Brk.TabPage page = tab_view.append(document);
        document.bind_property("title", page, "title", SYNC_CREATE);
    }
}
