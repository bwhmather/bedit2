/* Copyright 2025 Ben Mather <bwhmather@bwhmather.com>
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

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-preferences-window.ui")]
public sealed class Bedit.PreferencesWindow : Brk.PreferencesWindow {
    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit");

    [GtkChild]
    private unowned Brk.SwitchRow show_line_numbers_row;
    [GtkChild]
    private unowned Brk.SwitchRow show_right_margin_row;
    [GtkChild]
    private unowned Brk.SpinRow right_margin_position_row;
    [GtkChild]
    private unowned Brk.SwitchRow show_overview_map_row;
    [GtkChild]
    private unowned Brk.SwitchRow show_toolbar_row;
    [GtkChild]
    private unowned Brk.SwitchRow show_menubar_row;
    [GtkChild]
    private unowned Brk.SwitchRow show_statusbar_row;
    [GtkChild]
    private unowned Brk.SwitchRow highlight_selection_row;
    [GtkChild]
    private unowned Brk.SwitchRow highlight_current_line_row;
    [GtkChild]
    private unowned Brk.SwitchRow highlight_syntax_row;
    [GtkChild]
    private unowned Brk.SwitchRow insert_spaces_instead_of_tabs_row;
    [GtkChild]
    private unowned Brk.SpinRow tab_width_row;
    [GtkChild]
    private unowned Brk.SwitchRow auto_indent_row;
    [GtkChild]
    private unowned Brk.SwitchRow trim_trailing_whitespace_row;
    [GtkChild]
    private unowned Brk.SwitchRow word_wrap_row;
    [GtkChild]
    private unowned Brk.SwitchRow auto_check_spelling_row;
    [GtkChild]
    private unowned Brk.SwitchRow use_default_font_row;
    [GtkChild]
    private unowned Brk.FontRow editor_font_row;

    construct {
        settings.bind("show-line-numbers", show_line_numbers_row, "active", DEFAULT);
        settings.bind("show-right-margin", show_right_margin_row, "active", DEFAULT);
        this.right_margin_position_row.configure(new Gtk.Adjustment(80, 1, 999, 1, 10, 0), 1, 0);
        settings.bind("right-margin-position", this.right_margin_position_row, "value", DEFAULT);
        settings.bind("show-right-margin", this.right_margin_position_row, "visible", GET);
        settings.bind("show-overview-map", show_overview_map_row, "active", DEFAULT);
        settings.bind("show-toolbar", show_toolbar_row, "active", DEFAULT);
        settings.bind("show-menubar", show_menubar_row, "active", DEFAULT);
        settings.bind("show-statusbar", show_statusbar_row, "active", DEFAULT);
        settings.bind("highlight-selection", highlight_selection_row, "active", DEFAULT);
        settings.bind("highlight-current-line", highlight_current_line_row, "active", DEFAULT);
        settings.bind("highlight-syntax", highlight_syntax_row, "active", DEFAULT);
        settings.bind("insert-spaces-instead-of-tabs", insert_spaces_instead_of_tabs_row, "active", DEFAULT);
        this.tab_width_row.configure(new Gtk.Adjustment(4, 1, 32, 1, 4, 0), 1, 0);
        settings.bind("tab-width", this.tab_width_row, "value", DEFAULT);
        settings.bind("auto-indent", auto_indent_row, "active", DEFAULT);
        settings.bind("trim-trailing-whitespace", trim_trailing_whitespace_row, "active", DEFAULT);
        settings.bind("word-wrap", word_wrap_row, "active", DEFAULT);
        settings.bind("auto-check-spelling", auto_check_spelling_row, "active", DEFAULT);
        settings.bind("use-default-font", use_default_font_row, "active", DEFAULT);
        settings.bind("editor-font", editor_font_row, "font", DEFAULT);
        settings.bind("use-default-font", editor_font_row, "visible", GET | INVERT_BOOLEAN);
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.PreferencesWindow));
        base.dispose();
    }

    public PreferencesWindow(Gtk.Application application) {
        Object(application: application);
    }
}
