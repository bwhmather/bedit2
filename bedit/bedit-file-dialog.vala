/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
private enum Bedit.FileDialogViewMode {
    LIST,
    ICON,
    TREE
}
/*
private sealed class Bedit.FileDialogState : GLib.Object {
    // Application state.
    public Bedit.FileDialogViewMode view_mode;

    public bool show_binary { get; set; }
    public bool show_hidden { get; set; }

    // Window state.
    // Shared state.
    public GLib.File root_directory; // Path to root folder under mount.

    // Tree view specific.
    public string[] expanded;  // Sorted list of expanded directories under the current mount.

    // List view specific.
    public string[] sort_columns;

    public Bedit.FileDialogState
    dup() {
    }
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
    public GLib.File root_directory { get; set; }

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

    [GtkChild]
    private unowned Gtk.ColumnViewColumn size_column;

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.directory_list = new Gtk.DirectoryList(
            "standard::display-name,standard::size,time::modified,standard::type",
            GLib.File.new_for_path("/home/ben/pro")
        );
        this.directory_list.monitored = true;  // TODO
//        this.bind_property("root", this.directory_list, "file", SYNC_CREATE);

        this.column_view.model = new Gtk.MultiSelection(directory_list);

        // Name column.
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

        // Size column.
        factory = new Gtk.SignalListItemFactory();
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
            label.label = GLib.format_size(info.get_size());
        });
        this.size_column.factory = factory;
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogListView));
        base.dispose();
    }
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-icon-view.ui")]
private sealed class Bedit.FileDialogIconView : Gtk.Widget {
    public GLib.File root_directory { get; set; }
    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogIconView));
        base.dispose();
    }
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-tree-view.ui")]
private sealed class Bedit.FileDialogTreeView : Gtk.Widget {
    public GLib.File root_directory { get; set; }

    [GtkChild]
    private unowned Gtk.ListView list_view;

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        var directory_list = new Gtk.DirectoryList(
            "standard::display-name,standard::size,time::modified,standard::type",
            GLib.File.new_for_path("/home/ben/pro")
        );
        directory_list.monitored = true;  // TODO
//        this.bind_property("root", this.directory_list, "file", SYNC_CREATE);

        var factory = new Gtk.SignalListItemFactory();
        factory.setup.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;

            var label = new Gtk.Label("");
            label.halign = START;

            var expander = new Gtk.TreeExpander();
            expander.child = label;

            listitem.child = expander;
        });
        factory.bind.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            var expander = (Gtk.TreeExpander) listitem.child;
            var label = (Gtk.Label) expander.child;

            var row = (Gtk.TreeListRow) listitem.item;
            expander.list_row = row;

            var info = (GLib.FileInfo) row.item;
            label.label = info.get_display_name();
        });
        this.list_view.factory = factory;

        var tree_list_model = new Gtk.TreeListModel(
            directory_list,
            false,  // passthrough
            false,  // autoexpand
            (item) => {
                return null;
            }
        );

        this.list_view.model = new Gtk.MultiSelection(tree_list_model);
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogTreeView));
        base.dispose();
    }
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog.ui")]
private sealed class Bedit.FileDialogWindow : Gtk.Window {
    // Path to root folder under mount.
    public GLib.File root_directory { get; set; }

    public signal void open(GLib.File result);

    public GLib.SimpleActionGroup dialog_actions = new GLib.SimpleActionGroup();

    /* === Views ========================================================================================== */

    public Bedit.FileDialogViewMode view_mode { get; set; default = LIST; }

    public bool show_binary { get; set; }
    public bool show_hidden { get; set; }

    [GtkChild]
    private unowned Gtk.Stack view_stack;

    private void
    view_stack_update_visible_child() {
        if (this.filter_view_enabled) {
            this.view_stack.visible_child = this.filter_view;
            return;
        }
        switch (this.view_mode) {
        case LIST:
            this.view_stack.visible_child = this.list_view;
            break;
        case ICON:
            this.view_stack.visible_child = this.icon_view;
            break;
        case TREE:
            this.view_stack.visible_child = this.tree_view;
            break;
        }
    }

    private void
    views_init() {
        this.dialog_actions.add_action(new GLib.PropertyAction("show-binary", this, "show-binary"));
        this.dialog_actions.add_action(new GLib.PropertyAction("show-hidden", this, "show-hidden"));

        this.dialog_actions.add_action(new GLib.PropertyAction("view-mode", this, "view-mode"));

        this.notify["filter-view-visible"].connect(this.view_stack_update_visible_child);
        this.notify["view-mode"].connect(this.view_stack_update_visible_child);
        this.view_stack_update_visible_child();
    }


    /* --- Filter View ------------------------------------------------------------------------------------ */

    public bool filter_view_enabled { get; set; }

    [GtkChild]
    private unowned Bedit.FileDialogFilterView filter_view;

    private void
    filter_view_init() {
        this.bind_property("root-directory", this.filter_view, "root-directory", SYNC_CREATE | BIDIRECTIONAL);
    }

    /* --- List View -------------------------------------------------------------------------------------- */

    public string[] sort_columns;

    [GtkChild]
    private unowned Bedit.FileDialogListView list_view;

    private void
    list_view_init() {
        this.bind_property("root-directory", this.list_view, "root-directory", SYNC_CREATE | BIDIRECTIONAL);
    }

    /* --- Icon View -------------------------------------------------------------------------------------- */

    [GtkChild]
    private unowned Bedit.FileDialogIconView icon_view;

    private void
    icon_view_init() {
        this.bind_property("root-directory", this.icon_view, "root-directory", SYNC_CREATE | BIDIRECTIONAL);
    }

    /* --- Tree View -------------------------------------------------------------------------------------- */

    // Sorted list of expanded directories under the current mount.
    public string[] expanded;

    [GtkChild]
    private unowned Bedit.FileDialogTreeView tree_view;

    private void
    tree_view_init() {
        this.bind_property("root-directory", this.tree_view, "root-directory", SYNC_CREATE | BIDIRECTIONAL);
    }

    /* === Lifecycle ======================================================================================= */

    class construct {
        typeof (Bedit.FileDialogFilterView).ensure();
        typeof (Bedit.FileDialogListView).ensure();
        typeof (Bedit.FileDialogIconView).ensure();
        typeof (Bedit.FileDialogTreeView).ensure();
    }

    construct {
        this.views_init();
        this.filter_view_init();
        this.list_view_init();
        this.icon_view_init();
        this.tree_view_init();

        this.insert_action_group("dialog", this.dialog_actions);
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogWindow));
        base.dispose();
    }
}

