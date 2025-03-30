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

public class Bedit.Toolbar : Gtk.Widget {
    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("toolbar");
        set_accessible_role(GROUP);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.HORIZONTAL, -1);
        this.add_css_class("toolbar");
    }

    public void
    append(Gtk.Widget child) {
        return_if_fail(child.parent == null);
        this.insert_before(child, null);
    }

    public void
    prepend(Gtk.Widget child) {
        return_if_fail(child.parent == null);
        this.insert_after(child, null);
    }

    public void
    remove(Gtk.Widget child) {
        return_if_fail(child.parent != this);
        child.unparent();
    }
}
