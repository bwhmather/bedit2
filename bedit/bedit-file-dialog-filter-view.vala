/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */


/**
Layers:
  - Generate a list of candidates contained in the parent files.
  - Filter (if query ends with a slash there is an implicit empty filter).

Datastructures:
Stack:
  query segment -> candidates list
  e.g. [
    "~": [...],
    "pr": [..."],
    "b": [...]



Candidates are saved as a GLib.FileInfo with an additional "bedit::markup" attribute


Preprocess:
  - Strip prefix to determine root


For token in tokens:
  switch
  case ".."
    todo
  case "/":
    push working list on to top of stack.
  default: (query segment)
    if exact match:
        continue
    if cached segment is prefix of current query segment:
        truncate stack above current segment
        replace top by filtering and sorting using new query (how do you rebuild the markup? - Only build lazily for the top of the stack)

    else (if not a match or no segment):
      truncate stack including current segment
      iterate over new top of stack and generate new working list
      filter working list and sort using new query
 */


delegate void ResultCallback(GLib.FileInfo fileinfo);

private struct QueryStackEntry {
    string subquery;
    GLib.FileInfo[] matches;
}


private const string ATTRIBUTES = "standard::*,time::modified";

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-filter-view.ui")]
internal sealed class Bedit.FileDialogFilterView : Gtk.Widget {

    /* === State ========================================================================================== */

    public GLib.File? root_directory { get; set; default = null; }

    public string query { get; set; default = "";}

    private GLib.ListStore list_store;
    private Gtk.SingleSelection selection_model;

    private GLib.Cancellable query_cancellable;
    private QueryStackEntry[] query_stack = null;
    private void
    query_stack_truncate(int n) {
        for (var i = n; i < this.query_stack.length; i++) {
            this.query_stack[i] = {};
        }
        this.query_stack.resize(n);
    }