sealed class Bedit.FileDialog : GLib.Object {
    public string title { get; set; }

    public GLib.File initial_file { get; set; }
    public GLib.File initial_folder { get; set; }
    public string initial_name { get; set; }

    public Gtk.FileFilter default_filter { get; set; }
    public GLib.ListModel filters { get; set; }

    public string accept_label { get; set; }

    public async GLib.File?
    open(Gtk.Window? parent, GLib.Cancellable cancellable) throws Error {
        var window = new Bedit.FileDialogWindow();
        window.set_transient_for(parent);

        window.root_directory =  GLib.File.new_for_path("/home/ben/pro");
        window.view_mode = LIST;
        window.sort_columns = {};
        window.expanded = {};

        GLib.File? result = null;
        bool done = false;

        cancellable.connect((c) => {
            if (!done) {
                done = true;
                this.open.callback();
            }
        });
        window.open.connect((file) => {
            result = file;
            if (!done) {
                done = true;
                this.open.callback();
            }
        });
        window.unmap.connect((w) => {
            if (!done) {
                done = true;
                this.open.callback();
            }
        });
        window.present();
        yield;
        window.close();

        if (cancellable.is_cancelled()) {
            throw new GLib.IOError.CANCELLED("open cancelled");
        }

        return result;
    }
}
