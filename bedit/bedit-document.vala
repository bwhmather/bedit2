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

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-document.ui")]
public sealed class Bedit.Document : Gtk.Widget {
    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit2");
    private GLib.Settings settings_desktop = new GLib.Settings("org.gnome.desktop.interface");

    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned GtkSource.View source_view;
    private unowned GtkSource.Buffer source_buffer;
    private GtkSource.File source_file = new GtkSource.File();

    public string title { get; private set; }
    public unowned GLib.File? file {
        get { return this.source_file.location; }
        construct { this.source_file.location = value; }
    }
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
        this.source_buffer.notify["has-selection"].connect((sb, pspec) => {
            bool has_selection = this.source_buffer.has_selection;
            this.can_cut = has_selection;
            this.can_copy = has_selection;
        });
        this.source_buffer.modified_changed.connect((tb) => {
            this.modified = this.source_buffer.get_modified();
        });
        this.source_buffer.end_user_action.connect((tb) => {
            this.source_view.scroll_mark_onscreen(this.source_buffer.get_insert());
        });

        this.notify["loading"].connect((_, pspec) => { this.update_busy(); });
        this.notify["saving"].connect((_, pspec) => { this.update_busy(); });

        title_init();
        language_init();
        font_init();
        word_wrap_init();
        indentation_init();
        overview_map_init();
        highlight_current_line_init();
        highlight_syntax_init();
        line_numbers_init();
        start_mark_init();
        search_init();
        go_to_line_init();

