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

private class Bedit.TabPageBin : Gtk.Widget {
    Gtk.Widget? _child;
    public Gtk.Widget? child {
        get { return this._child; }
        set {
            return_if_fail(value == null || value.parent == null);

            if (child == this._child) {
                return;
            }

            if (this._child != null) {
                this._child.unparent();
            }

            this._child = child;
            this._child.set_parent(this);
        }
    }

    static construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
        set_css_name("tabpage");
        set_accessible_role(TAB_PANEL);
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.HORIZONTAL, -1);
    }

    public override void
    compute_expand_internal(out bool hexpand, out bool vexpand) {
        hexpand = false;
        vexpand = false;
        if (this.child != null) {
            hexpand = this.child.compute_expand(Gtk.Orientation.HORIZONTAL);
            vexpand = this.child.compute_expand(Gtk.Orientation.VERTICAL);
        }
    }

    public override bool
    focus(Gtk.DirectionType direction) {
        if (this.child == null) {
            return false;
        }

        return this.child.focus(direction);
    }
}


public class Bedit.TabPage : GLib.Object {
    internal Bedit.TabPageBin bin = new Bedit.TabPageBin();
//    internal GLib.WeakRef last_focus;

    /**
     * The child widget that this page wraps.
     */
    public Gtk.Widget child {
        get { return this.bin.child; }
        construct { this.bin.child = value; }
    }

    /**
     * The tab that this page was created from.
     *
     * As an example, if you were to follow a hyperlink on a page then the new
     * page would be a child of the original one and would be inserted
     * immediately after it.
     */
    public Bedit.TabPage parent { get; construct; }

    /**
     */
    public bool selected { get; }

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
    public GLib.Icon indicator_icon { get; set; }

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

    internal TabPage(Gtk.Widget child) {
        Object(child: child);
    }
}

private class Bedit.TabPageStack : Gtk.Widget {
    private GLib.ListStore children = new GLib.ListStore(typeof(Bedit.TabPage));

    internal int n_pages { get; private set; }

    internal Gtk.SelectionModel pages { owned get; private set; }

    private Bedit.TabPage? _selected_page;
    internal Bedit.TabPage? selected_page {
        get { return this._selected_page; }
        set {
            return_if_fail(value == null || value.bin.parent == this);

            var contains_focus = false;

            var old_value = this.selected_page;

            if (old_value == value) {
                return;
            }

            if (old_value != null) {
                if (!old_value.bin.in_destruction() && old_value.bin.has_focus) {
                    // TODO save focus.
                    contains_focus = false;
                }

                old_value.bin.set_child_visible(false);
            }

            this._selected_page = value;

            if (value != null) {
                if (!this.in_destruction()) {
                    value.bin.set_child_visible(true);

                    if (contains_focus) {
                        // TODO restore focus.
                    }
                }
            }
        }
    }
    internal bool is_transferring_page { get; private set; }

    public signal bool close_page(Bedit.TabPage page);
    public signal unowned Bedit.TabPageStack? create_window();
    public signal void indicator_activated(Bedit.TabPage page);
    public signal void page_attached(Bedit.TabPage page);
    public signal void page_detached(Bedit.TabPage page);
    public signal void setup_menu(Bedit.TabPage? page);

    static construct {
        set_layout_manager_type(typeof (Gtk.BinLayout));
        set_css_name("tabpages");
    }

    public override bool
    focus(Gtk.DirectionType direction) {
        if (this.selected_page == null) {
            return false;
        }

        // TODO restore focus.
        return this.selected_page.bin.focus(direction);
    }

    internal unowned Bedit.TabPage
    add_page(Gtk.Widget child, Bedit.TabPage? parent) {
        return_val_if_fail(child.parent == null, null);

        var page = new Bedit.TabPage(child);

        // TODO position should depend on parent, on the existing children of
        // the parent, and probably on lots of other subtle things.
        uint index = this.children.n_items;

        this.children.insert(index, page);
        page.bin.set_parent(this);

        if (this.n_pages == 1) {
            this.selected_page = page;
        }

        unowned Bedit.TabPage reference = page;
        return reference;
    }

    public void
    transfer_page(Bedit.TabPage page, Bedit.TabPageStack other_stack) {

    }
}


public class Bedit.TabView : Gtk.Widget {
    internal Bedit.TabPageStack stack = new Bedit.TabPageStack();

    /**
     * The number of pages in the tab view.
     */
    public int n_pages { get; private set; }

    /**
     * A selection model with the tab view's pages.
     *
     * This can be used to keep an up-to-date view. The model also implements
     * [iface@Gtk.SelectionModel] and can be used to track and change the selected
     * page.
     */
    public Gtk.SelectionModel pages { owned get; private set; }

    /**
     * The currently visible page.
     */
    public Bedit.TabPage selected_page { get; set; }

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

    static construct {
        set_layout_manager_type(typeof (Gtk.BoxLayout));
        set_css_name("tabview");
    }

    construct {
        this.update_property(Gtk.AccessibleProperty.ORIENTATION, Gtk.Orientation.VERTICAL, -1);
    }

    public unowned Bedit.TabPage
    add_page(Gtk.Widget child, Bedit.TabPage? parent) {
        return this.stack.add_page(child, parent);
    }

    public void
    transfer_page(Bedit.TabPage page, Bedit.TabView other_view) {
        this.stack.transfer_page(page, other_view.stack);
    }

    public void
    close_page_finish(Bedit.TabPage page, bool should_close) {

    }
}
