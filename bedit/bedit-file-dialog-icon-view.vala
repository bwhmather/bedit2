/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-icon-view.ui")]
internal sealed class Bedit.FileDialogIconView : Gtk.Widget {

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
