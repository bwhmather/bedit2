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


private sealed class Bedit.FileDialogPathBar : Gtk.Widget {
    public GLib.File root_directory { get; set; }

    class construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
    }

    public override void
    dispose() {
        base.dispose();
    }
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog.ui")]
private sealed class Bedit.FileDialogWindow : Gtk.Window {
    // Path to root folder under mount.

    public GLib.File root_directory { get; set; }
    public Gtk.DirectoryList directory_list;

    public signal void open(GLib.File result);

    public GLib.ListModel selection { get; set; default=new GLib.ListStore(typeof (GLib.File)); }

    public GLib.SimpleActionGroup dialog_actions = new GLib.SimpleActionGroup();

    /* === Views ========================================================================================== */

    [GtkChild]
    private unowned Gtk.Entry filter_entry;

    [GtkChild]
    private unowned Gtk.Entry location_entry;

    [GtkChild]
    private unowned Bedit.FileDialogPathBar path_bar;

    [GtkChild]
    private unowned Brk.ButtonGroup view_button_group;

    [GtkChild]
    private unowned Gtk.Stack view_stack;

    public bool filter_view_enabled { get; set; }
    public bool edit_location_enabled { get; set; }
    // This is the view that should be shown when filter mode is not enabled.
    public Bedit.FileDialogViewMode view_mode { get; set; default = LIST; }

    public bool show_binary { get; set; }
    public bool show_hidden { get; set; }

    private void
    view_stack_update_visible_child() {
        if (this.filter_view_enabled) {
            this.filter_entry.visible = true;
            this.location_entry.visible = false;
            this.path_bar.visible = false;
            this.view_button_group.visible = false;

            this.view_stack.visible_child = this.filter_view;
            return;
        }

        this.filter_entry.visible = false;
        this.view_button_group.visible = true;
        if (this.edit_location_enabled) {
            this.location_entry.visible = true;
            this.path_bar.visible = false;
        } else {
            this.location_entry.visible = false;
            this.path_bar.visible = true;
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
    action_open() {
        this.open(this.selection.get_item(0) as GLib.File);
    }

    private void
    views_init() {
        this.bind_property("root-directory", this.path_bar, "root-directory", BIDIRECTIONAL | SYNC_CREATE);

        this.dialog_actions.add_action(new GLib.PropertyAction("filter", this, "filter-view-enabled"));

        this.dialog_actions.add_action(new GLib.PropertyAction("view-mode", this, "view-mode"));

        this.dialog_actions.add_action(new GLib.PropertyAction("show-binary", this, "show-binary"));
        this.dialog_actions.add_action(new GLib.PropertyAction("show-hidden", this, "show-hidden"));

        var open_action = new GLib.SimpleAction("open", null);
        open_action.activate.connect(this.action_open);
        this.dialog_actions.add_action(open_action);
        this.notify["selection"].connect((d, pspec) => {
            open_action.set_enabled(this.selection.get_n_items() > 0);
        });
        open_action.set_enabled(this.selection.get_n_items() > 0);

        this.notify["filter-view-enabled"].connect(this.view_stack_update_visible_child);
        this.notify["view-mode"].connect(this.view_stack_update_visible_child);
        this.view_stack_update_visible_child();
    }

    /* --- Filter View ------------------------------------------------------------------------------------ */


    [GtkChild]
    private unowned Bedit.FileDialogFilterView filter_view;

    private void
    filter_view_init() {
        this.bind_property("root-directory", this.filter_view, "root-directory", SYNC_CREATE);
    }

    /* --- List View -------------------------------------------------------------------------------------- */

    public string[] sort_columns;

    [GtkChild]
    private unowned Bedit.FileDialogListView list_view;

    private void
    list_view_init() {
        this.list_view.directory_list = this.directory_list;

        this.notify["view-mode"].connect(() => {
            if (this.view_mode == LIST) {
                // Apply current selection to list view when switching from a different view.
                this.list_view.selection = this.selection;
            }
        });
        this.notify["filter-view-enabled"].connect((v, pspec) => {
            if (this.view_mode == LIST && !this.filter_view_enabled) {
                // Resync selection to match list view when coming out of filter mode.
                this.selection = this.list_view.selection;
            }
        });
        this.list_view.notify["selection"].connect((v, pspec) => {
            if (this.view_mode == LIST && !this.filter_view_enabled) {
                // Sync selection to match list view when list view visible.
                this.selection = this.list_view.selection;
            }
        });
    }

    /* --- Icon View -------------------------------------------------------------------------------------- */

    [GtkChild]
    private unowned Bedit.FileDialogIconView icon_view;

    private void
    icon_view_init() {
        this.icon_view.directory_list = this.directory_list;
    }

    /* --- Tree View -------------------------------------------------------------------------------------- */

    // Sorted list of expanded directories under the current mount.
    public string[] expanded;

    [GtkChild]
    private unowned Bedit.FileDialogTreeView tree_view;

    private void
    tree_view_init() {
        this.tree_view.directory_list = this.directory_list;
    }

    /* === Lifecycle ======================================================================================= */

    class construct {
        typeof (Bedit.FileDialogPathBar).ensure();
        typeof (Bedit.FileDialogFilterView).ensure();
        typeof (Bedit.FileDialogListView).ensure();
        typeof (Bedit.FileDialogIconView).ensure();
        typeof (Bedit.FileDialogTreeView).ensure();
    }

    construct {
        this.directory_list = new Gtk.DirectoryList(
            "standard::display-name,standard::size,time::modified,standard::type",
            this.root_directory
        );
        directory_list.monitored = true;
        this.bind_property("root-directory", this.directory_list, "file", SYNC_CREATE);

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

        window.root_directory =  GLib.File.new_for_path("/usr/include");
        window.view_mode = LIST;
        window.sort_columns = {};
        window.expanded = {};

        GLib.File? result = null;
        bool done = false;

        cancellable.connect((c) => {
            if (!done) {
                done = true;
                window.close();
                this.open.callback();
            }
        });
        window.open.connect((file) => {
            result = file;
            if (!done) {
                done = true;
                window.close();
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

        if (cancellable.is_cancelled()) {
            throw new GLib.IOError.CANCELLED("open cancelled");
        }

        return result;
    }
}
