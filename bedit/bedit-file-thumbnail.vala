/*
 * Copyright (c) 2026 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

private sealed class Bedit.FileThumbnail : Gtk.Widget {
    public GLib.FileInfo? fileinfo { get; set; }

    private Gtk.Image thumbnail;
    private Gtk.Label label;

    class construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
    }

    construct {
        this.thumbnail = new Gtk.Image();
        thumbnail.set_parent(this);

        this.label = new Gtk.Label("");
        label.halign = START;
        label.insert_after(this, this.thumbnail);

        this.notify["fileinfo"].connect(() => {
            if (this.fileinfo == null) {
                return;
            }

            var icon_theme = Gtk.IconTheme.get_for_display(this.get_display());
            var icon = this.fileinfo.get_icon();
            if (icon == null || !icon_theme.has_gicon(icon)) {
              icon = new GLib.ThemedIcon("text-x-generic");
            }
            this.thumbnail.set_from_gicon(icon);

            if (this.fileinfo.has_attribute("bedit::markup")) {
                this.label.set_markup(this.fileinfo.get_attribute_string("bedit::markup"));
            } else {
                this.label.set_text(this.fileinfo.get_attribute_string("standard::display-name"));
            }
        });
    }

    public override void
    dispose() {
        this.thumbnail.unparent();
        this.label.unparent();
        base.dispose();
    }
}
