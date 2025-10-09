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

    public GLib.File? root_directory { get; set; }

    public string query { get; set; default = "";}

    private GLib.ListStore list_store;
    private Gtk.SingleSelection selection_model;

    private QueryStackEntry[] query_stack;

    private void
    update_matches() {
        var remainder = this.query[:];

        GLib.File? root = this.root_directory;
        string subquery = "";
        if (remainder.has_prefix("~/")) {
            root = GLib.File.new_for_path(GLib.Environment.get_home_dir());
            subquery = "~/";
            remainder = remainder[2:];
        } else if (remainder.has_prefix("./")) {
            remainder = remainder[:2];
        } else if (remainder.has_prefix("/")) {
            root = GLib.File.new_for_path("/");
            subquery = "/";
            remainder = remainder[1:];
        }
        if (root == null) {
            print("no root\n");
            this.query_stack.resize(0);
            this.list_store.remove_all();
            return;
        }
        print("root\n");

        print("subquery: %s, remainder: %s\n", subquery, remainder);

        if (this.query_stack.length == 0 || this.query_stack[1].subquery != subquery) {
            this.query_stack.resize(1);
            this.query_stack[0].subquery = subquery;
            try {
                this.query_stack[0].matches = {root.query_info(ATTRIBUTES, NONE, null)};
            } catch {

            }
        }
        int n = 1;

        do {
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
                remainder = remainder[remainder.length-1:remainder.length-1];
            }

            print("subquery: %s, remainder: %s\n", subquery, remainder);

            if (this.query_stack.length > n && this.query_stack[n].subquery == subquery) {
                // Use cached entry from stack.
                continue;
            }

            // Changing a stack entry invalidates all higher entries.
            // Pop all higher entries but keep the current entry for now in case it can be reused.
            this.query_stack.resize(n + 1);

            if (subquery == "..") {
                this.query_stack[n].subquery = "..";
                this.query_stack[n].matches = {};

                if (this.query_stack[n - 1].matches.length == 0) {
                    continue;
                }

                // Find all parent directories of all previous matches.
                GLib.File last_parent = null;
                try {
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
                    this.query_stack[n].matches += parent.query_info(ATTRIBUTES, NONE, null);
                    last_parent = parent;
                }
                }catch{}
                continue;
            }

            if (this.query_stack[n].subquery == null || !subquery.has_prefix(this.query_stack[n].subquery) || this.query_stack[n].subquery == "..") {
                // New query is not a refinement of the current query.  We can't reuse the existing list.
                this.query_stack[n].matches = {};
                foreach (GLib.FileInfo fileinfo in this.query_stack[n-1].matches) {
                    GLib.FileType filetype = fileinfo.get_file_type();
                    if (filetype != DIRECTORY && filetype != SYMBOLIC_LINK) {
                        continue;
                    }

                    GLib.File file = fileinfo.get_attribute_object("standard::file") as GLib.File;
                    var enumerator = file.enumerate_children(ATTRIBUTES, NONE, null);
                    while (true) {
                        var next = enumerator.next_file(null);
                        if (next == null) {
                            break;
                        }
                        this.query_stack[n].matches += next;
                    }
                }
            }
            this.query_stack[n].subquery = subquery;

            // TODO Filter to only include matches and sort by match quality.
            // When sorting, files should be kept together with other files with the same parent directory.
        } while (remainder != "");

        // Remove any unused entries from the query cache as these were derived from a different query.
        this.query_stack.resize(n + 1);

        foreach (GLib.FileInfo matchinfo in this.query_stack[n].matches) {
            GLib.File match = matchinfo.get_attribute_object("standard::file") as GLib.File;
            print("%s\n", match.get_path());
        }

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

