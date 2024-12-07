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
