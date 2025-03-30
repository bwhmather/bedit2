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

/**
 * # CSS nodes
 *
 * ```
 * tabview[.empty][.single]
 * ├── tabbar
 * ┊   ├── tabbuttons.start
 * ┊   ┊
 * ┊   ├── tabs
 * ┊   ┊    ╰── tab[.selected]
 * ┊   ┊
 * ┊   ╰── tabbuttons.end
 * ┊
 * ╰── tabpages
 *     ╰── tabpage
 *          ╰── <child>
 * ```
 */

[Flags]
public enum Bedit.TabViewShortcuts {
    NONE,
    CONTROL_TAB,
    CONTROL_SHIFT_TAB,
    CONTROL_PAGE_UP,
    CONTROL_PAGE_DOWN,
    CONTROL_HOME,
    CONTROL_END,
    CONTROL_SHIFT_PAGE_UP,
    CONTROL_SHIFT_PAGE_DOWN,
    CONTROL_SHIFT_HOME,
    CONTROL_SHIFT_END,
    ALT_DIGITS,
    ALT_ZERO,
    ALL_SHORTCUTS
}

[GtkTemplate (ui = "/com/bwhmather/Bedit/ui/bedit-tab-page-tab.ui")]
private sealed class Bedit.TabPageTab : Gtk.Widget {
    public unowned Bedit.TabPage page { get; construct; }

    [GtkChild]
    private unowned Gtk.Label label;

    [GtkChild]
    private unowned Gtk.Button close_button;

    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("tab");
        set_accessible_role(TAB);
    }

    construct {
        this.page.bind_property("title", this.label, "label", SYNC_CREATE);
        this.page.notify["selected"].connect((p, pspec) => {
            if (this.page.selected) {
                this.add_css_class("selected");
            } else {
                this.remove_css_class("selected");
            }
        });
        this.close_button.clicked.connect((b) => {
            this.page.close();
        });
    }

    public override void
    dispose() {
        this.dispose_template(typeof(Bedit.TabPageTab));
    }

    internal TabPageTab(Bedit.TabPage page) {
        Object(page: page);
    }
}

private sealed class Bedit.TabPageBin : Gtk.Widget {
    public unowned Bedit.TabPage page { get; construct; }

    static construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
        set_css_name("tabpage");
        set_accessible_role(TAB_PANEL);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.HORIZONTAL, -1);
        this.page.child.set_parent(this);
    }

    public override void
    dispose() {
        while (this.get_last_child() != null) {
            this.get_last_child().unparent();
        }
    }

    public override void
    compute_expand_internal(out bool hexpand, out bool vexpand) {
        hexpand = this.page.child.compute_expand(Gtk.Orientation.HORIZONTAL);
        vexpand = this.page.child.compute_expand(Gtk.Orientation.VERTICAL);
    }

    public override bool
    focus(Gtk.DirectionType direction) {
        return this.page.child.focus(direction);
    }

    internal TabPageBin(Bedit.TabPage page) {
        Object(page: page);
    }
}


public sealed class Bedit.TabPage : GLib.Object {
    internal Bedit.TabPageTab tab;
    internal Bedit.TabPageBin bin;

//    internal GLib.WeakRef last_focus;

    /**
     * The child widget that this page wraps.
     */
    public Gtk.Widget child { get; construct; }

    /**
     * The tab that this page was created from.
     *
     * As an example, if you were to follow a hyperlink on a page then the new
     * page would be a child of the original one and would be inserted
     * immediately after it.
     */
    public Bedit.TabPage parent { get; construct; }

    /**
     * Will be set if this tab page is the currently selected page in its containing
     * view.
     */
    public bool selected { get; internal set; }

    /**
     * Human readable title that will be displayed in the page's tab.
     */
    public string title { get; set; }

    /**
     * Human readable tooltip that will be displayed when a user mouses over
     * the page's tab.
     *
     * Text is encoded using the Pango text markup language.
     */
    public string tooltip { get; set; }

    /**
     * An icon that should be displayed in the page's tab.
     */
    public GLib.Icon? icon { get; set; }

    /**
     * Whether the page is loading.
     *
     * If set to ``TRUE``, the tab will display a spinner in place of the
     * page's icon.
     */
    public bool loading { get; set; }

