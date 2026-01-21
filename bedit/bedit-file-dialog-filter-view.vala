/*
 * Copyright (c) 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

private const string ATTRIBUTES = "standard::*,time::modified";

private struct QueryStackEntry {
    string subquery;
    GLib.FileInfo[] matches;
}

[GtkTemplate ( ui = "/com/bwhmather/Bedit/ui/bedit-file-dialog-filter-view.ui")]
internal sealed class Bedit.FileDialogFilterView : Gtk.Widget {

    /* === State ========================================================================================== */

    public GLib.File? root_directory { get; set; default = null; }
    public string query { get; set; default = "";}
    public bool show_binary { get; set; }
    public bool show_hidden { get; set; }

    public bool loading { get; private set; default = false; }
    public GLib.FileInfo? selection { get { return this.selection_model.selected_item as GLib.FileInfo?; }}

    private GLib.ListStore list_store;
    private Gtk.SingleSelection selection_model;

    private QueryStackEntry[] query_stack = null;
    private void
    query_stack_truncate(int n) {
        for (var i = n; i < this.query_stack.length; i++) {
            this.query_stack[i] = {};
        }
        this.query_stack.resize(n);
    }

    private async void
    update_matches(GLib.Cancellable cancellable) {
        // Rules:
        //  1. Before every `yield`, the query stack must be left in a valid state.
        //  2. After every `yield` the state of `cancellable` must be checked and no further changes must be
        //     made if set.
        try {
            GLib.File? root = this.root_directory;
            string query = this.query;
            bool show_binary = this.show_binary;
            bool show_hidden = this.show_hidden;

            int n = 0;
            string remainder = query[:];

            string subquery = null;
            if (remainder.has_prefix("~/")) {
                root = GLib.File.new_for_path(GLib.Environment.get_home_dir());
                subquery = "~";
                remainder = remainder[2:];
            } else if (remainder.has_prefix("./")) {
                subquery = ".";
                remainder = remainder[2:];
            } else if (remainder.has_prefix("/")) {
                root = GLib.File.new_for_path("/");
                subquery = "";
                remainder = remainder[1:];
            }
            if (root == null) {
                this.query_stack_truncate(0);
                this.list_store.remove_all();
                this.selection_model.selected = 0;
                return;
            }

            this.loading = true;

            if (this.query_stack.length <= n || this.query_stack[n].subquery != subquery) {
                this.query_stack_truncate(n);  // Truncate before doing anything else to leave stack in valid state.

                var rootinfo = yield root.query_info_async(
                    ATTRIBUTES, NONE, GLib.Priority.DEFAULT, cancellable
                );

                rootinfo.set_attribute_object("standard::file", root);

                string markup = subquery == null ? "" : GLib.Markup.printf_escaped("<b>%s</b>", subquery);
                rootinfo.set_attribute_string("bedit::markup", markup);

                this.query_stack.resize(n + 1);
                this.query_stack[n].subquery = subquery;
                this.query_stack[n].matches = {rootinfo};
            }

            bool done = false;
            while (!done) {
                n += 1;

                // Subquery is from the beginning of the remaining query to either the next `/` or the very
                // end.  We need to read the next segment here and consume any following slashes. Segments can
                // be empty if the query ends with a trailing /.
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
                        if (last_parent != null && parent.equal(last_parent)) {
                            // Matches should already be sorted by parent directory so deduplicating just a
                            // matter of tracking it.
                            continue;
                        }

                        var parentinfo = yield parent.query_info_async(
                            ATTRIBUTES, NONE, GLib.Priority.DEFAULT, cancellable
                        );
                        parentinfo.set_attribute_object("standard::file", parent);

                        string markup = matchinfo.get_attribute_string("bedit::markup");
                        parentinfo.set_attribute_string("bedit::markup", markup != "" ? markup + "<b>/..</b>" : "<b>..</b>");

                        matches += parentinfo;
                        last_parent = parent;
                    }

                    this.query_stack.resize(n + 1);
                    this.query_stack[n].subquery = (owned) subquery;
                    this.query_stack[n].matches = (owned) matches;

                    continue;
                }

                GLib.FileInfo[] candidates = {};
                if (this.query_stack.length > n && (
                    subquery.has_prefix(this.query_stack[n].subquery) ||
                    this.query_stack[n].subquery == ".."
                )) {
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
                        var enumerator = yield parent.enumerate_children_async(
                            ATTRIBUTES, NONE, GLib.Priority.DEFAULT, cancellable
                        );
                        while (true) {
                            var fileinfos = yield enumerator.next_files_async(
                                64, GLib.Priority.DEFAULT, cancellable
                            );
                            if (fileinfos.length() == 0) {
                                break;
                            }
                            foreach (var fileinfo in fileinfos) {
                                var file = enumerator.get_child(fileinfo);
                                fileinfo.set_attribute_object("standard::file", file);

                                candidates += fileinfo;
                            }
                        }
                        yield enumerator.close_async(GLib.Priority.DEFAULT, cancellable);
                    }
                }

                // TODO Filter to only include matches and sort by match quality.
                // When sorting, files should be kept together with other files with the same parent
                // directory.

                GLib.FileInfo[] matches = {};
                var parent_index = 0;
                foreach (var candidate in candidates) {
                    if (!show_hidden && candidate.get_is_hidden()) {
                        continue;
                    }

                    string name = candidate.get_name();
                    if (!name.has_prefix(subquery)) {
                        continue;
                    }

                    // We need to dup in order to invalidate the list view items.
                    var match = candidate.dup();
                    var file = match.get_attribute_object("standard::file") as GLib.File;

                    string markup = "";
                    while (true) {
                        var parentinfo = this.query_stack[n - 1].matches[parent_index];
                        var parentfile = parentinfo.get_attribute_object("standard::file") as GLib.File;
                        if (parentfile.equal(file.get_parent())) {
                            markup = parentinfo.get_attribute_string("bedit::markup");
                            break;
                        }
                        parent_index++;
                    }

                    markup = markup == "" ? "" : markup + "<b>/</b>";
                    markup = markup + GLib.Markup.printf_escaped("<b>%s</b>%s", subquery, name[subquery.length:]);
                    match.set_attribute_string("bedit::markup", markup);

                    matches += match;
                }

                // Changing a stack entry invalidates all higher entries.
                this.query_stack.resize(n + 1);
                this.query_stack[n].subquery = (owned) subquery;
                this.query_stack[n].matches = (owned) matches;
            }

            // Remove any unused entries from the query cache as these were derived from a different query.
            this.query_stack_truncate(n + 1);
            this.list_store.splice(0, this.list_store.get_n_items(), this.query_stack[n].matches);
            this.selection_model.selected = 0;
            this.loading = false;

        } catch (GLib.IOError.CANCELLED _) {
            // We don't know if another query is going to be triggered so can't
            // safely unset loading or do any other cleanup here.  All we can
            // do is make sure all cancellable operations levae the view in a
            // consistent state and let the cancelling code clean up.
            return;
        } catch {
            this.query_stack_truncate(0);
            this.list_store.remove_all();
            this.selection_model.selected = 0;
            this.loading = false;
            // TODO
            return;
        }
    }

    private GLib.Cancellable query_cancellable = null;
    private void
    update() {
        if (this.query_cancellable != null) {
            this.query_cancellable.cancel();
            this.query_cancellable = null;
        }

        if (!this.get_mapped() || this.root_directory == null || this.query == "") {
            this.list_store.remove_all();
            this.selection_model.selected = 0;
            this.loading = false;
            return;
        }

        this.query_cancellable = new GLib.Cancellable();
        this.update_matches.begin(this.query_cancellable, (_, res) => {
            this.update_matches.end(res);
        });
    }

    /* === View =========================================================================================== */

    [GtkChild]
    private unowned Gtk.ListView list_view;

    private void
    view_init() {
        var factory = new Gtk.SignalListItemFactory();
        factory.setup.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            listitem.child = new Bedit.FileThumbnail();
        });
        factory.bind.connect((listitem_) => {
            var listitem = (Gtk.ListItem) listitem_;
            var thumbnail = (Bedit.FileThumbnail) listitem.child;
            thumbnail.fileinfo = (GLib.FileInfo) listitem.item;
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
        this.selection_model.notify["selected-item"].connect((sm, pspec) => {
            this.notify_property("selection");
        });

        this.notify["root-directory"].connect((fv, pspec) => {
            this.query_stack_truncate(0);
            this.update();
        });
        this.notify["query"].connect((fv, pspec) => {
            this.update();
        });
        this.notify["show-binary"].connect((fv, pspec) => {
            this.query_stack_truncate(0);
            this.update();
        });
        this.notify["show-hidden"].connect((fv, pspec) => {
            this.query_stack_truncate(0);
            this.update();
        });
        this.map.connect(() => {
            this.update();
        });
        this.unmap.connect(() => {
            this.update();
        });
        this.update();

        this.view_init();
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.FileDialogFilterView));
        base.dispose();
    }

    public void
    select_prev() {
        this.selection_model.selected -= 1;
    }

    public void
    select_next() {
        this.selection_model.selected += 1;
    }
}

