namespace Bedit {

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-searchbar.ui")]
public sealed class Searchbar : Gtk.Widget {

    [GtkChild]
    unowned Gtk.Revealer revealer;

    [GtkChild]
    unowned Gtk.Entry search_entry;

    [GtkChild]
    unowned Gtk.Entry replace_entry;

    [GtkChild]
    unowned Gtk.Button prev_button;

    [GtkChild]
    unowned Gtk.Button next_button;

    [GtkChild]
    unowned Gtk.Button case_sensitive_toggle;

    [GtkChild]
    unowned Gtk.ToggleButton regex_toggle;

    [GtkChild]
    unowned Gtk.ToggleButton replace_button;

    [GtkChild]
    unowned Gtk.Button replace_all_button;

    static construct {
        typeof (Brk.ButtonGroup).ensure();

        set_layout_manager_type (typeof (Gtk.BoxLayout));
    }
}

}
