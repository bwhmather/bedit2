/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-list-view.ui")]
internal sealed class Bedit.FileDialogListView : Gtk.Widget {

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