    public bool closing { get; internal set; }

    /**
     * An indicator icon for the page.
     *
     * A common use case isan audio or camera indicator in a web browser.
     *
     * This will be shown it at the beginning of the tab, alongside the icon
     * representing [property@TabPage:icon] or loading spinner.
     *
     * [property@TabPage:indicator-tooltip] can be used to set the tooltip on the
     * indicator icon.
     *
     * If [property@TabPage:indicator-activatable] is set to `TRUE`, the
     * indicator icon can act as a button.
     */
    public GLib.Icon? indicator_icon { get; set; }

    /**
     * Human readable tooltip that will be displayed when a user mouses over
     * the indicator icon in this page's tab.
     *
     * Text is encoded using the Pango text markup language.
     */
    public string indicator_tooltip { get; set; }

    /**
     * Whether the indicator icon is activatable.
     *
     * If set to `TRUE`, [signal@TabView::indicator-activated] will be emitted
     * when the indicator icon is clicked.
     *
     * Does nothing if [property@TabPage:indicator-icon] is not set.
     */
    public bool indicator_activatable { get; set; }

    /**
     * Whether the page needs attention.
     *
     * This will cause a line to be displayed under the tab representing the
     * page if set to `TRUE`. If the tab is not visible, the corresponding edge
     * of the tab bar will be highlighted.
     */
    public bool needs_attention { get; set; }

    internal signal void close();

    construct {
        this.tab = new Bedit.TabPageTab(this);
        this.bin = new Bedit.TabPageBin(this);
    }

    public override void
    dispose() {
        this.tab = null;
        this.bin = null;
    }

    internal TabPage(Gtk.Widget child) {
        Object(child: child);
    }
}

private sealed class Bedit.TabViewTabs : Gtk.Widget {
    public unowned Bedit.TabView view { get; construct; }

    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("tabs");
        set_accessible_role(TAB_LIST);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.HORIZONTAL, -1);

        this.view.pages.items_changed.connect((position, removed, added) => {
            Gtk.Widget? next = this.get_first_child();
            for (var i = 0; i < position; i++) {
                next = next.get_next_sibling();
            }

            for (var i = 0; i < removed; i++) {
                var target = next;
                next = next.get_next_sibling();
                target.unparent();
            }

            for (var i = 0; i < added; i++) {
                var page = this.view.pages.get_item(position + i) as Bedit.TabPage;
                page.tab.insert_before(this, next);
            }
        });
    }

    public TabViewTabs(Bedit.TabView view) {
        Object(view: view);
    }
}

private sealed class Bedit.TabViewBar : Gtk.Widget {
    public unowned Bedit.TabView view { get; construct; }

    private Bedit.TabViewTabs tabs;

    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("tabbar");
        set_accessible_role(GROUP);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.HORIZONTAL, -1);

        this.tabs = new Bedit.TabViewTabs(this.view);
        this.tabs.insert_after(this, null);
    }

    public TabViewBar(Bedit.TabView view) {
        Object(view: view);
    }
}

private sealed class Bedit.TabViewStack : Gtk.Widget {
    public unowned Bedit.TabView view { get; construct; }

    static construct {
        set_css_name("tabpages");
    }

    construct {
        this.view.pages.items_changed.connect((position, removed, added) => {
            Gtk.Widget? next = this.get_first_child();
            for (var i = 0; i < position; i++) {
                next = next.get_next_sibling();
            }

            for (var i = 0; i < removed; i++) {
                var target = next;
                next = next.get_next_sibling();
                target.unparent();
            }

            for (var i = 0; i < added; i++) {
                var page = this.view.pages.get_item(position + i) as Bedit.TabPage;
                page.bin.insert_before(this, next);
            }
        });

        this.view.notify["selected-page"].connect((v, pspec) => {
            if (this.in_destruction()) {
                return;
            }

            for (var child = this.get_first_child(); child != null; child = child.get_next_sibling()) {
                child.set_child_visible(child == this.view.selected_page.bin);
            }

            this.queue_allocate();
        });
    }

    internal TabViewStack(Bedit.TabView view) {
        Object(view: view);
    }

    public override bool
    focus(Gtk.DirectionType direction) {
        if (this.view.selected_page == null) {
            return false;
        }

        // TODO restore focus.
        return this.view.selected_page.bin.focus(direction);
    }

