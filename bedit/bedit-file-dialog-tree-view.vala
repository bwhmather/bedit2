/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-tree-view.ui")]
internal sealed class Bedit.FileDialogTreeView : Gtk.Widget {

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

