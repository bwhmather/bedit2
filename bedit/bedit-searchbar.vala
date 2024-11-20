namespace Bedit {

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-searchbar.ui")]
public sealed class Searchbar : Gtk.Widget {
    static construct {
        typeof (Brk.ButtonGroup).ensure();

        set_layout_manager_type (typeof (Gtk.BoxLayout));
    }
}

}
