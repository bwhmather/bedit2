
[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {

    [GtkChild]
    private unowned GtkSource.View source_view;
    private unowned GtkSource.Buffer source_buffer;
    private GtkSource.File source_file;

    public string title { get; private set; }
    public unowned GLib.File? file { get; construct; }
    public bool modified { get; }

    public bool loading { get; private set; }
    public bool saving { get; private set; }

    public bool can_undo { get { return this.source_buffer.can_undo; } }
    public bool can_redo { get { return this.source_buffer.can_redo; } }

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.source_buffer = source_view.get_buffer() as GtkSource.Buffer;
        this.source_buffer.notify["can-undo"].connect((sb, pspec) => {
            this.notify_property("can-undo");
        });
        this.source_buffer.notify["can-redo"].connect((sb, pspec) => {
            this.notify_property("can-redo");
        });

        this.source_file = new GtkSource.File();
        this.source_file.set_location(file);
        this.source_file.notify["location"].connect((sf, pspec) => {
            this.file = this.source_file.location;
            this.title = file.get_basename();
        });

        if (file != null) {
            reload_async.begin(null);
        }
    }

    public Document.for_file(GLib.File file) {
        Object(file: file);
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
    save(GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        saving = true;

        var source_saver = new GtkSource.FileSaver(source_buffer, source_file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, cancellable, null);

        saving = false;
        return true;
    }

    public async bool
    save_as(GLib.File file, GLib.Cancellable cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        saving = true;

        var source_saver = new GtkSource.FileSaver.with_target(source_buffer, source_file, file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, cancellable, null);

        saving = false;
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
