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
