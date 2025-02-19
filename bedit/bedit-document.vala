[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {
    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit2");

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

        word_wrap_init();
        overview_map_init();
        search_init();

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
        this.source_buffer.cut_clipboard(clipboard, this.source_view.editable);
    }

    public void
    copy() {
        var clipboard = this.get_display().get_clipboard();
        this.source_buffer.copy_clipboard(clipboard);
    }

    public void
    paste() {
        var clipboard = this.get_display().get_clipboard();
        this.source_buffer.paste_clipboard(clipboard, null, this.source_view.editable);
    }

    public void
    select_all() {
        Gtk.TextIter start;
        Gtk.TextIter end;
        this.source_buffer.get_bounds(out start, out end);
        this.source_buffer.select_range(start, end);
    }

    public void
    sort_lines() {
        Gtk.TextIter start;
        Gtk.TextIter end;

        if (!this.source_buffer.get_selection_bounds(out start, out end)) {
            this.source_buffer.get_bounds(out start, out end);
        }
        this.source_buffer.sort_lines(start, end, NONE, 0);
    }

    public void
    delete_line() {
        Gtk.TextIter start;
        Gtk.TextIter end;
        if (!this.source_buffer.get_selection_bounds(out start, out end)) {
            var cursor = this.source_buffer.get_insert();
            this.source_buffer.get_iter_at_mark(out start, cursor);
            end = start;
        }
        start.order(ref end);

        start.set_line_offset(0);
        end.forward_lines(1);

        if (end.is_end()) {
            if (start.backward_line() && !start.ends_line()) {
                start.forward_to_line_end();
            }
        }

        if (!start.equal(end)){
            this.source_buffer.begin_user_action();
            this.source_buffer.delete_interactive(ref start, ref end, this.source_view.editable);
            this.source_buffer.end_user_action();

            this.source_view.scroll_mark_onscreen(this.source_buffer.get_insert());
        }
    }

    /* === Word Wrap ====================================================================================== */

    public bool word_wrap { get; set; }

    private void
    update_word_wrap() {
        if (this.word_wrap) {
            this.source_view.wrap_mode = WORD_CHAR;
        } else {
            this.source_view.wrap_mode = NONE;
        }
    }

    private void
    word_wrap_init() {
        this.settings.bind("word-wrap", this, "word-wrap", GET);
        this.notify["word-wrap"].connect((d, pspec) => { this.update_word_wrap(); });
        this.update_word_wrap();
    }

    /* === Overview Map =================================================================================== */

    [GtkChild]
    private unowned GtkSource.Map overview_map;

    [GtkChild]
    private unowned Gtk.ScrolledWindow scrolled_window;

    public bool show_overview_map { get; set; }

    private void
    update_show_overview_map() {
        if (this.show_overview_map) {
            this.overview_map.visible = true;
            this.scrolled_window.vscrollbar_policy = EXTERNAL;
        } else {
            this.overview_map.visible = false;
            this.scrolled_window.vscrollbar_policy = ALWAYS;
        }
    }

    private void
    overview_map_init() {
        this.settings.bind("show-overview-map", this, "show-overview-map", GET);
        this.notify["show-overview-map"].connect((d, pspec) => { this.update_show_overview_map(); });
        this.update_show_overview_map();
    }

    /* === Search and Replace ============================================================================= */

    private GtkSource.SearchContext? search_context;
    private GLib.Cancellable? search_cancellable;
    private Gtk.TextMark? search_start_mark;
    private bool search_start_mark_reset_blocked;

    public int num_search_occurrences { get; private set; }
    public int selected_search_occurrence { get; private set; }

    private void
    clear_search_start_mark() {
        if (search_start_mark != null) {
            this.source_buffer.delete_mark(this.search_start_mark);
            this.search_start_mark = null;
        }
    }

    private void
    set_search_start_mark() {
        return_if_fail(this.search_start_mark == null);

        Gtk.TextIter start_iter;
        this.source_buffer.get_selection_bounds(out start_iter, null);
        this.search_start_mark = this.source_buffer.create_mark(null, start_iter, false);
    }

    private void
    reset_search_start_mark() {
        if (!this.search_start_mark_reset_blocked) {
            this.clear_search_start_mark();
            this.set_search_start_mark();
        }
    }

    private void
    update_search_occurrences() {
        int count = -1;
        int position = -1;
        Gtk.TextIter selection_start;
        Gtk.TextIter selection_end;

        if (this.search_context != null) {
            count = this.search_context.get_occurrences_count();
            if (count == -1) {
                // Search not finished yet.  Leave previous state in place to
                // avoid flashing.
                return;
            }

            this.source_buffer.get_selection_bounds(out selection_start, out selection_end);
            position = this.search_context.get_occurrence_position(selection_start, selection_end);
        }

        this.freeze_notify();
        this.num_search_occurrences = count;
        this.selected_search_occurrence = position;
        this.thaw_notify();
    }

    public void
    find(string query, bool regex, bool case_sensitive) {
        if (this.search_context == null) {
            this.search_context = new GtkSource.SearchContext(this.source_buffer, null);
            this.search_context.notify["occurrences-count"].connect((sc, pspec) => {
                this.update_search_occurrences();
            });
        }

        var settings = this.search_context.settings;
        settings.search_text = query;
        settings.regex_enabled = regex;
        settings.case_sensitive = case_sensitive;
        settings.wrap_around = true;
    }

    private void
    scroll_to_cursor() {
        this.source_view.scroll_to_mark(this.source_buffer.get_insert(), 0.25, false, 0.0, 0.0);
    }

    private void
    wait_focus_first() {
        Gtk.TextIter start_at;
        Gtk.TextIter match_start;
        Gtk.TextIter match_end;
        bool found;

        if (this.search_cancellable == null) {
            return;
        }

        return_if_fail(this.search_context != null);
        return_if_fail(this.search_start_mark != null);

        // TODO it would be nice if there was a way to block on the current run
        // of bedit_searchbar_focus_first() instead of killing it and starting
        // again.
        this.search_cancellable.cancel();
        this.search_cancellable = null;

        this.source_buffer.get_iter_at_mark(out start_at, this.search_start_mark);

        found = this.search_context.forward(start_at, out match_start, out match_end, null);

        if (found) {
            this.search_start_mark_reset_blocked = true;
            this.source_buffer.select_range(match_start, match_end);
            this.search_start_mark_reset_blocked = false;

        } else {
            this.source_buffer.select_range(start_at, start_at);
        }

        this.scroll_to_cursor();
    }

    private async void
    focus_first_async() throws Error {
        Gtk.TextIter start_at;
        Gtk.TextIter match_start;
        Gtk.TextIter match_end;
        bool found;

        assert(this.search_context != null);
        assert(this.search_start_mark != null);

        if (this.cancellable!= null) {
            this.search_cancellable.cancel();
        }
        this.search_cancellable = new GLib.Cancellable();

        this.source_buffer.get_iter_at_mark(out start_at, this.search_start_mark);

        found = yield this.search_context.forward_async(
            start_at, this.search_cancellable, out match_start, out match_end, null
        );

        if (found) {
            this.search_start_mark_reset_blocked = true;
            this.source_buffer.select_range(match_start, match_end);
            this.search_start_mark_reset_blocked = false;

        } else {
            this.source_buffer.select_range(start_at, start_at);
        }

        this.scroll_to_cursor();
    }

    public void
    focus_first() {
        this.focus_first_async.begin((_, res) => {
            try {
                this.focus_first_async.end(res);
            } catch(GLib.IOError.CANCELLED err) {
            } catch(Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    public void
    find_next() {
        Gtk.TextIter start_at;
        Gtk.TextIter match_start;
        Gtk.TextIter match_end;
        bool found;

        this.wait_focus_first();

        if (this.search_context == null) {
            return;
        }

        this.source_buffer.get_selection_bounds(null, out start_at);

        found = this.search_context.forward(start_at, out match_start, out match_end, null);
        if (found) {
            this.source_buffer.select_range(match_start, match_end);
            this.reset_search_start_mark();
            this.scroll_to_cursor();
        }
    }

    public void
    find_prev() {
        Gtk.TextIter start_at;
        Gtk.TextIter match_start;
        Gtk.TextIter match_end;
        bool found;

        this.wait_focus_first();

        if (this.search_context == null) {
            return;
        }

        this.source_buffer.get_selection_bounds(out start_at, null);

        found = this.search_context.backward(start_at, out match_start, out match_end, null);
        if (found) {
            this.source_buffer.select_range(match_start, match_end);
            this.reset_search_start_mark();
            this.scroll_to_cursor();
        }
    }

    public void
    replace(string replacement) throws Error {
        Gtk.TextIter selection_start;
        Gtk.TextIter selection_end;

        return_if_fail(this.search_context != null);

        this.wait_focus_first();

        this.source_buffer.get_selection_bounds(out selection_start, out selection_end);
        this.search_context.replace(selection_start, selection_end, replacement, -1);

        this.find_next();
    }

    public void
    replace_all(string replacement) throws Error {
        return_if_fail(this.search_context != null);

        this.search_context.replace_all(replacement, -1);
    }

    public void
    clear_search() {
        if (this.search_cancellable != null) {
            this.search_cancellable.cancel();
        }
        this.search_cancellable = null;

        this.search_context = null;

        this.update_search_occurrences();
    }

    public string?
    get_selection() {
        Gtk.TextIter selection_start;
        Gtk.TextIter selection_end;
        bool found;

        found = this.source_buffer.get_selection_bounds(out selection_start, out selection_end);
        if (!found) {
            return null;
        }

        return this.source_buffer.get_slice(selection_start, selection_end, true);
    }

    private void
    search_init() {
        this.set_search_start_mark();

        this.source_buffer.mark_set.connect((loc, mark) => {
            if (mark == this.source_buffer.get_insert()) {
                this.reset_search_start_mark();
            }

            if (mark == this.source_buffer.get_insert() || mark == this.source_buffer.get_selection_bound()) {
                this.update_search_occurrences();
            }
        });
        this.source_buffer.changed.connect(() => {
            this.reset_search_start_mark();
        });
    }
}