    public override void
    measure(Gtk.Orientation orientation, int for_size, out int minimum, out int natural, out int minimum_baseline, out int natural_baseline) {
        minimum = 0;
        natural = 0;

        for (var child = this.get_first_child(); child != null; child = child.get_next_sibling()) {
            int child_minimum, child_natural;
            child.measure(
                orientation, for_size,
                out child_minimum, out child_natural,
                null, null
            );

            if (child_minimum > minimum) {
                minimum = child_minimum;
            }
            if (child_natural > natural) {
                natural = child_natural;
            }
        }
        minimum_baseline = -1;
        natural_baseline = -1;
    }

    public override void
    size_allocate (int width, int height, int baseline) {
        for (var child = this.get_first_child(); child != null; child = child.get_next_sibling()) {
            if (child.get_child_visible()) {
                child.allocate(width, height, baseline, null);
            }
        }
    }

    public override void
    snapshot(Gtk.Snapshot snapshot) {
        if (this.view.selected_page != null) {
            this.snapshot_child(this.view.selected_page.bin, snapshot);
        }
    }

    public override Gtk.SizeRequestMode
    get_request_mode() {
        return Gtk.SizeRequestMode.HEIGHT_FOR_WIDTH;
    }

    public override void
    compute_expand_internal(out bool hexpand, out bool vexpand) {
        hexpand = true;
        vexpand = true;
    }
}

public sealed class Bedit.TabView : Gtk.Widget {
    private GLib.ListStore page_list;
    private Gtk.SingleSelection page_selection;

    private Bedit.TabViewBar bar;
    private Bedit.TabViewStack stack;

    /**
     * The number of pages in the tab view.
     */
    public uint n_pages { get { return this.page_list.n_items; } }

    /**
     * A selection model with the tab view's pages.
     *
     * This can be used to keep an up-to-date view. The model also implements
     * [iface@Gtk.SelectionModel] and can be used to track and change the selected
     * page.
     */
    public Gtk.SelectionModel pages { get { return page_selection; } }

    public Bedit.TabPage
    get_page(uint i) {
        return this.page_list.get_item(i) as Bedit.TabPage;
    }

    /**
     * The currently visible page.
     */
    public Bedit.TabPage? selected_page {
        get {
            return this.page_selection.selected_item as Bedit.TabPage?;
        }
        set {
            uint position;
            return_if_fail(this.page_list.find(value, out position));
            this.page_selection.selected = position;
        }
    }

    /**
     * Whether a page is being transferred.
     *
     * This property will be set to `TRUE` when a drag-n-drop tab transfer starts
     * on any `BrkTabView`, and to `FALSE` after it ends.
     *
     * During the transfer, children cannot receive pointer input and a tab can
     * be safely dropped on the tab view.
     */
    public bool is_transferring_page { get; private set; }

    /**
     * Tab context menu model.
     *
     * When a context menu is shown for a tab, it will be constructed from the
     * provided menu model. Use the [signal@TabView::setup-menu] signal to set up
     * the menu actions for the particular tab.
     */
    public GLib.MenuModel menu_model { get; set; }

    /**
     * Requests to close page.
     *
     * Calling this function will result in the [signal@TabView::close-page] signal
     * being emitted for @page. Closing the page can then be confirmed or
     * denied via [method@TabView.close_page_finish].
     *
     * If the page is waiting for a [method@TabView.close_page_finish] call, this
     * function will do nothing.
     *
     * The default handler for [signal@TabView::close-page] will immediately confirm
     * closing the page. This behavior can be changed by registering your own
     * handler for that signal.
     *
     * If @page was selected, another page will be selected instead:
     *
     * If the [property@TabPage:parent] value is `NULL`, the next page will be
     * selected when possible, or if the page was already last, the previous page
     * will be selected instead.
     *
     * If it's not `NULL`, the previous page will be selected if it's a descendant
     * (possibly indirect) of the parent.
     */
    public signal bool close_page(Bedit.TabPage page);

