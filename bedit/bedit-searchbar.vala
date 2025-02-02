namespace Bedit {

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-searchbar.ui")]
public sealed class Searchbar : Gtk.Widget {

    [GtkChild]
    unowned Gtk.Entry search_entry;

    [GtkChild]
    unowned Gtk.Entry replace_entry;

    public bool search_active { get; private set; }
    public bool replace_active { get; private set; }
    public bool regex { get; set; }
    public bool case_sensitive { get; set; }

    static construct {
        typeof (Brk.ButtonGroup).ensure();

        set_layout_manager_type (typeof (Gtk.BoxLayout));
    }

    /*
    private bool
    search_entry_on_key_press_event(Gtk.Widget search_entry, Gdk.KeyEvent event) {
        assert(search_entry == this.search_entry);
        return true;
    }
    */

    private void
    search_entry_on_activate(Gtk.Entry search_entry) {
        assert(search_entry == this.search_entry);
    }

    private void
    search_entry_on_changed(Gtk.Editable search_entry) {
        assert(search_entry == this.search_entry);
    }

    /*
    private void
    search_entry_on_escaped(Gtk.Entry search_entry) {
        assert(search_entry == this.search_entry);
    }
    */

    private void
    replace_entry_on_activate(Gtk.Entry replace_entry) {
        assert(replace_entry == this.replace_entry);
    }

    /*
    private void
    replace_entry_on_escaped(Gtk.Entry replace_entry) {
        assert(replace_entry == this.replace_entry);
    }
    */


    public GLib.SimpleActionGroup search_actions = new GLib.SimpleActionGroup();

    private void
    action_find_prev() {

    }

    private void
    action_find_next() {

    }

    private void
    action_replace() {

    }

    private void
    action_replace_all() {

    }

    const GLib.ActionEntry[] search_action_entries = {
        {"find-previous", action_find_prev},
        {"find-next", action_find_next},
        {"replace", action_replace},
        {"replace-all", action_replace_all},
    };

    private void
    search_actions_set_action_enabled(string name, bool enabled) {
        var action = this.search_actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    search_actions_update() {
        search_actions_set_action_enabled("find-previous", this.search_active);
        search_actions_set_action_enabled("find-next", this.search_active);
        search_actions_set_action_enabled("replace", this.replace_active);
        search_actions_set_action_enabled("replace-all", this.replace_active);
    }

    construct {
        this.search_actions.add_action_entries(search_action_entries, this);
        this.search_actions.add_action(new GLib.PropertyAction("case-sensitive", this, "case-sensitive"));
        this.search_actions.add_action(new GLib.PropertyAction("regex", this, "regex"));

        this.insert_action_group("search", this.search_actions);

        this.notify["search-active"].connect((d, pspec) => { this.search_actions_update(); });
        this.notify["replace-active"].connect((d, pspec) => { this.search_actions_update(); });
        this.search_actions_update();


//        this.search_entry.connect("key-press-event", this.search_entry_on_key_press_event);
        this.search_entry.activate.connect(this.search_entry_on_activate);
        this.search_entry.changed.connect(this.search_entry_on_changed);
        //this.search_entry.escaped.connect(this.search_entry_on_escaped);

        this.replace_entry.activate.connect(this.replace_entry_on_activate);
        //this.replace_entry.connect("escaped", this.replace_entry_on_escaped);

    }
}

}
