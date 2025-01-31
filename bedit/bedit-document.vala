[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {
    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned GtkSource.View source_view;
    private unowned GtkSource.Buffer source_buffer;
    private GtkSource.File source_file;

    public string title { get; private set; }
    public unowned GLib.File? file { get; construct; }
    public bool modified { get; private set; }

    public bool loading { get; private set; }
    public bool saving { get; private set; }
    public bool busy { get; private set; }

    public signal void closed();

    public bool can_undo { get; private set; }
    public bool can_redo { get; private set; }
    public bool can_cut { get; private set; }
    public bool can_copy { get; private set; }
    public bool can_paste { get; private set; default = true; }

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    private void
    update_busy() {
        this.busy = this.loading || this.saving;
    }

    construct {
        this.source_buffer = source_view.get_buffer() as GtkSource.Buffer;
        this.source_buffer.notify["can-undo"].connect((sb, pspec) => {
            this.can_undo = source_buffer.can_undo;
        });
        this.source_buffer.notify["can-redo"].connect((sb, pspec) => {
            this.can_redo = source_buffer.can_redo;
        });
        this.source_buffer.notify["has-selection"].connect((db, pspec) => {
            bool has_selection = this.source_buffer.has_selection;
            this.can_cut = has_selection;
            this.can_copy = has_selection;
        });
        this.source_buffer.modified_changed.connect((tb) => {
            this.modified = this.source_buffer.get_modified();
        });

        this.source_file = new GtkSource.File();
        this.source_file.set_location(file);
        this.source_file.notify["location"].connect((sf, pspec) => {
            this.file = this.source_file.location;
            this.title = this.file.get_basename();
        });

        this.notify["loading"].connect((_, pspec) => { this.update_busy(); });
        this.notify["saving"].connect((_, pspec) => { this.update_busy(); });

        if (file != null) {
            reload_async.begin(null);
        }
    }

    ~Document() {
        assert(!this.busy);
    }

    public Document.for_file(GLib.File file) {
        Object(file: file);
    }

    public async void
    save_async(GLib.File file) throws Error {
        return_val_if_fail(!loading, false);
        return_val_if_fail(!saving, false);

        saving = true;

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

    public void
    undo() {
        this.source_buffer.undo();
    }

    public void
    redo() {
        this.source_buffer.redo();
    }

    public void
    cut() {
        var clipboard = this.get_display().get_clipboard();
        this.source_buffer.cut_clipboard(clipboard, true);
    }

    public void
    copy() {
        var clipboard = this.get_display().get_clipboard();
        this.source_buffer.copy_clipboard(clipboard);
    }

    public void
    paste() {
        var clipboard = this.get_display().get_clipboard();
        this.source_buffer.paste_clipboard(clipboard, null, true);
    }

    public void
    select_all() {
        Gtk.TextIter start;
        Gtk.TextIter end;
        this.source_buffer.get_bounds(out start, out end);
        this.source_buffer.select_range(start, end);
    }
}
