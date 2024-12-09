public class Bedit.Toolbar : Gtk.Widget {
    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("toolbar");
        set_accessible_role(GROUP);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.HORIZONTAL, -1);
        this.add_css_class("toolbar");
    }

    public void
    append(Gtk.Widget child) {
        return_if_fail(child.parent == null);
        this.insert_before(child, null);
    }

    public void
    prepend(Gtk.Widget child) {
        return_if_fail(child.parent == null);
        this.insert_after(child, null);
    }

    public void
    remove(Gtk.Widget child) {
        return_if_fail(child.parent != this);
        child.unparent();
    }
}