        if (file != null) {
            reload_async.begin(null);
        }
    }

    ~Document() {
        assert(!this.busy);

        title_deinit();
    }

    public Document.for_file(GLib.File file) {
        Object(file: file);
    }

    public override bool
    grab_focus() {
        return this.source_view.grab_focus();
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

    /* === Editing ======================================================================================== */

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

    /* === Metadata ======================================================================================= */

    /* --- Title ------------------------------------------------------------------------------------------ */

    private static bool[] title_allocated_draft_numbers = new bool[16];

    private uint title_draft_number;

    private void
    title_acquire_draft_number() {
        if (this.title_draft_number == 0) {
            while (true) {
                this.title_draft_number++;

                if (this.title_draft_number >= title_allocated_draft_numbers.length) {
                    title_allocated_draft_numbers.resize(title_allocated_draft_numbers.length + 1);
                    title_allocated_draft_numbers[this.title_draft_number] = true;
                    break;
                }

                if (!title_allocated_draft_numbers[this.title_draft_number]) {
                    title_allocated_draft_numbers[this.title_draft_number] = true;
                    break;
                }
            }
        }
    }

    private void
    title_release_draft_number() {
        if (this.title_draft_number != 0) {
            title_allocated_draft_numbers[this.title_draft_number] = false;
            this.title_draft_number = 0;
        }
    }

    private void
    title_update() {
        if (this.file == null) {
            this.title_acquire_draft_number();
            this.title = "Untitled Document %u".printf(this.title_draft_number);
        } else {
            this.title_release_draft_number();
            this.title = this.file.get_basename();
        }
    }

    private void
    title_init() {
        this.source_file.notify["location"].connect((sf, pspec) => { this.title_update(); });
        this.title_update();
    }

    private void
    title_deinit() {
        this.title_release_draft_number();
    }

    /* --- Language --------------------------------------------------------------------------------------- */

    public GtkSource.Language language { get; set; }

    private void
    language_update() {
        if (this.file == null) {
            return;
        }

        var language_manager = GtkSource.LanguageManager.get_default();
        var language = language_manager.guess_language(this.file.get_path(), null);
        if (language == null) {
            // Don't clear language if already set.
            return;
        }

        this.language = language;
    }

    private void
    language_init() {
        this.bind_property("language", this.source_buffer, "language", SYNC_CREATE);

        this.source_file.notify["location"].connect((sf, pspec) => { this.language_update(); });
        this.language_update();
    }

    /* === Indentation ===================================================================================== */

    public uint tab_width { get; set; }
    public bool insert_spaces_instead_of_tabs { get; set; }

    private void
    indentation_init() {
        this.settings.bind("tab-width", this, "tab-width", GET);
        this.bind_property("tab-width", this.source_view, "tab-width", SYNC_CREATE);

        this.settings.bind("insert-spaces-instead-of-tabs", this, "insert-spaces-instead-of-tabs", GET);
        this.bind_property("insert-spaces-instead-of-tabs", this.source_view, "insert-spaces-instead-of-tabs", SYNC_CREATE);
    }

    /* === Appearance ===================================================================================== */

    /* --- Font ------------------------------------------------------------------------------------------- */

    private Gtk.CssProvider font_css_provider;

    public bool use_default_font { get; set; }
    public string editor_font { get; set; }

    private void
    update_font() {
        string font_name;
        if (this.use_default_font) {
            font_name = settings_desktop.get_string("monospace-font-name");
        } else {
            font_name = this.editor_font;
        }
        var font_desc = Pango.FontDescription.from_string(font_name);
        var font_css = Bedit.font_description_to_css(font_desc);
        font_css_provider.load_from_string("textview { %s }".printf(font_css));
    }


    private void
    font_init() {
        this.font_css_provider = new Gtk.CssProvider();
        this.source_view.get_style_context().add_provider(this.font_css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

        this.settings.bind("use-default-font", this, "use-default-font", GET);
        this.settings.bind("editor-font", this, "editor-font", GET);

        this.notify["use-default-font"].connect((d, pspec) => { this.update_font(); });
        this.notify["editor-font"].connect((d, pspec) => { this.update_font(); });
        this.update_font();
    }

    /* --- Word Wrap -------------------------------------------------------------------------------------- */

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

    /* --- Highlight Current Line ------------------------------------------------------------------------- */

    public bool highlight_current_line { get; set; }

    private void
    highlight_current_line_init() {
        this.settings.bind("highlight-current-line", this, "highlight-current-line", GET);
        this.bind_property("highlight-current-line", this.source_view, "highlight-current-line", SYNC_CREATE);
    }

    /* --- Highlight Current Line ------------------------------------------------------------------------- */

    public bool highlight_syntax { get; set; }

    private void
    highlight_syntax_init() {
        this.settings.bind("highlight-syntax", this, "highlight-syntax", GET);
        this.bind_property("highlight-syntax", this.source_buffer, "highlight-syntax", SYNC_CREATE);
    }

    /* --- Line Numbers ----------------------------------------------------------------------------------- */

    public bool show_line_numbers { get; set; }

    private void
    line_numbers_init() {
        this.settings.bind("show-line-numbers", this, "show-line-numbers", GET);
        this.bind_property("show-line-numbers", this.source_view, "show-line-numbers", SYNC_CREATE);
    }

    /* --- Overview Map ----------------------------------------------------------------------------------- */

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

    /* === Navigation ===================================================================================== */

    private Gtk.TextMark? start_mark;
    private bool start_mark_reset_blocked;

    private void
    scroll_to_cursor() {
        this.source_view.scroll_to_mark(this.source_buffer.get_insert(), 0.25, false, 0.0, 0.0);
    }

    private void
    clear_start_mark() {
        if (start_mark != null) {
            this.source_buffer.delete_mark(this.start_mark);
            this.start_mark = null;
        }
    }

    private void
    set_start_mark() {
        return_if_fail(this.start_mark == null);

        Gtk.TextIter start_iter;
        this.source_buffer.get_selection_bounds(out start_iter, null);
        this.start_mark = this.source_buffer.create_mark(null, start_iter, false);
    }

    private void
    reset_start_mark() {
        if (!this.start_mark_reset_blocked) {
            this.clear_start_mark();
            this.set_start_mark();
        }
    }


    private void
    start_mark_init() {
        this.set_start_mark();

        this.source_buffer.mark_set.connect((loc, mark) => {
            if (mark == this.source_buffer.get_insert()) {
                this.reset_start_mark();
            }
        });
        this.source_buffer.changed.connect(() => {
            this.reset_start_mark();
        });
    }

    /* --- Search and Replace ----------------------------------------------------------------------------- */

    private GtkSource.SearchContext? search_context;
    private GLib.Cancellable? search_cancellable;

    public int num_search_occurrences { get; private set; }
    public int selected_search_occurrence { get; private set; }

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
    wait_focus_first() {
        Gtk.TextIter start_at;
        Gtk.TextIter match_start;
        Gtk.TextIter match_end;
        bool found;

        if (this.search_cancellable == null) {
            return;
        }

        return_if_fail(this.search_context != null);
        return_if_fail(this.start_mark != null);

        // TODO it would be nice if there was a way to block on the current run
        // of bedit_searchbar_focus_first() instead of killing it and starting
        // again.
        this.search_cancellable.cancel();
        this.search_cancellable = null;

        this.source_buffer.get_iter_at_mark(out start_at, this.start_mark);

        found = this.search_context.forward(start_at, out match_start, out match_end, null);

        if (found) {
            this.start_mark_reset_blocked = true;
            this.source_buffer.select_range(match_start, match_end);
            this.start_mark_reset_blocked = false;

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
        assert(this.start_mark != null);

        if (this.cancellable!= null) {
            this.search_cancellable.cancel();
        }
        this.search_cancellable = new GLib.Cancellable();

        this.source_buffer.get_iter_at_mark(out start_at, this.start_mark);

        found = yield this.search_context.forward_async(
            start_at, this.search_cancellable, out match_start, out match_end, null
        );

        if (found) {
            this.start_mark_reset_blocked = true;
            this.source_buffer.select_range(match_start, match_end);
            this.start_mark_reset_blocked = false;

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
            this.reset_start_mark();
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
            this.reset_start_mark();
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
        this.source_buffer.mark_set.connect((loc, mark) => {
            if (mark == this.source_buffer.get_insert() || mark == this.source_buffer.get_selection_bound()) {
                this.update_search_occurrences();
            }
        });
    }

    /* --- Go To Line ------------------------------------------------------------------------------------- */

    [GtkChild]
    private unowned Gtk.Revealer go_to_line_revealer;

    [GtkChild]
    private unowned Gtk.Entry go_to_line_entry;

    uint go_to_line_timeout_id;

    private void
    go_to_line_commit() {
        this.reset_start_mark();
    }

    private void
    go_to_line_hide() {
        Gtk.TextIter start_iter;

        if (!this.go_to_line_revealer.reveal_child) {
            return;
        }
        this.go_to_line_revealer.reveal_child = false;

        if (this.go_to_line_timeout_id != 0) {
            GLib.Source.remove(this.go_to_line_timeout_id);
            this.go_to_line_timeout_id = 0;
        }

        this.source_buffer.get_iter_at_mark(out start_iter, this.start_mark);
        this.source_buffer.place_cursor(start_iter);
        this.scroll_to_cursor();
    }

    private void
    go_to_line_update() {
        string text;
        string line_text;
        string column_text;
        Gtk.TextIter start_at;
        int line;
        int column;

        text = go_to_line_entry.get_text();
        this.source_buffer.get_iter_at_mark(out start_at, this.start_mark);

        if (text[0] == '\0') {
            this.source_buffer.place_cursor(start_at);
            this.scroll_to_cursor();

            // TODO clear error.

            return;
        }

        var components = text.split(":", 2);
        line_text = components[0];
        column_text = components[1];

        line = 0;
        switch (text[0]) {
        case '\0':
            line = start_at.get_line();
            break;

        case '-':
            int curr_line = start_at.get_line();

            int offset = 0;
            if (text[1] != '\0') {
                bool ok = int.try_parse(text[1:], out offset);
                if (!ok) {
                    // Assume overflow.  Snap to first number that will trigger out of bounds and set error
                    // on input.
                    offset = curr_line + 1;
                }
            }
            line = curr_line - offset;
            break;

        case '+':
            int curr_line = start_at.get_line();

            int offset = 0;
            if (text[1] != '\0') {
                bool ok = int.try_parse(text[1:], out offset);
                if (!ok) {
                    // Assume overflow.  Snap to offset that is (almost) guaranteed to be out of bounds to set
                    // error on input..
                    offset = int.MAX - line;
                }
            }

            line = curr_line + offset;
            break;

        default:
            bool ok = int.try_parse(text, out line);
            if (!ok) {
                line = int.MAX;
            }
            line -= 1;
            break;
        }

        column = 0;
        if (column_text != null && column_text[0] != '\0') {
            column = int.parse(column_text);
        }

        Gtk.TextIter iter;
        this.source_buffer.get_iter_at_line_offset(out iter, line, column);
        this.start_mark_reset_blocked = true;
        this.source_buffer.place_cursor(iter);
        this.start_mark_reset_blocked = false;
        this.scroll_to_cursor();

        if (iter.get_line() != line || iter.get_line_offset() != column) {
            // TODO set error.
        } else {
            // TODO clear error.
        }
    }

    public void
    go_to_line_show() {
        Gtk.TextIter iter;

        this.go_to_line_revealer.reveal_child = true;

        var cursor = this.source_buffer.get_insert();
        this.source_buffer.get_iter_at_mark(out iter, cursor);
        this.go_to_line_entry.text = iter.get_line().to_string();
        this.go_to_line_entry.select_region(0, -1);

        this.go_to_line_entry.grab_focus();
    }

    private void
    go_to_line_init() {
        this.go_to_line_entry.changed.connect((e) => { this.go_to_line_update(); });

        var focus_controller = new Gtk.EventControllerFocus();
        focus_controller.leave.connect((ec) => { this.go_to_line_hide(); });
        this.go_to_line_entry.add_controller(focus_controller);

        var shortcut_controller = new Gtk.ShortcutController();
        shortcut_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Escape"),
            new Gtk.CallbackAction((w, a) =>{ this.go_to_line_hide();  return true; })
        ));
        this.go_to_line_entry.add_controller(shortcut_controller);

        this.go_to_line_entry.activate.connect((e) => {
            this.go_to_line_commit();
            this.go_to_line_hide();
        });
    }
}
