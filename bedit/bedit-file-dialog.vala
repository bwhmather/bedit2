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

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-filter-view.ui")]
private sealed class Bedit.FileDialogFilterView : Gtk.Widget {

    /* === State ========================================================================================== */

    public GLib.File root_directory { get; set; }

    /* === Lifecycle ====================================================================================== */

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogFilterView));
        base.dispose();
    }
}


[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-list-view.ui")]
private sealed class Bedit.FileDialogListView : Gtk.Widget {

    /* === State ========================================================================================== */

    /* --- Directory State -------------------------------------------------------------------------------- */

    private Gtk.DirectoryList _directory_list;
    public Gtk.DirectoryList directory_list {
        get {
            if (this._directory_list == null) {
                this._directory_list = new Gtk.DirectoryList(
                    "standard::display-name,standard::size,time::modified,standard::type",
                    null
                );
            }
            return this._directory_list;
        }
        set {
            if (value == this._directory_list) {
                return;
            }
            if (this._directory_list != null) {
                GLib.SignalHandler.disconnect_by_data(this._directory_list, this);
            }
            this._directory_list = value;
        }
    }

    /* --- Selection -------------------------------------------------------------------------------------- */

    private Gtk.MultiSelection selection_model = new Gtk.MultiSelection(null);
    // Files that should be in the current selection but haven't been loaded into the directory list yet.
    private GLib.HashTable<GLib.File, void *> pending_selection = new GLib.HashTable<GLib.File, void *>(GLib.File.hash, GLib.File.equal);

    public GLib.ListModel selection {
        owned get {
            var list_store = new GLib.ListStore(typeof(GLib.File));
            for (var i = 0; i < this.directory_list.n_items; i++) {
                if (this.selection_model.is_selected(i)) {
                    var fileinfo = this.directory_list.get_item(i) as GLib.FileInfo;
                    var file = fileinfo.get_attribute_object("standard::file") as GLib.File;
                    list_store.append(file);
                }
            }
            pending_selection.foreach((file, _) => {
                list_store.append(file);
            });
            return (owned) list_store;
        }
        set {
            this.pending_selection.remove_all();
            var root_directory = this.directory_list.file;
            for (var i = 0; i < (value != null? value.get_n_items() : 0); i++) {
                var file = value.get_item(i) as GLib.File;
                if (!file.has_parent(root_directory)) {
                    // File not visible in current state of view.  Only safe thing to do is to clear the
                    // entire selection.  Silently dropping just some files from the selection or worse
                    // leaving invisible files selected is not acceptable.
                    this.pending_selection.remove_all();
                    break;
                }
                this.pending_selection[file] = null;
            }
            var selected = new Gtk.Bitset.empty();
            var mask = new Gtk.Bitset.range(0, this.directory_list.n_items);
            for (var i = 0; i < this.directory_list.n_items; i++) {
                var fileinfo = this.directory_list.get_item(i) as GLib.FileInfo;
                var file = fileinfo.get_attribute_object("standard::file") as GLib.File;
                if (this.pending_selection.steal(file)) {
                    selected.add(i);
                }
            }
            this.selection_model.set_selection(selected, mask);
        }
    }

    private void
    selection_init() {
        this.notify["directory-list"].connect((lv, pspec) => {
            this.selection_model.model = this.directory_list;

            // This binding requires that the selection model is updated first.  Do not reorder.
            this.directory_list.items_changed.connect((dl, position, removed, added) => {
                // Check if any of the newly added items is in the pending selection and should be selected.
                var selected = new Gtk.Bitset.empty();
                var mask = new Gtk.Bitset.range(position, added);
                for (var i = position; i < position + added; i++) {
                    var fileinfo = this.directory_list.get_item(i) as GLib.FileInfo;
                    var file = fileinfo.get_attribute_object("standard::file") as GLib.File;
                    if (this.pending_selection.steal(file)) {
                        selected.add(i);
                    }
                }
                this.selection_model.set_selection(selected, mask);
            });

            this.directory_list.notify["loading"].connect((dl, pspec) => {
                if (!this.directory_list.loading) {
                    // All files that actually exist in the directory should now also be in the directory list
                    // model.  Any files in the selection that aren't in the directory list model don't exist
                    // anymore and should be removed.
                    this.pending_selection.remove_all();
                }
            });
        });

        this.selection_model.selection_changed.connect((sm, p, n_items) => {
            this.notify_property("selection");
        });
    }

    /* === View =========================================================================================== */

    [GtkChild]
    private unowned Gtk.ColumnView column_view;

    [GtkChild]
    private unowned Gtk.ColumnViewColumn name_column;

    [GtkChild]
    private unowned Gtk.ColumnViewColumn size_column;

    private void
    view_init() {
        this.column_view.model = this.selection_model;

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

    /* === Lifecycle ====================================================================================== */

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.selection_init();
        this.view_init();
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogListView));
        base.dispose();
    }
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-icon-view.ui")]
private sealed class Bedit.FileDialogIconView : Gtk.Widget {

