private sealed class Bedit.ToolbarViewBar : Gtk.Widget {
    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("bar");
        set_accessible_role(GROUP);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.VERTICAL, -1);
        var layout_manager = this.layout_manager as Gtk.BoxLayout;
        layout_manager.orientation = VERTICAL;
    }

    public override void
    dispose() {
        while (this.get_last_child() != null) {
            this.get_last_child().unparent();
        }
    }

    public void
    append(Gtk.Widget child) {
        return_if_fail(child.parent == null);
        child.insert_before(this, null);
    }
}


public sealed class Bedit.ToolbarView : Gtk.Widget, Gtk.Buildable {
    Bedit.ToolbarViewBar top_bar;
    Bedit.ToolbarViewBar bottom_bar;

    Gtk.Widget? _content;
    public Gtk.Widget? content {
        get {
            return this.content;
        }
        set {
            if (this._content != null) {
                this._content.unparent();
            }
            this._content = value;
            this._content.insert_before(this, bottom_bar);
        }
    }

    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("toolbarview");
        set_accessible_role(GROUP);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.VERTICAL, -1);
        var layout_manager = this.layout_manager as Gtk.BoxLayout;
        layout_manager.orientation = VERTICAL;

        this.top_bar = new Bedit.ToolbarViewBar();
        this.top_bar.vexpand = false;
        this.top_bar.add_css_class("top-bar");
        this.top_bar.set_parent(this);

        this.bottom_bar = new Bedit.ToolbarViewBar();
        this.bottom_bar.vexpand = false;
        this.bottom_bar.add_css_class("bottom-bar");
        this.bottom_bar.set_parent(this);
    }

    public override void
    dispose() {
        if (this.top_bar != null) {
            this.top_bar.unparent();
            this.top_bar = null;
        }

        if (this.bottom_bar != null) {
            this.bottom_bar.unparent();
            this.bottom_bar = null;
        }

        if (this.content != null) {
            this.content.unparent();
            this.content = null;
        }
    }

    public void
    add_child(Gtk.Builder builder, GLib.Object child, string? type) {
        if (type == "top") {
            this.add_top_bar(child as Gtk.Widget);
            return;
        }
        
        if (type == "bottom") {
            this.add_bottom_bar(child as Gtk.Widget);
            return;
        }

        if (type == null && child is Gtk.Widget) {
            this.content = child as Gtk.Widget;
            return;
        }

        base.add_child(builder, child, type);
    }

    public void
    add_top_bar(Gtk.Widget bar) {
        this.top_bar.append(bar);
    }

    public void
    add_bottom_bar(Gtk.Widget bar) {
        this.bottom_bar.append(bar);
    }
}
