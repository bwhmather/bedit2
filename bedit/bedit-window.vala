[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-window.ui")]
public sealed class Bedit.Window : Gtk.ApplicationWindow {
    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned Brk.TabView tab_view;

    public Bedit.Document? active_document { get; private set; }

    /* === File Actions =================================================================================== */

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
    }

    /* === Search Actions ================================================================================= */

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

    const GLib.ActionEntry[] action_entries = {
        {"new", on_new},
        {"open", on_open},
        {"close", on_close_window},
        {"find", on_find},
        {"find-next", on_find_next},
        {"find-previous", on_find_previous},
        {"replace", on_replace},
        {"replace-all", on_replace_all},
    };

    class construct {
        typeof (Brk.TabBar).ensure();
        typeof (Brk.TabView).ensure();
        typeof (Brk.ToolbarView).ensure();
        typeof (Brk.Toolbar).ensure();
        typeof (Bedit.Document).ensure();
        typeof (Bedit.Searchbar).ensure();
    }

    construct {
        tab_view.notify["selected-page"].connect((view, pspec) => {
            var page = tab_view.selected_page;
            if (page == null) {
                this.active_document = null;
                this.insert_action_group("doc", null);
            } else {
                this.active_document = page.child as Bedit.Document;
                this.insert_action_group("doc", new Bedit.DocumentActions(this.active_document));
            }
        });

        var w = (this as Gtk.Widget);
        w.destroy.connect((w) => {
            cancellable.cancel();
        });

        this.add_action_entries(action_entries, this);
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
