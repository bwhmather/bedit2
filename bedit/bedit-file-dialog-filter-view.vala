/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-filter-view.ui")]
internal sealed class Bedit.FileDialogFilterView : Gtk.Widget {

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

