/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
private enum Bedit.FileDialogViewMode {
    LIST,
    ICON,
    TREE
}
/*
private sealed class Bedit.FileDialogState : GLib.Object {
    // Application state.
    public Bedit.FileDialogViewMode view_mode;

    public bool show_binary { get; set; }
    public bool show_hidden { get; set; }

    // Window state.
    // Shared state.
    public GLib.File root_directory; // Path to root folder under mount.

    // Tree view specific.
    public string[] expanded;  // Sorted list of expanded directories under the current mount.

    // List view specific.
    public string[] sort_columns;

    public Bedit.FileDialogState
    dup() {
    }
}
*/
//state: map[windowId]FileDialogState


// On begin:
// Try to load state.
// Fall back to creating a new state with the passed in root folder.
// Fall back to creating a new state from the home directory.


// On end:
// Set or update saved state for window.


private sealed class Bedit.FileDialogPathBar : Gtk.Widget {
    public GLib.File root_directory { get; set; }

    class construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
    }

    public override void
    dispose() {
        base.dispose();
    }
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog.ui")]
private sealed class Bedit.FileDialogWindow : Gtk.Window {
    // Path to root folder under mount.

    public GLib.File root_directory { get; set; }
    public Gtk.DirectoryList directory_list;

    public signal void open(GLib.File result);

    public GLib.ListModel selection { get; set; default=new GLib.ListStore(typeof (GLib.File)); }

    public GLib.SimpleActionGroup dialog_actions = new GLib.SimpleActionGroup();

    /* === Views ========================================================================================== */

    [GtkChild]
    private unowned Brk.ToolbarView outer_view;

    [GtkChild]
    private unowned Brk.ToolbarView inner_view;

    [GtkChild]
    private unowned Gtk.Entry location_entry;

    [GtkChild]
    private unowned Bedit.FileDialogPathBar path_bar;

    [GtkChild]
    private unowned Brk.ButtonGroup view_button_group;

    [GtkChild]
    private unowned Gtk.Stack view_stack;

    public bool filter_view_enabled { get; set; }
    public bool edit_location_enabled { get; set; }
    // This is the view that should be shown when filter mode is not enabled.
    public Bedit.FileDialogViewMode view_mode { get; set; default = LIST; }

    public bool show_binary { get; set; }
    public bool show_hidden { get; set; }

    private void
    view_stack_update_visible_child() {
        if (this.filter_view_enabled) {
            this.filter_entry.visible = true;
            this.location_entry.visible = false;
            this.path_bar.visible = false;
            this.view_button_group.visible = false;

            this.view_stack.visible_child = this.filter_view;
            return;
        }

        this.filter_entry.visible = false;
        this.view_button_group.visible = true;
        if (this.edit_location_enabled) {
            this.location_entry.visible = true;
            this.path_bar.visible = false;
        } else {
            this.location_entry.visible = false;
            this.path_bar.visible = true;
        }
        switch (this.view_mode) {
        case LIST:
            this.view_stack.visible_child = this.list_view;
            break;
        case ICON:
            this.view_stack.visible_child = this.icon_view;
            break;
        case TREE:
            this.view_stack.visible_child = this.tree_view;
            break;
        }
    }

    private void
    action_open() {
        this.open(this.selection.get_item(0) as GLib.File);
    }