    private async void
    update_matches() {
        // At every `yield`, the query stack must be left in a valid state.

        int n = 0;
        var remainder = this.query[:];

        GLib.File? root = this.root_directory;
        string subquery = "";
        if (remainder.has_prefix("~/")) {
            root = GLib.File.new_for_path(GLib.Environment.get_home_dir());
            subquery = "~/";
            remainder = remainder[2:];
        } else if (remainder.has_prefix("./")) {
            remainder = remainder[2:];
        } else if (remainder.has_prefix("/")) {
            root = GLib.File.new_for_path("/");
            subquery = "/";
            remainder = remainder[1:];
        }
        if (root == null) {
            this.query_stack_truncate(0);
            this.list_store.remove_all();
            return;
        }

        if (this.query_stack.length <= n || this.query_stack[n].subquery != subquery) {
            this.query_stack_truncate(n);  // Truncate before doing anything else to leave stack in valid state.

            try {
                var rootinfo = yield root.query_info_async(ATTRIBUTES, NONE, GLib.Priority.DEFAULT, this.query_cancellable);
                rootinfo.set_attribute_object("standard::file", root);

                this.query_stack.resize(n + 1);
                this.query_stack[n].subquery = subquery;
                this.query_stack[n].matches = {rootinfo};
            } catch {
                // TODO
                return;
            }
        }

        bool done = false;
        while (!done) {
            n += 1;

            // Subquery is from the beginning of the remaining query to either the next `/` or the very end.
            // We need to read the next segment here and consume any following slashes.
            // Segments can be empty if the query ends with a trailing /.
            int next_slash = remainder.index_of_char('/', 0);
            if (next_slash >= 0) {
                subquery = remainder[:next_slash];
                remainder = remainder[next_slash + 1:];
            } else {
                subquery = remainder;
                remainder = "";
                done = true;
            }

            if (this.query_stack.length > n && this.query_stack[n].subquery == subquery) {
                // Use cached entry from stack.
                continue;
            }

            if (subquery == "..") {
                this.query_stack_truncate(n);  // Truncate to leave stack in valid state.

                // Find all parent directories of all previous matches.
                GLib.FileInfo[] matches = {};
                GLib.File last_parent = null;
                foreach (GLib.FileInfo matchinfo in this.query_stack[n-1].matches) {
                    GLib.File match = matchinfo.get_attribute_object("standard::file") as GLib.File;
                    GLib.File parent = match.get_parent();

                    if (parent == null) {
                        // Can't traverse to parent of root directory.
                        continue;
                    }
                    if (parent.equal(last_parent)) {
                        // Matches should already be sorted by parent directory so deduplicating just a matter
                        // of tracking it.
                        continue;
                    }
                    try {
                        var parentinfo = yield parent.query_info_async(ATTRIBUTES, NONE, GLib.Priority.DEFAULT, this.query_cancellable);
                        parentinfo.set_attribute_object("standard::file", parent);

                        matches += parentinfo;
                    } catch {
                        // TODO
                        return;
                    }
                    last_parent = parent;
                }

                this.query_stack.resize(n + 1);
                this.query_stack[n].subquery = (owned) subquery;
                this.query_stack[n].matches = (owned) matches;

                continue;
            }

            GLib.FileInfo[] candidates = {};
            if (this.query_stack.length > n && (subquery.has_prefix(this.query_stack[n].subquery) || this.query_stack[n].subquery != "..")) {
                // New query is a refinement of the current query so we can copy the existing matches as a
                // starting point.
                candidates = this.query_stack[n].matches;
            } else {
                // New query is not a refinement of the current query so we cannot reuse the existing list.
                foreach (GLib.FileInfo parentinfo in this.query_stack[n-1].matches) {
                    GLib.FileType parenttype = parentinfo.get_file_type();
                    if (parenttype != DIRECTORY && parenttype != SYMBOLIC_LINK) {
                        continue;
                    }

                    GLib.File parent = parentinfo.get_attribute_object("standard::file") as GLib.File;
                    try {
                        var enumerator = yield parent.enumerate_children_async(ATTRIBUTES, NONE, GLib.Priority.DEFAULT, this.query_cancellable);
                        while (true) {
                            var fileinfos = yield enumerator.next_files_async(64, GLib.Priority.DEFAULT, this.query_cancellable);
                            if (fileinfos.length() == 0) {
                                break;
                            }
                            foreach (var fileinfo in fileinfos) {
                                var file = enumerator.get_child(fileinfo);
                                fileinfo.set_attribute_object("standard::file", file);

                                candidates += fileinfo;
                            }
                        }
                        yield enumerator.close_async(GLib.Priority.DEFAULT, this.query_cancellable);
                    } catch {
                        // TODO
                        return;
                    }
                }
            }

            // TODO Filter to only include matches and sort by match quality.
            // When sorting, files should be kept together with other files with the same parent directory.

            GLib.FileInfo[] matches = {};
            foreach (var candidate in candidates) {
                if (!candidate.get_name().has_prefix(subquery)) {
                    continue;
                }
                matches += candidate;
            }

            // Changing a stack entry invalidates all higher entries.
            this.query_stack.resize(n + 1);
            this.query_stack[n].subquery = (owned) subquery;
            this.query_stack[n].matches = (owned) matches;
        }

        // Remove any unused entries from the query cache as these were derived from a different query.
        this.query_stack_truncate(n + 1);

        this.list_store.splice(0, this.list_store.get_n_items(), this.query_stack[n].matches);
    }

    /* === View =========================================================================================== */

    [GtkChild]
    private unowned Gtk.ListView list_view;

    private void
    view_init() {
        var factory = new Gtk.SignalListItemFactory();
        factory.setup.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            var label = new Gtk.Label("");
            label.halign = START;
            listitem.child = label;
        });
        factory.bind.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            Gtk.Label label = (Gtk.Label) listitem.child;
            GLib.FileInfo info = (GLib.FileInfo) listitem.item;
            label.label = info.get_display_name();
        });
        this.list_view.factory = factory;

        this.list_view.model = this.selection_model;
    }

    /* === Lifecycle ====================================================================================== */

    class construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
    }

    construct {
        this.list_store = new GLib.ListStore(typeof(GLib.FileInfo));
        this.selection_model = new Gtk.SingleSelection(this.list_store);

        this.notify["root-directory"].connect((fv, pspec) => {
            this.query_stack.resize(0);
            this.update_matches();
        });
        this.notify["query"].connect((fv, pspec) => {
            this.update_matches();
        });

        this.view_init();
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogFilterView));
        base.dispose();
    }
}