    /* === State ========================================================================================== */

    /* --- Directory State -------------------------------------------------------------------------------- */

    private Gtk.DirectoryList _directory_list;
    public Gtk.DirectoryList directory_list {
        get {
            if (this._directory_list == null) {
                this._directory_list = new Gtk.DirectoryList(
                    "standard::display-name,standard::size,time::modified,standard::type",
                    null
                );
            }
            return this._directory_list;
        }
        set {
            if (value == this._directory_list) {
                return;
            }
            if (this._directory_list != null) {
                GLib.SignalHandler.disconnect_by_data(this._directory_list, this);
            }
            this._directory_list = value;
        }
    }

    /* === Lifecycle ====================================================================================== */

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogIconView));
        base.dispose();
    }
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-tree-view.ui")]
private sealed class Bedit.FileDialogTreeView : Gtk.Widget {

    /* === State ========================================================================================== */

    /* --- Directory State -------------------------------------------------------------------------------- */

    private Gtk.DirectoryList _directory_list;
    public Gtk.DirectoryList directory_list {
        get {
            if (this._directory_list == null) {
                this._directory_list = new Gtk.DirectoryList(
                    "standard::display-name,standard::size,time::modified,standard::type",
                    null
                );
            }
            return this._directory_list;
        }
        set {
            if (value == this._directory_list) {
                return;
            }
            if (this._directory_list != null) {
                GLib.SignalHandler.disconnect_by_data(this._directory_list, this);
            }
            this._directory_list = value;
        }
    }

    /* === View =========================================================================================== */

    [GtkChild]
    private unowned Gtk.ListView list_view;

    private void
    view_init() {
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
            this.directory_list,
            false,  // passthrough
            false,  // autoexpand
            (item) => {
                return null;
            }
        );

        this.list_view.model = new Gtk.MultiSelection(tree_list_model);
    }

    /* === Lifecycle ====================================================================================== */

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        view_init();
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
    public Gtk.DirectoryList directory_list;

    public signal void open(GLib.File result);

    private GLib.ListModel selection { get; set; }

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
    views_init() {
        this.bind_property("root-directory", this.path_bar, "root-directory", BIDIRECTIONAL | SYNC_CREATE);

        this.dialog_actions.add_action(new GLib.PropertyAction("filter", this, "filter-view-enabled"));

        this.dialog_actions.add_action(new GLib.PropertyAction("view-mode", this, "view-mode"));

        this.dialog_actions.add_action(new GLib.PropertyAction("show-binary", this, "show-binary"));
        this.dialog_actions.add_action(new GLib.PropertyAction("show-hidden", this, "show-hidden"));


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
                this.list_view.selection = this.selection;
            }
        });
        this.list_view.notify["selection"].connect((v, pspec) => {
            if (this.view_mode == LIST) {
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

        window.root_directory =  GLib.File.new_for_path("/usr/lib");
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
