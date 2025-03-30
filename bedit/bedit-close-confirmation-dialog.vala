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

public enum Bedit.CloseAction {
    CANCEL,
    SAVE,
    DISCARD,
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-close-confirmation-dialog.ui")]
public sealed class Bedit.CloseConfirmationDialog : Gtk.Window {
    public signal void save();
    public signal void discard();

    public Bedit.Document document { get; construct; }

    private void
    action_cancel(Gtk.Widget widget, string? action_name, GLib.Variant? parameter) {
        this.close();
    }

    private void
    action_discard(Gtk.Widget widget, string? action_name, GLib.Variant? parameter) {
        this.discard();
        this.close();
    }

    private void
    action_save(Gtk.Widget widget, string? action_name, GLib.Variant? parameter) {
        this.save();
        this.close();
    }

    static construct {
        install_action("cancel", null, (Gtk.WidgetActionActivateFunc) action_cancel);
        install_action("discard", null, (Gtk.WidgetActionActivateFunc) action_discard);
        install_action("save", null, (Gtk.WidgetActionActivateFunc) action_save);

        set_accessible_role(DIALOG);
    }

    public CloseConfirmationDialog(Gtk.Window window, Bedit.Document document) {
        Object(
            transient_for: window,
            document: document
        );
    }

    construct {
        this.transient_for.get_group().add_window(this);
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.CloseConfirmationDialog));
    }

    public static async Bedit.CloseAction
    run_async(GLib.Cancellable? cancellable, Gtk.Window window, Bedit.Document document) throws Error {
        int status = Bedit.CloseAction.CANCEL;

        var dialog = new Bedit.CloseConfirmationDialog(window, document);

        dialog.save.connect((_) => {
            status =  Bedit.CloseAction.SAVE;
            dialog.close();
        });

        dialog.discard.connect((_) => {
            status =  Bedit.CloseAction.DISCARD;
            dialog.close();
        });

        cancellable.connect((_) => {
            dialog.close();
        });

        dialog.unmap.connect((_) => {
            run_async.callback();
        });
        dialog.present();
        yield;


        return status;
    }
}
