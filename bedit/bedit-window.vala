[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-window.ui")]
public sealed class Bedit.Window : Gtk.ApplicationWindow {
    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned Brk.TabView tab_view;

    public Bedit.Document? active_document { get; private set; }


    /* === Document Operations ============================================================================ */

    private async bool
    document_save_async(Bedit.Document document) throws Error {
        var file = document.file;
        if (file == null) {
            var file_dialog = new Gtk.FileDialog();
            try {
                file = yield file_dialog.save(this, null);
            } catch (Gtk.DialogError.DISMISSED err) {
                return false;
            }
        }

        yield document.save_async(file);

        return true;
    }

    private async bool
    document_save_as_async(Bedit.Document document) throws Error {
        GLib.File file;
        var file_dialog = new Gtk.FileDialog();
        try {
            file = yield file_dialog.save(this, null);
        } catch (Gtk.DialogError.DISMISSED err) {
            return false;
        }

        yield document.save_async(file);

        return true;
    }

    private async bool
    document_confirm_close_async(Bedit.Document document) throws Error {
        if (document.loading) {
            return true;
        }

        var handle = document.notify["saving"].connect((d, pspec) => {
            document_confirm_close_async.callback();
        });
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


        return yield this.document_save_async(document);
    }

    /* === Document Actions =============================================================================== */

    private GLib.SimpleActionGroup document_actions = new GLib.SimpleActionGroup();

    /* --- Saving Documents ------------------------------------------------------------------------------- */

    private void
    action_doc_save() {
        this.document_save_async.begin(this.active_document, (_, res) => {
            try {
                this.document_save_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    private void
    action_doc_save_as() {
        this.document_save_as_async.begin(this.active_document, (_, res) => {
            try {
                this.document_save_as_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    private void
    action_doc_revert() {
    }

    /* --- Printing Documents ----------------------------------------------------------------------------- */

    private void
    action_doc_print() {
    }

    private void
    action_doc_print_preview() {
    }

    /* --- Closing Tabs ----------------------------------------------------------------------------------- */


    private void
    action_doc_close() {
    }

    /* --- Edit History ----------------------------------------------------------------------------------- */

    private void
    action_doc_undo() {
        this.active_document.undo();
    }

    private void
    action_doc_redo() {
        this.active_document.redo();
    }

    /* --- Clipboard -------------------------------------------------------------------------------------- */

    private void
    action_doc_cut() {
    }

    private void
    action_doc_copy() {
    }

    private void
    action_doc_paste() {
    }

    /* --- Selection -------------------------------------------------------------------------------------- */

    private void
    action_doc_select_all() {
    }

    /* --- Commenting and Uncommenting -------------------------------------------------------------------- */

    private void
    action_doc_comment() {
    }

    private void
    action_doc_uncomment() {
    }

    /* --- Insert Date and Time --------------------------------------------------------------------------- */

    private void
    action_doc_insert_date_and_time() {
    }

    /* --- Line Operations -------------------------------------------------------------------------------- */

    private void
    action_doc_sort_lines() {
    }

    private void
    action_doc_join_lines() {
    }

    private void
    action_doc_delete_line() {
    }

    private void
    action_doc_duplicate_line() {
    }

    /* --- Navigate to Line ------------------------------------------------------------------------------- */

    private void
    action_doc_show_go_to_line() {
    }

    /* --- Document Action State -------------------------------------------------------------------------- */

    const GLib.ActionEntry[] document_action_entries = {
        {"save", action_doc_save},
        {"save-as", action_doc_save_as},
        {"revert", action_doc_revert},
        {"print-preview", action_doc_print_preview},
        {"print", action_doc_print},
        {"close", action_doc_close},
        {"undo", action_doc_undo},
        {"redo", action_doc_redo},
        {"cut", action_doc_cut},
        {"copy", action_doc_copy},
        {"paste", action_doc_paste},
        {"select-all", action_doc_select_all},
        {"comment", action_doc_comment},
        {"uncomment", action_doc_uncomment},
        {"insert-date-and-time", action_doc_insert_date_and_time},
        {"sort-lines", action_doc_sort_lines},
        {"join-lines", action_doc_join_lines},
        {"delete", action_doc_delete_line},
        {"duplicate", action_doc_duplicate_line},
        {"show-go-to-line", action_doc_show_go_to_line},
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
    action_win_new() {
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
    action_win_open() {
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
    action_win_close_window() {
        this.close_request();
    }

    /* --- Find ------------------------------------------------------------------------------------------- */

    private void
    action_win_find() {
    }

    private void
    action_win_find_next() {
    }

    private void
    action_win_find_previous() {
    }

    /* --- Replace ---------------------------------------------------------------------------------------- */

    private void
    action_win_replace() {
    }

    private void
    action_win_replace_all() {
    }

    /* --- Window Action State ---------------------------------------------------------------------------- */

    const GLib.ActionEntry[] window_action_entries = {
        {"new", action_win_new},
        {"open", action_win_open},
        {"close", action_win_close_window},
        {"find", action_win_find},
        {"find-next", action_win_find_next},
        {"find-previous", action_win_find_previous},
        {"replace", action_win_replace},
        {"replace-all", action_win_replace_all},
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

    private bool
    on_window_close_request(Gtk.Window window) {

        return false;
    }

    private bool
    on_tab_view_close_page_request(Brk.TabView view, Brk.TabPage page) {
        var document = page.child as Bedit.Document;
        this.document_confirm_close_async.begin(document, (_, res) => {
            try {
                var should_close = this.document_confirm_close_async.end(res);
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
