/* Copyright 2026 Ben Mather <bwhmather@bwhmather.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

public sealed class Bedit.StyleSchemeChooserRow : Brk.PreferencesRow {
    private Gtk.ListBox list_box;

    public string scheme_id { get; set; default = ""; }
    public string subtitle { get; set; default = ""; }

    private void
    on_color_scheme_changed() {
        this.list_box.invalidate_filter();
    }

    private void
    update_selected() {
        this.list_box.unselect_all();
        for (
            var child = this.list_box.get_first_child();
            child != null;
            child = child.get_next_sibling()
        ) {
            var row = child as Gtk.ListBoxRow;
            if (row == null) {
                continue;
            }
            var preview = row.get_data<GtkSource.StyleSchemePreview>("preview");
            var scheme = preview.scheme;
            var canonical_id = scheme.get_metadata("light-variant") ?? scheme.get_id();
            bool selected = canonical_id == this.scheme_id;
            preview.selected = selected;
            if (selected) {
                this.list_box.select_row(row);
            }
        }
    }

    construct {
        var gtk_settings = Gtk.Settings.get_for_display(this.get_display());

        this.list_box = new Gtk.ListBox();
        this.list_box.add_css_class("rich-list");
        this.list_box.selection_mode = SINGLE;
        this.list_box.activate_on_single_click = true;
        this.list_box.set_filter_func((row) => {
            var variant = row.get_data<GtkSource.StyleSchemePreview>("preview").scheme.get_metadata("variant");
            var dark = gtk_settings.gtk_interface_color_scheme == Gtk.InterfaceColorScheme.DARK;
            return variant == (dark ? "dark" : "light") || variant == null;
        });
        this.list_box.row_activated.connect((row) => {
            var scheme = row.get_data<GtkSource.StyleSchemePreview>("preview").scheme;
            this.scheme_id = scheme.get_metadata("light-variant") ?? scheme.get_id();
        });

        var scheme_manager = GtkSource.StyleSchemeManager.get_default();
        foreach (var scheme_id in scheme_manager.get_scheme_ids()) {
            var scheme = scheme_manager.get_scheme(scheme_id);

            var name_label = new Gtk.Label(scheme.get_name());
            name_label.xalign = 0;
            name_label.add_css_class("title");

            var desc_label = new Gtk.Label(scheme.get_description());
            desc_label.xalign = 0;
            desc_label.wrap = true;
            desc_label.wrap_mode = WORD_CHAR;
            desc_label.add_css_class("subtitle");

            var title_box = new Gtk.Box(VERTICAL, 0);
            title_box.hexpand = true;
            title_box.valign = CENTER;
            title_box.add_css_class("title");
            title_box.append(name_label);
            title_box.append(desc_label);

            var preview = new GtkSource.StyleSchemePreview(scheme);
            preview.valign = CENTER;

            var row_box = new Gtk.Box(HORIZONTAL, 0);
            row_box.valign = CENTER;
            row_box.add_css_class("header");
            row_box.append(title_box);
            row_box.append(preview);

            var row = new Gtk.ListBoxRow();
            row.set_data<GtkSource.StyleSchemePreview>("preview", preview);
            row.set_child(row_box);
            this.list_box.append(row);
        }

        var scrolled = new Gtk.ScrolledWindow();
        scrolled.hscrollbar_policy = NEVER;
        scrolled.vscrollbar_policy = ALWAYS;
        scrolled.height_request = 300;
        scrolled.set_child(this.list_box);

        var title_label = new Gtk.Label(null);
        title_label.xalign = 0;
        title_label.add_css_class("title");
        this.bind_property("title", title_label, "label", SYNC_CREATE);

        var subtitle_label = new Gtk.Label(null);
        subtitle_label.xalign = 0;
        subtitle_label.wrap = true;
        subtitle_label.wrap_mode = WORD_CHAR;
        subtitle_label.add_css_class("subtitle");
        this.bind_property("subtitle", subtitle_label, "label", SYNC_CREATE);
        this.bind_property("subtitle", subtitle_label, "visible", SYNC_CREATE,
            (_, src, ref dst) => { dst = src.get_string() != ""; return true; });

        var title_box = new Gtk.Box(VERTICAL, 0);
        title_box.add_css_class("title");
        title_box.append(title_label);
        title_box.append(subtitle_label);

        var header_box = new Gtk.Box(HORIZONTAL, 0);
        header_box.add_css_class("header");
        header_box.append(title_box);

        var frame = new Gtk.Frame(null);
        frame.set_child(scrolled);

        var box = new Gtk.Box(VERTICAL, 0);
        box.append(header_box);
        box.append(frame);

        this.set_child(box);

        gtk_settings.notify["gtk-interface-color-scheme"].connect(this.on_color_scheme_changed);

        this.notify["scheme-id"].connect(() => { this.update_selected(); });
        this.update_selected();
    }

    public override void
    dispose() {
        var gtk_settings = Gtk.Settings.get_for_display(this.get_display());
        GLib.SignalHandler.disconnect_by_data(gtk_settings, this);
        base.dispose();
    }
}