    private void
    views_init() {
        this.bind_property("root-directory", this.path_bar, "root-directory", BIDIRECTIONAL | SYNC_CREATE);

        this.dialog_actions.add_action(new GLib.PropertyAction("filter", this, "filter-view-enabled"));

        this.dialog_actions.add_action(new GLib.PropertyAction("view-mode", this, "view-mode"));

        this.dialog_actions.add_action(new GLib.PropertyAction("show-binary", this, "show-binary"));
        this.dialog_actions.add_action(new GLib.PropertyAction("show-hidden", this, "show-hidden"));

        var open_action = new GLib.SimpleAction("open", null);
        open_action.activate.connect(this.action_open);
        this.dialog_actions.add_action(open_action);
        this.notify["selection"].connect((d, pspec) => {
            open_action.set_enabled(this.selection.get_n_items() > 0);
        });
        open_action.set_enabled(this.selection.get_n_items() > 0);

        this.notify["filter-view-enabled"].connect(this.view_stack_update_visible_child);
        this.notify["view-mode"].connect(this.view_stack_update_visible_child);
        this.view_stack_update_visible_child();

        // Unhandled keypresses get redirected to the filter view entry and, if
        // handled there, result in a switch to filter mode.
        var event_controller = new Gtk.EventControllerKey();
        event_controller.name = "Redirect";
        event_controller.propagation_phase = BUBBLE;
        event_controller.key_pressed.connect((controller, keyval, keycode, modifiers) => {
            if (event_controller.forward(this.filter_entry.get_delegate())) {
                this.filter_view_enabled = true;
                // It's more usual for the forwarding widget to keep focus but
                // we need the entry to have focus so that it can enable event
                // buffering on submit.
                this.filter_entry.grab_focus_without_selecting();
                return true;
            }
            return false;
        });
        this.outer_view.add_controller(event_controller);

        var filter_focus_controller = new Gtk.EventControllerFocus();
        filter_focus_controller.leave.connect(() => {
            this.filter_view_enabled = false;
        });
        this.inner_view.add_controller(filter_focus_controller);

        var filter_cancel_controller = new Gtk.ShortcutController();
        filter_cancel_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Escape"),
            new Gtk.CallbackAction(() => {
                // Partially duplicated on filter entry to capture Escape
                // during CAPTURE phasee when buffering enabled.
                if (this.filter_view_enabled) {
                    this.filter_view_enabled = false;
                    return true;
                }
                return false;
            })
        ));
        this.inner_view.add_controller(filter_cancel_controller);
    }

    /* --- Filter View ------------------------------------------------------------------------------------ */

    [GtkChild]
    private unowned Gtk.Entry filter_entry;

    [GtkChild]
    private unowned Bedit.FileDialogFilterView filter_view;

    // Keyboard input on the filter entry can be handled asnychronously if the
    // view is still catching up.  Navigation (using up and down keys  before
    // the entry is submitted is buffered manually here.  Everything after the
    // entry is submitted is buffered using a Gtk.EventControllerBuffer.  Input
    // anywhere else is handled synchronously and ignores these two mechanisms.
    private int[] filter_entry_navigate = null;
    private bool filter_entry_submit = false;

    private void
    filter_entry_clear_commands() {
        this.filter_entry_navigate = null;
        this.filter_entry_submit = false;
    }

    private void
    filter_entry_play_commands() {
        if (this.filter_view.loading) {
            return;
        }

        foreach (int step in this.filter_entry_navigate) {
            while (step < 0) {
                this.filter_view.select_prev();
                step += 1;
            }
            while (step > 0) {
                this.filter_view.select_next();
                step -= 1;
            }
        }
        if (this.filter_entry_submit) {

        }
        this.filter_entry_clear_commands();
    }


    private void
    filter_view_init() {
        var buffer_controller = new Gtk.EventControllerBuffer();
        buffer_controller.propagation_phase = CAPTURE;
        buffer_controller.discarded.connect(() => {
            // Queued commands are effectively the same as buffered input.
            // If we lose the input buffer then commands should also be dropped.
            this.filter_entry_clear_commands();
        });

        var cancel_controller = new Gtk.ShortcutController();
        cancel_controller.propagation_phase = CAPTURE;
        cancel_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Escape"),
            new Gtk.CallbackAction(() => {
                // This shortcut controller is only installed so that we can
                // bypass the keyboard buffer and cancel a long running query
                // without having to wait until it finishes.  Usually we want
                // to wait until BUBBLE so that the input method can get a
                // chance to consume it first.
                if (buffer_controller.enabled) {
                    this.filter_view_enabled = false;
                    return true;
                }
                return false;
            })
        ));

        var shortcut_controller = new Gtk.ShortcutController();
        shortcut_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Enter"),
            new Gtk.CallbackAction(() => {
                return_val_if_fail(!this.filter_entry_submit, false);  // Should be caught by buffer.
                buffer_controller.enable();
                this.filter_entry_submit = true;

                return true;
            })
        ));
        shortcut_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Up"),
            new Gtk.CallbackAction(() => {
                return_val_if_fail(!this.filter_entry_submit, false);  // Should be caught by buffer.
                this.filter_entry_navigate += -1;
                this.filter_entry_play_commands();
                return true;
            })
        ));
        shortcut_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("Down"),
            new Gtk.CallbackAction(() => {
                return_val_if_fail(!this.filter_entry_submit, false);  // Should be caught by buffer.
                this.filter_entry_navigate += 1;
                this.filter_entry_play_commands();
                return true;
            })
        ));

        // The order here is important.  The cancel controller must be able to
        // intercept events before they are buffered.  The shortcut controller
        // must not receive events until after they are released.
        this.filter_entry.add_controller(cancel_controller);
        this.filter_entry.add_controller(buffer_controller);
        this.filter_entry.add_controller(shortcut_controller);

        this.filter_entry.bind_property("text", this.filter_view, "query", SYNC_CREATE);
        this.filter_entry.notify["text"].connect((fe, pspec) => {
            // Buffered keyboard events will have happened before whatever
            // changed the entry (for example, a careful middle click paste)
            // and so can't be replayed after.
            buffer_controller.discard();
        });
        this.notify["filter-view-enabled"].connect((v, pspec) => {
            if (!this.filter_view_enabled) {
                this.filter_entry.text = "";
            }
        });

        this.filter_view.notify["loading"].connect((fv, pspec) => {
            if (!this.filter_view.loading) {
                this.filter_entry_play_commands();
                buffer_controller.replay();
            }
        });

        // Settings.
        this.bind_property("root-directory", this.filter_view, "root-directory", SYNC_CREATE);
        this.bind_property("show-binary", this.filter_view, "show-binary", SYNC_CREATE);
        this.bind_property("show-hidden", this.filter_view, "show-hidden", SYNC_CREATE);
    }

    /* --- List View -------------------------------------------------------------------------------------- */

    public string[] sort_columns;

    [GtkChild]
    private unowned Bedit.FileDialogListView list_view;

    private void
    list_view_init() {
        this.list_view.directory_list = this.directory_list;

        this.notify["view-mode"].connect(() => {
            if (this.view_mode == LIST) {
                // Apply current selection to list view when switching from a different view.
                this.list_view.selection = this.selection;
            }
        });
        this.notify["filter-view-enabled"].connect((v, pspec) => {
            if (this.view_mode == LIST && !this.filter_view_enabled) {
                // Resync selection to match list view when coming out of filter mode.
                this.selection = this.list_view.selection;
            }
        });
        this.list_view.notify["selection"].connect((v, pspec) => {
            if (this.view_mode == LIST && !this.filter_view_enabled) {
                // Sync selection to match list view when list view visible.
                this.selection = this.list_view.selection;
            }
        });
    }

    /* --- Icon View -------------------------------------------------------------------------------------- */

    [GtkChild]
    private unowned Bedit.FileDialogIconView icon_view;

    private void
    icon_view_init() {
        this.icon_view.directory_list = this.directory_list;
    }

    /* --- Tree View -------------------------------------------------------------------------------------- */

    // Sorted list of expanded directories under the current mount.
    public string[] expanded;

    [GtkChild]
    private unowned Bedit.FileDialogTreeView tree_view;

    private void
    tree_view_init() {
        this.tree_view.directory_list = this.directory_list;
    }

    /* === Lifecycle ======================================================================================= */

    class construct {
        typeof (Bedit.FileDialogPathBar).ensure();
        typeof (Bedit.FileDialogFilterView).ensure();
        typeof (Bedit.FileDialogListView).ensure();
        typeof (Bedit.FileDialogIconView).ensure();
        typeof (Bedit.FileDialogTreeView).ensure();
    }

    construct {
        this.directory_list = new Gtk.DirectoryList(
            "standard::display-name,standard::size,time::modified,standard::type",
            this.root_directory
        );
        directory_list.monitored = true;
        this.bind_property("root-directory", this.directory_list, "file", SYNC_CREATE);

        this.views_init();
        this.filter_view_init();
        this.list_view_init();
        this.icon_view_init();
        this.tree_view_init();

        this.insert_action_group("dialog", this.dialog_actions);
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogWindow));
        base.dispose();
    }
}

