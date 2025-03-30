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
public sealed class Bedit.PreferencesWindow : Gtk.Window {
//    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit2");


    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.PreferencesWindow));
    }

    public PreferencesWindow(Gtk.Application application) {
        Object(application: application);
    }
}