    /**
     * Emitted when a tab should be transferred into a new window.
     *
     * This can happen after a tab has been dropped on desktop.
     *
     * The signal handler is expected to create a new window, position it as
     * needed and return its `BrkTabView` that the page will be transferred into.
     */
    public signal unowned Bedit.TabView? create_window();
    public signal void indicator_activated(Bedit.TabPage page);
    public signal void page_attached(Bedit.TabPage page);
    public signal void page_detached(Bedit.TabPage page);
    public signal void setup_menu(Bedit.TabPage? page);

    private void
    select_prev_page() {
        if (this.page_list.n_items == 0) {
            return;
        }
        var target = this.page_selection.selected;
        if (target == 0) {
            target = this.page_list.n_items;
        }
        target -= 1;
        this.page_selection.selected = target;
    }

    private void
    select_next_page() {
        if (this.page_list.n_items == 0) {
            return;
        }
        var target = this.page_selection.selected;
        target += 1;
        if (target >= this.page_list.n_items) {
            target = 0;
        }
        this.page_selection.selected = target;
    }

    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("tabview");
    }

    construct {
        var shortcut_controller = new Gtk.ShortcutController();
        shortcut_controller.propagation_phase = CAPTURE;
        shortcut_controller.scope = GLOBAL;
        shortcut_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("<Control><Shift>Tab"),
            new Gtk.CallbackAction((v, args) => { this.select_prev_page(); return true; })
        ));
        shortcut_controller.add_shortcut(new Gtk.Shortcut(
            Gtk.ShortcutTrigger.parse_string("<Control>Tab"),
            new Gtk.CallbackAction((v, args) => { this.select_next_page(); return true; })
        ));
        this.add_controller(shortcut_controller);

        this.page_list = new GLib.ListStore(typeof(Bedit.TabPage));
        this.page_list.notify["n-items"].connect((l, pspec) => { this.notify_property("n-pages"); });

        this.page_selection = new Gtk.SingleSelection(this.page_list);
        this.page_selection.can_unselect = false;
        this.page_selection.autoselect = true;
        this.page_selection.notify["selected"].connect((s, pspec) => {this.notify_property("selected-page");});

        this.notify["selected-page"].connect((s, pspec) => {
            for (var i = 0; i < this.n_pages; i++) {
                var page = this.get_page(i);
                page.selected = page == this.selected_page;
            }
        });

        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.VERTICAL, -1);

        var layout_manager = this.layout_manager as Gtk.BoxLayout;
        layout_manager.orientation = VERTICAL;

        this.bar = new Bedit.TabViewBar(this);
        this.bar.insert_before(this, null);

        this.stack = new Bedit.TabViewStack(this);
        this.stack.insert_before(this, null);
    }

    private void
    bind_page(Bedit.TabPage page) {
        page.close.connect((p) => this.close_page(p));
    }

    private void
    unbind_page(Bedit.TabPage page) {
        page.selected = false;
        GLib.SignalHandler.disconnect_by_data(page, this);
    }

    public unowned Bedit.TabPage
    add_page(Gtk.Widget child, Bedit.TabPage? parent) {
        return_val_if_fail(child.parent == null, null);

        var page = new Bedit.TabPage(child);

        // TODO position should depend on parent, on the existing children of
        // the parent, and probably on lots of other subtle things.
        uint index = this.page_list.n_items;

        this.page_list.insert(index, page);
        this.bind_page(page);

        if (this.page_list.n_items == 1) {
            this.selected_page = page;
        }

        unowned Bedit.TabPage reference = page;
        return reference;
    }

    /**
     * Transfers @page from @self to @other_view.
     *
     * The @page object will be reused.
     */
    public void
    transfer_page(Bedit.TabPage page, Bedit.TabView other_view) {
    }

    /**
     * Completes a [method:@TabView.close_page] call for @page.
     *
     * If @confirm is `TRUE`, @page will be closed.  If it's `FALSE`, it will be
     * reverted to its previous state and [method@TabView.close_page] can be called
     * for it again.
     *
     * This functions should not be called unless a custom handler for
     * [signal@TabView::close-page] is used.
     */
    public void
    close_page_finish(Bedit.TabPage page, bool should_close) {
        uint position;
        return_if_fail(this.page_list.find(page, out position));

        page.closing = false;
        if (!should_close) {
            return;
        }

        this.unbind_page(page);
        this.page_list.remove(position);
    }
}
