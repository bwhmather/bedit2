[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-window.ui")]
public sealed class Bedit.Window : Gtk.ApplicationWindow {

    private GLib.SimpleActionGroup doc_actions = new GLib.SimpleActionGroup();

    [GtkChild]
    private unowned Brk.TabView tab_view;

    public Bedit.Document? active_document { get; private set; }

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
                this.insert_action_group("doc", doc_actions);
            }
        });

        GLib.SimpleAction action;
        var win_actions = this as GLib.ActionMap;

        // File.
        action = new SimpleAction("new", null);
        action.activate.connect(() => {
            var document = new Bedit.Document();
            this.add_document(document);
        });
        win_actions.add_action(action);

        action = new SimpleAction("open", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("save", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("save-as", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("revert", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("print-preview", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("print", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("close", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("close", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);


        // Edit.
        action = new SimpleAction("undo", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("redo", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("cut", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("copy", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("paste", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("delete", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("duplicate", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("select-all", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("comment", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("uncomment", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("insert-date-and-time", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("sort-lines", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        action = new SimpleAction("join-lines", null);
        action.activate.connect(() => {

        });
        doc_actions.add_action(action);

        // View.
        // ...

        // Search.
        action = new SimpleAction("find", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("find-next", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("find-previous", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("replace", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("replace-all", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);

        action = new SimpleAction("show-go-to-line", null);
        action.activate.connect(() => {

        });
        win_actions.add_action(action);
    }

    public Window(Gtk.Application application) {
        Object(
            application: application,
            show_menubar: true
        );
    }

    public void add_document(Bedit.Document document) {
        Brk.TabPage page = tab_view.append(document);
        document.bind_property("title", page, "title", SYNC_CREATE);
    }
}