/**
 * Asnychronous API for opening a file chooser dialog.
 *
 * Bedit.FileDialog collects the arguments that are needed topresent the dialog to the user such as a title
 * or whether it should be modal.
 *
 * It is safe to reuse a file dialog object multiple times to serve multiple requests.  You do not need to
 * wait for previous requests to finish before asking for a new window.
 */
sealed class Bedit.FileDialog : GLib.Object {
    public string title { get; set; }

    public GLib.File initial_file { get; set; }
    public GLib.File initial_folder { get; set; }
    public string initial_name { get; set; }

    public Gtk.FileFilter default_filter { get; set; }
    public GLib.ListModel filters { get; set; }

    public string accept_label { get; set; }

    /**
     * Opens a new file chooser dialog to allow the user to select a single file for reading.
     *
     * If the user closes the chooser without selecting a file will return NULL.
     * Will throw a CANCELLED error if interrupted.
     */
    public async GLib.File?
    open(Gtk.Window? parent, GLib.Cancellable cancellable) throws Error {
        var window = new Bedit.FileDialogWindow();
        window.set_transient_for(parent);

        window.root_directory =  GLib.File.new_for_path("/usr/include");
        window.view_mode = LIST;
        window.sort_columns = {};
        window.expanded = {};

        GLib.File? result = null;
        bool done = false;

        cancellable.connect((c) => {
            if (!done) {
                done = true;
                window.close();
                this.open.callback();
            }
        });
        window.open.connect((file) => {
            result = file;
            if (!done) {
                done = true;
                window.close();
                this.open.callback();
            }
        });
        window.unmap.connect((w) => {
            if (!done) {
                done = true;
                this.open.callback();
            }
        });
        window.present();
        yield;

        if (cancellable.is_cancelled()) {
            throw new GLib.IOError.CANCELLED("open cancelled");
        }

        return result;
    }
}
