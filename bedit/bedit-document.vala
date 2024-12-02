[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {
    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned GtkSource.View source_view;
    private unowned GtkSource.Buffer source_buffer;
    private GtkSource.File source_file;

    public string title { get; private set; }
    public unowned GLib.File? file { get; construct; }
    public bool modified { get; }

    public bool loading { get; private set; }
    public bool saving { get; private set; }

    public signal void closed();

    public bool can_undo { get; private set; }
    public bool can_redo { get; private set; }

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.source_buffer = source_view.get_buffer() as GtkSource.Buffer;
        this.source_buffer.notify["can-undo"].connect((sb, pspec) => {
            this.can_undo = source_buffer.can_undo;
        });
        this.source_buffer.notify["can-redo"].connect((sb, pspec) => {
            this.can_redo = source_buffer.can_redo;
        });

        this.source_file = new GtkSource.File();
        this.source_file.set_location(file);
        this.source_file.notify["location"].connect((sf, pspec) => {
            this.file = this.source_file.location;
            this.title = this.file.get_basename();
        });

        if (file != null) {
            reload_async.begin(null);
        }
    }

    public Document.for_file(GLib.File file) {
        Object(file: file);
    }

    public async void
    save_async() throws Error {
        return_val_if_fail(this.file is GLib.File, false);
        return_val_if_fail(!this.loading, false);
        return_val_if_fail(!this.saving, false);

        saving = true;

        var source_saver = new GtkSource.FileSaver(this.source_buffer, this.source_file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, this.cancellable, null);

        saving = false;
    }

    public async void
    save_as_async() throws Error {
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        saving = true;

        var file_dialog = new Gtk.FileDialog();
        var file = yield file_dialog.save(this.root as Gtk.Window, null);
        return_val_if_fail(file is GLib.File, false);  // TODO

        var source_saver = new GtkSource.FileSaver.with_target(this.source_buffer, this.source_file, file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, this.cancellable, null);

        saving = false;
    }

    public async bool
    reload_async(GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        loading = true;

        var source_loader = new GtkSource.FileLoader(source_buffer, source_file);
        yield source_loader.load_async(Priority.LOW, cancellable, null);

        loading = false;
        return true;
    }

    public async bool
    request_close_async() throws Error {
        var window = this.root as Gtk.Window;

        var save_changes_dialog = new Bedit.CloseConfirmationDialog(window.application);
        save_changes_dialog.set_transient_for(window);
        save_changes_dialog.present();


        while (this.saving) {
            this.notify["saving"].connect((d, pspec) => { request_close_async.callback(); });
            yield;
        }

        this.closed();

        return true;
    }

    public void
    undo() {
        this.source_buffer.undo();
    }

    public void
    redo() {
        this.source_buffer.redo();
    }
}
