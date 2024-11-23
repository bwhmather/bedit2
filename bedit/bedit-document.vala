
[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {

    [GtkChild]
    private unowned GtkSource.View source_view;
    private unowned GtkSource.Buffer source_buffer = null;
    private GtkSource.File source_file;

    public string title { get; private set; }
    public unowned GLib.File? file { get; construct; }
    public bool modified { get; }

    public bool loading { get; private set; }
    public bool saving { get; private set; }
    public bool busy { get { return loading || saving; } }

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        source_buffer = source_view.get_buffer() as GtkSource.Buffer;

        source_file = new GtkSource.File();
        source_file.set_location(file);
        source_file.notify["location"].connect((s, b) => {
            file = source_file.location;
            title = file.get_basename();
        });
    }

    public override void constructed() {
        reload_async.begin(null);
    }

    public Document.for_file(GLib.File file) {
        Object(file: file);
    }

    public async bool reload_async(GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!busy, false);

        loading = true;
        notify_property("busy");

        var source_loader = new GtkSource.FileLoader(source_buffer, source_file);
        yield source_loader.load_async(Priority.LOW, cancellable, null);

        loading = false;
        notify_property("busy");

        return true;
    }

    public async bool save_async(GLib.Cancellable? cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!busy, false);

        saving = true;
        notify_property("busy");

        var source_saver = new GtkSource.FileSaver(source_buffer, source_file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, cancellable, null);

        saving = false;
        notify_property("busy");

        return true;
    }

    public async bool save_as_async(GLib.File file, GLib.Cancellable cancellable) throws Error {
        return_val_if_fail(file is GLib.File, false);
        return_val_if_fail(!busy, false);

        saving = true;
        notify_property("busy");

        var source_saver = new GtkSource.FileSaver.with_target(source_buffer, source_file, file);
        source_saver.flags = IGNORE_INVALID_CHARS | IGNORE_MODIFICATION_TIME;

        yield source_saver.save_async(Priority.DEFAULT, cancellable, null);

        saving = false;
        notify_property("busy");

        return true;
    }
}
