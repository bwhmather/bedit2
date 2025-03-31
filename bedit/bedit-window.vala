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

private async void
document_wait_idle(Bedit.Document document) {
    var handle = document.notify["busy"].connect((d, pspec) => {
        document_wait_idle.callback();
    });
    while (document.busy) {
        yield;
    }
    document.disconnect(handle);
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-window.ui")]
public sealed class Bedit.Window : Gtk.ApplicationWindow {
    private GLib.Settings settings = new GLib.Settings("com.bwhmather.Bedit2");

    private GLib.Cancellable cancellable = new GLib.Cancellable();

    [GtkChild]
    private unowned Bedit.TabView tab_view;

    public Bedit.Document? active_document { get; private set; }

    delegate void ActiveDocumentNotifyCallback();

    private void
    active_document_notify_connect(string name, ActiveDocumentNotifyCallback callback) {
        ulong handle = 0;
        Bedit.Document? current;

        if (this.active_document != null) {
            handle = this.active_document.notify[name].connect((d, pspec) => { callback(); });
        }
        current = this.active_document;

        this.notify["active-document"].connect((da, pspec) => {
            if (current != null) {
                current.disconnect(handle);
            }
            if (this.active_document != null) {
                handle = this.active_document.notify[name].connect((d, pspec) => { callback(); });
            }
            current = this.active_document;

            callback();
        });
    }

    [GtkChild]
    private unowned Bedit.Statusbar status_bar;

    /* === Menubar ======================================================================================== */

    private void
    menubar_init() {
        this.settings.bind("show-menubar", this, "show-menubar", GET);
    }

    /* === Toolbar ======================================================================================== */

    public bool show_toolbar { get; set; }

    private void
    toolbar_init() {
        this.settings.bind("show-toolbar", this, "show-toolbar", GET);
    }

    /* === Statusbar ====================================================================================== */

    public bool show_statusbar { get; set; }

    [GtkChild]
    private unowned Gtk.Label language_label;

    private void
    language_update() {
        if (this.active_document == null || this.active_document.language == null) {
            this.language_label.label = "";
        } else {
            this.language_label.label = this.active_document.language.name;
        }
    }

    private void
    statusbar_init() {
        this.settings.bind("show-statusbar", this, "show-statusbar", GET);
        this.active_document_notify_connect("language", this.language_update);
    }

    /* === Document Operations ============================================================================ */

    private async bool
    document_save_as_async(Bedit.Document document) throws Error {
        GLib.File file;
        var file_dialog = new Gtk.FileDialog();
        try {
            file = yield file_dialog.save(this, null);
        } catch (Gtk.DialogError.DISMISSED err) {
            return false;
        }

        yield document_wait_idle(document);
        yield document.save_async(file);

        return true;
    }

    private async bool
    document_save_async(Bedit.Document document) throws Error {
        yield document_wait_idle(document);
        if (document.file != null) {
            yield document.save_async(document.file);
            return true;
        }

        return yield this.document_save_as_async(document);
    }

    private async bool
    document_confirm_close_async(Bedit.Document document) throws Error {
        if (document.loading) {
            return true;
        }

        yield document_wait_idle(document);

        if (!document.modified) {
            return true;
        }

        var action = yield Bedit.CloseConfirmationDialog.run_async(this.cancellable, this, document);
        switch (action) {
        case CANCEL:
            return false;
        case DISCARD:
            return true;
        case SAVE:
            break;
        }

        return yield this.document_save_async(document);
    }

    /* === Document Actions =============================================================================== */

    private GLib.SimpleActionGroup document_actions = new GLib.SimpleActionGroup();

    /* --- Saving Documents ------------------------------------------------------------------------------- */

    private void
    action_doc_save() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);

        this.document_save_async.begin(this.active_document, (_, res) => {
            try {
                this.document_save_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    private void
    action_doc_save_as() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);

        this.document_save_as_async.begin(this.active_document, (_, res) => {
            try {
                this.document_save_as_async.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    private void
    action_doc_revert() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);
    }

    /* --- Printing Documents ----------------------------------------------------------------------------- */

    private void
    action_doc_print() {
    }

    private void
    action_doc_print_preview() {
    }

    /* --- Closing Tabs ----------------------------------------------------------------------------------- */


    private void
    action_doc_close() {
        this.tab_view.close_page(this.tab_view.selected_page);

    }

    /* --- Edit History ----------------------------------------------------------------------------------- */

    private void
    action_doc_undo() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);
        return_if_fail(this.active_document.can_undo);

        this.active_document.undo();
    }

    private void
    action_doc_redo() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);
        return_if_fail(this.active_document.can_redo);

        this.active_document.redo();
    }

    /* --- Commenting and Uncommenting -------------------------------------------------------------------- */

    private void
    action_doc_comment() {
    }

    private void
    action_doc_uncomment() {
    }

    /* --- Insert Date and Time --------------------------------------------------------------------------- */

    private void
    action_doc_insert_date_and_time() {
    }

    /* --- Line Operations -------------------------------------------------------------------------------- */

    private void
    action_doc_sort_lines() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);

        this.active_document.sort_lines();
    }

    private void
    action_doc_join_lines() {
    }

    private void
    action_doc_delete_line() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);

        this.active_document.delete_line();
    }

    private void
    action_doc_duplicate_line() {
    }

    /* --- Navigate to Line ------------------------------------------------------------------------------- */

    private void
    action_doc_show_go_to_line() {
        return_if_fail(this.active_document != null);
        // Focus will change repeatedly as stack unwinds if triggered from
        // menu.  This will cause the go-to-line widget to close.  Delay and
        // trigger from main loop instead.
        GLib.Idle.add_once(() => {
            this.active_document.go_to_line_show();
        });
    }

    /* --- Document Action State -------------------------------------------------------------------------- */

    const GLib.ActionEntry[] document_action_entries = {
        {"save", action_doc_save},
        {"save-as", action_doc_save_as},
        {"revert", action_doc_revert},
        {"print-preview", action_doc_print_preview},
        {"print", action_doc_print},
        {"close", action_doc_close},
        {"undo", action_doc_undo},
        {"redo", action_doc_redo},
        {"comment", action_doc_comment},
        {"uncomment", action_doc_uncomment},
        {"insert-date-and-time", action_doc_insert_date_and_time},
        {"sort-lines", action_doc_sort_lines},
        {"join-lines", action_doc_join_lines},
        {"delete", action_doc_delete_line},
        {"duplicate", action_doc_duplicate_line},
        {"show-go-to-line", action_doc_show_go_to_line},
    };

    private void
    document_actions_set_action_enabled(string name, bool enabled) {
        var action = this.document_actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    document_actions_update() {
        bool exists = this.active_document != null;
        bool idle = exists && !this.active_document.busy;
        bool has_file = exists && this.active_document.file != null;
        bool can_undo = exists && this.active_document.can_undo;
        bool can_redo = exists && this.active_document.can_redo;

        document_actions_set_action_enabled("save", exists && idle && has_file);
        document_actions_set_action_enabled("save-as", exists && idle);
        document_actions_set_action_enabled("revert", exists && idle);
        document_actions_set_action_enabled("print-preview", exists && idle);
        document_actions_set_action_enabled("print", exists && idle);
        document_actions_set_action_enabled("close", exists && idle);
        document_actions_set_action_enabled("undo", exists && idle && can_undo);
        document_actions_set_action_enabled("redo", exists && idle && can_redo);
        document_actions_set_action_enabled("comment", exists && idle);
        document_actions_set_action_enabled("uncomment", exists && idle);
        document_actions_set_action_enabled("insert-date-and-time", exists && idle);
        document_actions_set_action_enabled("sort-lines", exists && idle);
        document_actions_set_action_enabled("join-lines", exists && idle);
        document_actions_set_action_enabled("delete", exists && idle);
        document_actions_set_action_enabled("duplicate", exists && idle);
        document_actions_set_action_enabled("show-go-to-line", exists && idle);
    }

    private void
    document_actions_init() {
        this.document_actions.add_action_entries(document_action_entries,this);
        this.insert_action_group("doc", this.document_actions);

        this.active_document_notify_connect("can-undo", this.document_actions_update);
        this.active_document_notify_connect("can-redo", this.document_actions_update);
        this.active_document_notify_connect("file", this.document_actions_update);
        this.active_document_notify_connect("loading", this.document_actions_update);
        this.active_document_notify_connect("saving", this.document_actions_update);

        this.notify["active-document"].connect((da, pspec) => {
            this.document_actions_update();
        });

        this.document_actions_update();
    }

    /* === Clipboard ====================================================================================== */

    private GLib.SimpleActionGroup clipboard_actions = new GLib.SimpleActionGroup();

    private void
    action_clipboard_cut() {
        return_if_fail(this.active_document != null);
        return_if_fail(this.active_document.can_cut);

        this.active_document.cut();
    }

    private void
    action_clipboard_copy() {
        return_if_fail(this.active_document != null);
        return_if_fail(this.active_document.can_copy);

        this.active_document.copy();
    }

    private void
    action_clipboard_paste() {
        return_if_fail(this.active_document != null);
        return_if_fail(this.active_document.can_paste);

        this.active_document.paste();
    }

    /* --- Clipboard Action State ------------------------------------------------------------------------- */

    const GLib.ActionEntry[] clipboard_action_entries = {
        {"cut", action_clipboard_cut},
        {"copy", action_clipboard_copy},
        {"paste", action_clipboard_paste},
    };

    private void
    clipboard_actions_set_action_enabled(string name, bool enabled) {
        var action = this.clipboard_actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    clipboard_actions_update() {
        bool exists = this.active_document != null;
        bool idle = exists && !this.active_document.busy;
        bool can_cut = exists && this.active_document.can_cut;
        bool can_copy = exists && this.active_document.can_copy;
        bool can_paste = exists && this.active_document.can_paste;

        clipboard_actions_set_action_enabled("cut", exists && idle && can_cut);
        clipboard_actions_set_action_enabled("copy", exists && idle && can_copy);
        clipboard_actions_set_action_enabled("paste", exists && idle && can_paste);
    }

    private void
    clipboard_actions_init() {
        this.clipboard_actions.add_action_entries(clipboard_action_entries,this);
        this.insert_action_group("clipboard", this.clipboard_actions);

        this.active_document_notify_connect("can-cut", this.clipboard_actions_update);
        this.active_document_notify_connect("can-copy", this.clipboard_actions_update);
        this.active_document_notify_connect("can-paste", this.clipboard_actions_update);

        this.notify["active-document"].connect((da, pspec) => {
            this.clipboard_actions_update();
        });

        this.clipboard_actions_update();
    }

    /* === Selection Actions ============================================================================== */

    private GLib.SimpleActionGroup selection_actions = new GLib.SimpleActionGroup();

    private void
    action_selection_select_all() {
        return_if_fail(this.active_document != null);
        return_if_fail(!this.active_document.busy);

        this.active_document.select_all();
    }


    /* --- Selection Action State ------------------------------------------------------------------------- */

    const GLib.ActionEntry[] selection_action_entries = {
        {"select-all", action_selection_select_all},
    };

    private void
    selection_actions_set_action_enabled(string name, bool enabled) {
        var action = this.selection_actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    selection_actions_update() {
        bool exists = this.active_document != null;
        bool idle = exists && !this.active_document.busy;

        selection_actions_set_action_enabled("select-all", exists && idle);
    }

    private void
    selection_actions_init() {
        this.selection_actions.add_action_entries(selection_action_entries,this);
        this.insert_action_group("selection", this.selection_actions);

        this.notify["active-document"].connect((da, pspec) => {
            this.selection_actions_update();
        });

        this.selection_actions_update();
    }

    /* === Window Actions ================================================================================= */

    /* --- Creating New Documents and Opening Existing Ones ----------------------------------------------- */

    public void
    open_new() {
        var document = new Bedit.Document();
        this.add_document(document);
    }

    public void
    open_file(GLib.File file) {
        var document = new Bedit.Document.for_file(file);
        this.add_document(document);
    }

    private void
    action_win_new() {
        this.open_new();
    }

    private async void
    do_open() throws Error {
        var file_dialog = new Gtk.FileDialog();
        var file = yield file_dialog.open(this, this.cancellable);
        this.open_file(file);
    }

    private void
    action_win_open() {
        this.do_open.begin((_, res) => {
            try {
                this.do_open.end(res);
            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
    }

    /* --- Closing Windows and Tabs ----------------------------------------------------------------------- */

    private void
    action_win_close_window() {
        this.close_request();
    }

    /* --- Window Action State ---------------------------------------------------------------------------- */

    const GLib.ActionEntry[] window_action_entries = {
        {"new", action_win_new},
        {"open", action_win_open},
        {"close", action_win_close_window},
    };

    private void
    window_actions_init() {
        this.add_action_entries(window_action_entries, this);
    }

    /* === Search ========================================================================================= */

    [GtkChild]
    unowned Gtk.Revealer search_revealer;

    [GtkChild]
    unowned Gtk.Entry search_entry;

    [GtkChild]
    unowned Gtk.Entry replace_entry;

    public bool search_visible { get; private set; }
    public bool replace_visible { get; private set; }
    public bool search_active { get; private set; }
    public bool replace_active { get; private set; }
    public string query { get; private set; }
    public bool regex { get; set; }
    public bool case_sensitive { get; set; default=true; }

    private bool
    search_entry_on_key_press_event(Gtk.EventControllerKey controller, uint keyval, uint keycode, Gdk.ModifierType modifiers) {
        if ((keyval == Gdk.Key.ISO_Enter || keyval == Gdk.Key.KP_Enter || keyval == Gdk.Key.Return) && modifiers == 0){
            // WARNING: This is shadowed by the search entry activate binding and so will never actually
            // be triggered.
            if (this.active_document != null) {
                this.active_document.find_next();
            }
            return Gdk.EVENT_STOP;
        }

        if ((keyval == Gdk.Key.ISO_Enter || keyval == Gdk.Key.KP_Enter || keyval == Gdk.Key.Return) && modifiers == Gdk.ModifierType.SHIFT_MASK){
            if (this.active_document != null) {
                this.active_document.find_prev();
            }
            return Gdk.EVENT_STOP;
        }

        if (keyval == Gdk.Key.Tab && modifiers == 0) {
            this.replace_visible = true;
            this.replace_entry.grab_focus();
            this.replace_entry.select_region(0, -1);
            return Gdk.EVENT_STOP;
        }

        return Gdk.EVENT_PROPAGATE;
    }

    private void
    search_entry_on_activate(Gtk.Entry search_entry) {
        assert(search_entry == this.search_entry);
        if (this.active_document != null) {
            this.active_document.find_next();
        }
    }

    private void
    replace_entry_on_activate(Gtk.Entry replace_entry) {
        assert(replace_entry == this.replace_entry);
        if (this.active_document != null) {
            try {
                this.active_document.replace(this.replace_entry.text);
            } catch(Error err) {}
        }
    }

    public GLib.SimpleActionGroup search_actions = new GLib.SimpleActionGroup();

    private void
    set_search_text_from_selection() {
        string escaped;

        if (this.active_document == null) {
            return;
        }

        var selection = this.active_document.get_selection();
        if (selection == null) {
            return;
        }
        if (selection.length == 0) {
            return;
        }

        if (this.regex) {
            escaped = GLib.Regex.escape_string(selection, -1);
        } else {
            escaped = selection.escape();
        }

        if (strcmp(this.search_entry.text, escaped) == 0) {
            return;
        }

        this.search_entry.text = escaped;
        this.search_entry.set_position(-1);
    }

    private void
    action_search_find() {
        this.freeze_notify();
        this.search_visible = true;
        this.set_search_text_from_selection();
        this.thaw_notify();

        this.search_entry.grab_focus();
        this.search_entry.select_region(0, -1);
    }

    private void
    action_search_find_prev() {
        this.active_document.find_prev();
    }

    private void
    action_search_find_next() {
        this.active_document.find_next();
    }

    private void
    action_search_replace() {
        try {
            this.active_document.replace(this.replace_entry.text);
        } catch(Error err) {}
    }

    private void
    action_search_replace_all() {
        try {
            this.active_document.replace_all(this.replace_entry.text);
        } catch(Error err) {}
    }

    private void
    action_search_hide() {
        if (this.active_document != null && this.get_focus().is_ancestor(this.search_revealer))  {
            this.active_document.grab_focus();
        }
        this.freeze_notify();
        this.search_visible = false;
        this.replace_visible = false;
        this.thaw_notify();
    }

    const GLib.ActionEntry[] search_action_entries = {
        {"find", action_search_find},
        {"find-previous", action_search_find_prev},
        {"find-next", action_search_find_next},
        {"replace", action_search_replace},
        {"replace-all", action_search_replace_all},
        {"hide", action_search_hide},
    };

    private void
    search_actions_set_action_enabled(string name, bool enabled) {
        var action = this.search_actions.lookup_action(name) as GLib.SimpleAction;
        action.set_enabled(enabled);
    }

    private void
    search_actions_update() {
        search_actions_set_action_enabled("find-previous", this.search_active);
        search_actions_set_action_enabled("find-next", this.search_active);
        search_actions_set_action_enabled("replace", this.replace_active);
        search_actions_set_action_enabled("replace-all", this.replace_active);
    }

    private void
    update_search() {
        bool search_active = true;
        bool replace_active = true;
        string search_text;

        if (this.active_document == null) {
            search_active = false;
            replace_active = false;
        }

        if (!this.search_visible) {
            search_active = false;
        }
        if (!this.replace_visible) {
            replace_active = false;
        }

        search_text = this.search_entry.text;
        if (search_text == null || search_text[0] == '\0') {
            search_active = false;
            replace_active = false;
        }

        this.search_active = search_active;
        this.replace_active = replace_active;

        if (this.active_document != null) {
            if (this.search_active) {
                this.active_document.find(this.search_entry.text, this.regex, this.case_sensitive);
            } else {
                this.active_document.clear_search();
            }
        }
    }

    private void
    focus_first() {
        if (this.active_document != null && this.search_active) {
            this.active_document.focus_first();
        }
    }

    uint status_bar_search_context_id = 0;

    private void
    status_bar_update_update_search() {
        int count;
        int selected;
        string message = null;

        if (this.active_document != null) {
            count = this.active_document.num_search_occurrences;
            selected = this.active_document.selected_search_occurrence;

            if (count == 0) {
                message = "No matches found";
            } else if (count == 1 && selected > 0) {
                message = "%i of %i match".printf(selected, count);
            } else if (count > 1 && selected > 0) {
                message = "%i of %i matches".printf(selected, count);
            } else if (count == 1) {
                message = "%i match".printf(count);
            } else if (count > 1) {
                message = "%i matches".printf(count);
            }
        }

        if (this.status_bar_search_context_id == 0) {
            this.status_bar_search_context_id = this.status_bar.new_context_id();
        }

        // TODO only remove if changed.
        this.status_bar.remove_all(this.status_bar_search_context_id);
        if (message != null) {
            this.status_bar.push(this.status_bar_search_context_id, message);
        }
    }

    private void
    search_init() {
        Gtk.EventControllerKey event_controller;

        this.search_actions.add_action_entries(search_action_entries, this);

        this.search_actions.add_action(new GLib.PropertyAction("case-sensitive", this, "case-sensitive"));
        this.search_actions.add_action(new GLib.PropertyAction("regex", this, "regex"));

        this.insert_action_group("search", this.search_actions);

        event_controller = new Gtk.EventControllerKey();
        event_controller.key_pressed.connect(search_entry_on_key_press_event);
        this.search_entry.add_controller(event_controller);
        this.search_entry.activate.connect(this.search_entry_on_activate);
        this.search_entry.changed.connect((_) => {
            this.update_search();
            this.focus_first();
        });

        this.bind_property("search-visible", this.search_revealer, "reveal-child", SYNC_CREATE);

        this.replace_entry.activate.connect(this.replace_entry_on_activate);

        this.notify["case-sensitive"].connect((s, pspec) => { this.update_search(); });
        this.notify["regex"].connect((s, pspec) => { this.update_search(); });

        this.notify["active-document"].connect((s, pspec) => { this.update_search(); });
        this.notify["search-visible"].connect((s, pspec) => {this.update_search();});
        this.notify["replace-visible"].connect((s, pspec) => {this.update_search();});

        this.notify["search-active"].connect((s, pspec) => { this.search_actions_update(); });
        this.notify["replace-active"].connect((s, pspec) => { this.search_actions_update(); });

        this.active_document_notify_connect("num-search-occurrences", this.status_bar_update_update_search);
        this.active_document_notify_connect("selected-search-occurrence", this.status_bar_update_update_search);

        this.search_actions_update();
    }

    /* === Lifecycle ====================================================================================== */

    private void
    on_tab_view_selected_tab_changed(GLib.Object _, GLib.ParamSpec pspec) {
        var page = this.tab_view.selected_page;
        if (page == null) {
            this.active_document = null;
        } else {
            this.active_document = page.child as Bedit.Document;
        }
    }

    private bool closing = false;

    private bool
    on_window_close_request(Gtk.Window window) {
        this.closing = true;

        if (this.tab_view.selected_page != null) {
            this.tab_view.close_page(this.tab_view.selected_page);
            return Gdk.EVENT_STOP;
        }

        return Gdk.EVENT_PROPAGATE;
    }


    private bool
    on_tab_view_close_page_request(Bedit.TabView view, Bedit.TabPage page) {
        var document = page.child as Bedit.Document;
        this.document_confirm_close_async.begin(document, (_, res) => {
            try {
                var should_close = this.document_confirm_close_async.end(res);
                view.close_page_finish(page, should_close);

                if (!should_close) {
                    this.closing = false;
                }
                if (this.closing){
                    if (this.tab_view.selected_page != null) {
                        this.tab_view.close_page(this.tab_view.selected_page);
                    } else {
                        GLib.Idle.add_once(this.close);
                    }
                }

            } catch (Error err) {
                warning("Error: %s\n", err.message);
            }
        });
        return Gdk.EVENT_STOP;
    }

    class construct {
        typeof (Bedit.ButtonGroup).ensure();
        typeof (Bedit.ToolbarView).ensure();
        typeof (Bedit.Statusbar).ensure();
        typeof (Bedit.Toolbar).ensure();
        typeof (Bedit.TabView).ensure();
        typeof (Bedit.Document).ensure();

        add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Escape"),
            new Gtk.NamedAction("search.hide")
        ));
    }

    construct {
        var window_group = new Gtk.WindowGroup();
        window_group.add_window(this);

        tab_view.notify["selected-page"].connect(on_tab_view_selected_tab_changed);
        tab_view.close_page.connect(on_tab_view_close_page_request);

        this.close_request.connect(on_window_close_request);
        var w = (this as Gtk.Widget);
        w.destroy.connect((w) => {
            cancellable.cancel();
        });

        this.menubar_init();
        this.toolbar_init();
        this.statusbar_init();
        this.window_actions_init();
        this.document_actions_init();
        this.clipboard_actions_init();
        this.selection_actions_init();
        this.search_init();
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.Window));
    }

    public Window(Gtk.Application application) {
        Object(
            application: application,
            show_menubar: true
        );
    }

    private void
    add_document(Bedit.Document document) {
        Bedit.TabPage page = this.tab_view.add_page(document, null);
        document.bind_property("title", page, "title", SYNC_CREATE);
        this.tab_view.selected_page = page;
        this.tab_view.grab_focus();
    }
}
