/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
/*
private enum Bedit.FileDialogViewMode {
    Icons,
    Tree,
    List
}

private class Bedit.FileDialogState {
    public Bedit.ViewMode view_mode;
    public Gio.Volume volume;
    public string folder; // Path to root folder under mount.
    public string[] expanded;  // Sorted list of expanded directories under the current mount.

    public Bedit.FileDialogState
    dup();
}
*/

//state: map[windowId]FileDialogState

// On begin:
// Try to load state.
// Fall back to creating a new state with the passed in root folder.
// Fall back to creating a new state from the home directory.


// On end:
// Set or update saved state for window.

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-filter-view.ui")]
private sealed class Bedit.FileDialogFilterView : Gtk.Widget {
    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogFilterView));
        base.dispose();
    }
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-list-view.ui")]
private sealed class Bedit.FileDialogListView : Gtk.Widget {
    public GLib.File root_directory { get; set; }
    private Gtk.DirectoryList directory_list;

    [GtkChild]
    private unowned Gtk.ColumnView column_view;

    [GtkChild]
    private unowned Gtk.ColumnViewColumn name_column;

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.directory_list = new Gtk.DirectoryList("standard::display-name", GLib.File.new_for_path("/home/ben/pro"));
        this.directory_list.monitored = true;  // TODO
//        this.bind_property("root", this.directory_list, "file", SYNC_CREATE);

        this.column_view.model = new Gtk.MultiSelection(directory_list);

        var factory = new Gtk.SignalListItemFactory();
        factory.setup.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            var label = new Gtk.Label("");
            label.halign = START;
            listitem.child = label;
        });
        factory.bind.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            Gtk.Label label = (Gtk.Label) listitem.child;
            GLib.FileInfo info = (GLib.FileInfo) listitem.item;
            label.label = info.get_display_name();
        });
        this.name_column.factory = factory;
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogListView));
        base.dispose();
    }
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-icon-view.ui")]
private sealed class Bedit.FileDialogIconView : Gtk.Widget {
    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogIconView));
        base.dispose();
    }
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-tree-view.ui")]
private sealed class Bedit.FileDialogTreeView : Gtk.Widget {

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogTreeView));
        base.dispose();
    }
}


[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog.ui")]
private sealed class Bedit.FileDialogWindow : Gtk.Window {
    [GtkChild]
    private unowned Gtk.Stack view_stack;

    [GtkChild]
    private unowned Bedit.FileDialogListView list_view;

    class construct {
        typeof (Bedit.FileDialogFilterView).ensure();
        typeof (Bedit.FileDialogListView).ensure();
        typeof (Bedit.FileDialogIconView).ensure();
        typeof (Bedit.FileDialogTreeView).ensure();
    }

    construct {
        this.view_stack.visible_child = this.list_view;
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogWindow));
        base.dispose();
    }
}

sealed class Bedit.FileDialog : GLib.Object {
    public string accept_label { get; set; }

    public Gtk.FileFilter default_filter { get; set; }
    public GLib.ListModel filters { get; set; }

    public GLib.File initial_file { get; set; }
    public GLib.File initial_folder { get; set; }
    public string initial_name { get; set; }

    public Gtk.Window parent { get; set; }

    public async GLib.File?
    open(Gtk.Window? parent, GLib.Cancellable cancellable) throws Error {
        var window = new Bedit.FileDialogWindow();
        window.present();
        return null;
    }
}
