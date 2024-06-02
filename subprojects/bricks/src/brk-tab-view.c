/*
 * Copyright (C) 2020-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "brk-tab-view-private.h"

#include "brk-bin.h"
#include "brk-gizmo-private.h"
#include "brk-marshalers.h"
#include "brk-widget-utils-private.h"

/* FIXME replace with groups */
static GSList *tab_view_list;

#define MIN_ASPECT_RATIO 0.8
#define MAX_ASPECT_RATIO 2.7
#define MIN_THUMBNAIL_BITMAP_WIDTH 250
#define MAX_THUMBNAIL_BITMAP_WIDTH 500
#define MIN_THUMBNAIL_BITMAP_HEIGHT 200
#define MAX_THUMBNAIL_BITMAP_HEIGHT 600

/**
 * BrkTabView:
 *
 * A dynamic tabbed container.
 *
 * `BrkTabView` is a container which shows one child at a time. While it
 * provides keyboard shortcuts for switching between pages, it does not provide
 * a visible tab switcher and relies on external widgets for that, such as
 * [class@TabBar], [class@TabOverview] and [class@TabButton].
 *
 * `BrkTabView` maintains a [class@TabPage] object for each page, which holds
 * additional per-page properties. You can obtain the `BrkTabPage` for a page
 * with [method@TabView.get_page], and as the return value for
 * [method@TabView.append] and other functions for adding children.
 *
 * `BrkTabView` only aims to be useful for dynamic tabs in multi-window
 * document-based applications, such as web browsers, file managers, text
 * editors or terminals. It does not aim to replace [class@Gtk.Notebook] for use
 * cases such as tabbed dialogs.
 *
 * As such, it does not support disabling page reordering or detaching.
 *
 * `BrkTabView` adds a number of global page switching and reordering shortcuts.
 * The [property@TabView:shortcuts] property can be used to manage them.
 *
 * See [flags@TabViewShortcuts] for the list of the available shortcuts. All of
 * the shortcuts are enabled by default.
 *
 * [method@TabView.add_shortcuts] and [method@TabView.remove_shortcuts] can be
 * used to manage shortcuts in a convenient way, for example:
 *
 * ```c
 * brk_tab_view_remove_shortcuts (view, BRK_TAB_VIEW_SHORTCUT_CONTROL_HOME |
 *                                      BRK_TAB_VIEW_SHORTCUT_CONTROL_END);
 * ```
 *
 * ## CSS nodes
 *
 * `BrkTabView` has a main CSS node with the name `tabview`.
 *
 * ## Accessibility
 *
 * `BrkTabView` uses the `GTK_ACCESSIBLE_ROLE_TAB_PANEL` for the tab pages which
 * are the accessible parent objects of the child widgets.
 */

/**
 * BrkTabPage:
 *
 * An auxiliary class used by [class@TabView].
 */

/**
 * BrkTabViewShortcuts:
 * @BRK_TAB_VIEW_SHORTCUT_NONE: No shortcuts
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_TAB:
 *   <kbd>Ctrl</kbd>+<kbd>Tab</kbd> - switch to the next page
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_TAB:
 *   <kbd>Shift</kbd>+<kbd>Ctrl</kbd>+<kbd>Tab</kbd> - switch to the previous
 *   page
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP:
 *   <kbd>Ctrl</kbd>+<kbd>Page Up</kbd> - switch to the previous page
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN:
 *   <kbd>Ctrl</kbd>+<kbd>Page Down</kbd> - switch to the next page
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_HOME:
 *   <kbd>Ctrl</kbd>+<kbd>Home</kbd> - switch to the first page
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_END:
 *   <kbd>Ctrl</kbd>+<kbd>End</kbd> - switch to the last page
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Page Up</kbd> - move the selected
 *   page backward
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Page Down</kbd> - move the selected
 *   page forward
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Home</kbd> - move the selected page
 *   at the start
 * @BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END:
 *   <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>End</kbd> - move the current page at
 *   the end
 * @BRK_TAB_VIEW_SHORTCUT_ALT_DIGITS:
 *  <kbd>Alt</kbd>+<kbd>1</kbd>⋯<kbd>9</kbd> - switch to pages 1-9
 * @BRK_TAB_VIEW_SHORTCUT_ALT_ZERO:
 *  <kbd>Alt</kbd>+<kbd>0</kbd> - switch to page 10
 * @BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS: All of the shortcuts
 *
 * Describes available shortcuts in an [class@TabView].
 *
 * Shortcuts can be set with [property@TabView:shortcuts], or added/removed
 * individually with [method@TabView.add_shortcuts] and
 * [method@TabView.remove_shortcuts].
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.2
 */

struct _BrkTabPage
{
  GObject parent_instance;

  GtkWidget *bin;
  GtkWidget *child;
  BrkTabPage *parent;
  gboolean selected;
  char *title;
  char *tooltip;
  GIcon *icon;
  gboolean loading;
  GIcon *indicator_icon;
  char *indicator_tooltip;
  gboolean indicator_activatable;
  gboolean needs_attention;
  char *keyword;
  float thumbnail_xalign;
  float thumbnail_yalign;

  GtkWidget *last_focus;
  GBinding *transfer_binding;

  GtkATContext *at_context;

  gboolean closing;
  GdkPaintable *paintable;

  gboolean live_thumbnail;
  gboolean invalidated;
  gboolean in_destruction;
};

static void brk_tab_page_accessible_init (GtkAccessibleInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (BrkTabPage, brk_tab_page, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, brk_tab_page_accessible_init))

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_PARENT,
  PAGE_PROP_SELECTED,
  PAGE_PROP_TITLE,
  PAGE_PROP_TOOLTIP,
  PAGE_PROP_ICON,
  PAGE_PROP_LOADING,
  PAGE_PROP_INDICATOR_ICON,
  PAGE_PROP_INDICATOR_TOOLTIP,
  PAGE_PROP_INDICATOR_ACTIVATABLE,
  PAGE_PROP_NEEDS_ATTENTION,
  PAGE_PROP_KEYWORD,
  PAGE_PROP_THUMBNAIL_XALIGN,
  PAGE_PROP_THUMBNAIL_YALIGN,
  PAGE_PROP_LIVE_THUMBNAIL,
  LAST_PAGE_PROP,
  PAGE_PROP_ACCESSIBLE_ROLE
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _BrkTabView
{
  GtkWidget parent_instance;

  GListStore *children;

  int n_pages;
  BrkTabPage *selected_page;
  GMenuModel *menu_model;
  BrkTabViewShortcuts shortcuts;

  int transfer_count;
  int overview_count;
  gulong unmap_extra_pages_cb;

  GtkSelectionModel *pages;
};

static void brk_tab_view_buildable_init (GtkBuildableIface *iface);
static void brk_tab_view_accessible_init (GtkAccessibleInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (BrkTabView, brk_tab_view, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, brk_tab_view_buildable_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE, brk_tab_view_accessible_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_IS_TRANSFERRING_PAGE,
  PROP_SELECTED_PAGE,
  PROP_MENU_MODEL,
  PROP_SHORTCUTS,
  PROP_PAGES,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_PAGE_ATTACHED,
  SIGNAL_PAGE_DETACHED,
  SIGNAL_PAGE_REORDERED,
  SIGNAL_CLOSE_PAGE,
  SIGNAL_SETUP_MENU,
  SIGNAL_CREATE_WINDOW,
  SIGNAL_INDICATOR_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static gboolean
page_should_be_visible (BrkTabView *view,
                        BrkTabPage *page)
{
  if (!view->overview_count)
    return FALSE;

  return page->live_thumbnail || page->invalidated;
}

static void
set_page_selected (BrkTabPage *self,
                   gboolean    selected)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  selected = !!selected;

  if (self->selected == selected)
    return;

  self->selected = selected;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_SELECTED]);
}

static void set_page_parent (BrkTabPage *self,
                             BrkTabPage *parent);

static void
page_parent_notify_cb (BrkTabPage *self)
{
  BrkTabPage *grandparent = brk_tab_page_get_parent (self->parent);

  self->parent = NULL;

  if (grandparent)
    set_page_parent (self, grandparent);
  else
    g_object_notify_by_pspec (G_OBJECT (self), props[PAGE_PROP_PARENT]);
}

static void
set_page_parent (BrkTabPage *self,
                 BrkTabPage *parent)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));
  g_return_if_fail (parent == NULL || BRK_IS_TAB_PAGE (parent));

  if (self->parent == parent)
    return;

  if (self->parent)
    g_object_weak_unref (G_OBJECT (self->parent),
                         (GWeakNotify) page_parent_notify_cb,
                         self);

  self->parent = parent;

  if (self->parent)
    g_object_weak_ref (G_OBJECT (self->parent),
                       (GWeakNotify) page_parent_notify_cb,
                       self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PAGE_PROP_PARENT]);
}

static void
map_or_unmap_page (BrkTabPage *self)
{
  GtkWidget *parent;
  BrkTabView *view;
  gboolean should_be_visible;

  parent = gtk_widget_get_parent (self->bin);

  if (!BRK_IS_TAB_VIEW (parent))
    return;

  view = BRK_TAB_VIEW (parent);

  if (!view->overview_count || !gtk_widget_get_mapped (GTK_WIDGET (view)))
    return;

  should_be_visible = self == view->selected_page ||
                      page_should_be_visible (view, self);

  if (gtk_widget_get_child_visible (self->bin) == should_be_visible)
    return;

  gtk_widget_set_child_visible (self->bin, should_be_visible);
  gtk_widget_queue_allocate (parent);
}

static void
brk_tab_page_dispose (GObject *object)
{
  BrkTabPage *self = BRK_TAB_PAGE (object);

  self->in_destruction = TRUE;

  set_page_parent (self, NULL);

  g_clear_object (&self->at_context);

  g_clear_object (&self->bin);
  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (brk_tab_page_parent_class)->dispose (object);
}

static void
brk_tab_page_finalize (GObject *object)
{
  BrkTabPage *self = (BrkTabPage *)object;

  g_clear_object (&self->child);
  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->tooltip, g_free);
  g_clear_object (&self->icon);
  g_clear_object (&self->indicator_icon);
  g_clear_pointer (&self->indicator_tooltip, g_free);
  g_clear_pointer (&self->keyword, g_free);
  g_clear_weak_pointer (&self->last_focus);

  G_OBJECT_CLASS (brk_tab_page_parent_class)->finalize (object);
}

static void
brk_tab_page_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  BrkTabPage *self = BRK_TAB_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, brk_tab_page_get_child (self));
    break;

  case PAGE_PROP_PARENT:
    g_value_set_object (value, brk_tab_page_get_parent (self));
    break;

  case PAGE_PROP_SELECTED:
    g_value_set_boolean (value, brk_tab_page_get_selected (self));
    break;

  case PAGE_PROP_TITLE:
    g_value_set_string (value, brk_tab_page_get_title (self));
    break;

  case PAGE_PROP_TOOLTIP:
    g_value_set_string (value, brk_tab_page_get_tooltip (self));
    break;

  case PAGE_PROP_ICON:
    g_value_set_object (value, brk_tab_page_get_icon (self));
    break;

  case PAGE_PROP_LOADING:
    g_value_set_boolean (value, brk_tab_page_get_loading (self));
    break;

  case PAGE_PROP_INDICATOR_ICON:
    g_value_set_object (value, brk_tab_page_get_indicator_icon (self));
    break;

  case PAGE_PROP_INDICATOR_TOOLTIP:
    g_value_set_string (value, brk_tab_page_get_indicator_tooltip (self));
    break;

  case PAGE_PROP_INDICATOR_ACTIVATABLE:
    g_value_set_boolean (value, brk_tab_page_get_indicator_activatable (self));
    break;

  case PAGE_PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, brk_tab_page_get_needs_attention (self));
    break;

  case PAGE_PROP_KEYWORD:
    g_value_set_string (value, brk_tab_page_get_keyword (self));
    break;

  case PAGE_PROP_THUMBNAIL_XALIGN:
    g_value_set_float (value, brk_tab_page_get_thumbnail_xalign (self));
    break;

  case PAGE_PROP_THUMBNAIL_YALIGN:
    g_value_set_float (value, brk_tab_page_get_thumbnail_yalign (self));
    break;

  case PAGE_PROP_LIVE_THUMBNAIL:
    g_value_set_boolean (value, brk_tab_page_get_live_thumbnail (self));
    break;

  case PAGE_PROP_ACCESSIBLE_ROLE:
    g_value_set_enum (value, GTK_ACCESSIBLE_ROLE_TAB_PANEL);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_page_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  BrkTabPage *self = BRK_TAB_PAGE (object);

  switch (prop_id) {
  case PAGE_PROP_CHILD:
    g_set_object (&self->child, g_value_get_object (value));
    brk_bin_set_child (BRK_BIN (self->bin), g_value_get_object (value));
    break;

  case PAGE_PROP_PARENT:
    set_page_parent (self, g_value_get_object (value));
    break;

  case PAGE_PROP_TITLE:
    brk_tab_page_set_title (self, g_value_get_string (value));
    break;

  case PAGE_PROP_TOOLTIP:
    brk_tab_page_set_tooltip (self, g_value_get_string (value));
    break;

  case PAGE_PROP_ICON:
    brk_tab_page_set_icon (self, g_value_get_object (value));
    break;

  case PAGE_PROP_LOADING:
    brk_tab_page_set_loading (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_INDICATOR_ICON:
    brk_tab_page_set_indicator_icon (self, g_value_get_object (value));
    break;

  case PAGE_PROP_INDICATOR_TOOLTIP:
    brk_tab_page_set_indicator_tooltip (self, g_value_get_string (value));
    break;

  case PAGE_PROP_INDICATOR_ACTIVATABLE:
    brk_tab_page_set_indicator_activatable (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_NEEDS_ATTENTION:
    brk_tab_page_set_needs_attention (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_KEYWORD:
    brk_tab_page_set_keyword (self, g_value_get_string (value));
    break;

  case PAGE_PROP_THUMBNAIL_XALIGN:
    brk_tab_page_set_thumbnail_xalign (self, g_value_get_float (value));
    break;

  case PAGE_PROP_THUMBNAIL_YALIGN:
    brk_tab_page_set_thumbnail_yalign (self, g_value_get_float (value));
    break;

  case PAGE_PROP_LIVE_THUMBNAIL:
    brk_tab_page_set_live_thumbnail (self, g_value_get_boolean (value));
    break;

  case PAGE_PROP_ACCESSIBLE_ROLE:
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_page_class_init (BrkTabPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = brk_tab_page_dispose;
  object_class->finalize = brk_tab_page_finalize;
  object_class->get_property = brk_tab_page_get_property;
  object_class->set_property = brk_tab_page_set_property;

  /**
   * BrkTabPage:child: (attributes org.gtk.Property.get=brk_tab_page_get_child)
   *
   * The child of the page.
   */
  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BrkTabPage:parent: (attributes org.gtk.Property.get=brk_tab_page_get_parent)
   *
   * The parent page of the page.
   *
   * See [method@TabView.add_page] and [method@TabView.close_page].
   */
  page_props[PAGE_PROP_PARENT] =
    g_param_spec_object ("parent", NULL, NULL,
                         BRK_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:selected: (attributes org.gtk.Property.get=brk_tab_page_get_selected)
   *
   * Whether the page is selected.
   */
  page_props[PAGE_PROP_SELECTED] =
    g_param_spec_boolean ("selected", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * BrkTabPage:title: (attributes org.gtk.Property.get=brk_tab_page_get_title org.gtk.Property.set=brk_tab_page_set_title)
   *
   * The title of the page.
   *
   * [class@TabBar] will display it in the center of the tab and will use it as a tooltip unless
   * [property@TabPage:tooltip] is set.
   */
  page_props[PAGE_PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:tooltip: (attributes org.gtk.Property.get=brk_tab_page_get_tooltip org.gtk.Property.set=brk_tab_page_set_tooltip)
   *
   * The tooltip of the page.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * If not set, [class@TabBar] and [class@TabOverview] will use
   * [property@TabPage:title] as a tooltip instead.
   */
  page_props[PAGE_PROP_TOOLTIP] =
    g_param_spec_string ("tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:icon: (attributes org.gtk.Property.get=brk_tab_page_get_icon org.gtk.Property.set=brk_tab_page_set_icon)
   *
   * The icon of the page.
   *
   * [class@TabBar] and [class@TabOverview] display the icon next to the title,
   * unless [property@TabPage:loading] is set to `TRUE`.
   */
  page_props[PAGE_PROP_ICON] =
    g_param_spec_object ("icon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:loading: (attributes org.gtk.Property.get=brk_tab_page_get_loading org.gtk.Property.set=brk_tab_page_set_loading)
   *
   * Whether the page is loading.
   *
   * If set to `TRUE`, [class@TabBar] and [class@TabOverview] will display a
   * spinner in place of icon.
   */
  page_props[PAGE_PROP_LOADING] =
    g_param_spec_boolean ("loading", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:indicator-icon: (attributes org.gtk.Property.get=brk_tab_page_get_indicator_icon org.gtk.Property.set=brk_tab_page_set_indicator_icon)
   *
   * An indicator icon for the page.
   *
   * A common use case is an audio or camera indicator in a web browser.
   *
   * [class@TabBar] will show it at the beginning of the tab, alongside icon
   * representing [property@TabPage:icon] or loading spinner.
   *
   * [class@TabOverview] will show it at the at the top part of the thumbnail.
   *
   * [property@TabPage:indicator-tooltip] can be used to set the tooltip on the
   * indicator icon.
   *
   * If [property@TabPage:indicator-activatable] is set to `TRUE`, the
   * indicator icon can act as a button.
   */
  page_props[PAGE_PROP_INDICATOR_ICON] =
    g_param_spec_object ("indicator-icon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:indicator-tooltip: (attributes org.gtk.Property.get=brk_tab_page_get_indicator_tooltip org.gtk.Property.set=brk_tab_page_set_indicator_tooltip)
   *
   * The tooltip of the indicator icon.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * See [property@TabPage:indicator-icon].
   *
   * Since: 1.2
   */
  page_props[PAGE_PROP_INDICATOR_TOOLTIP] =
    g_param_spec_string ("indicator-tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:indicator-activatable: (attributes org.gtk.Property.get=brk_tab_page_get_indicator_activatable org.gtk.Property.set=brk_tab_page_set_indicator_activatable)
   *
   * Whether the indicator icon is activatable.
   *
   * If set to `TRUE`, [signal@TabView::indicator-activated] will be emitted
   * when the indicator icon is clicked.
   *
   * If [property@TabPage:indicator-icon] is not set, does nothing.
   */
  page_props[PAGE_PROP_INDICATOR_ACTIVATABLE] =
    g_param_spec_boolean ("indicator-activatable", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:needs-attention: (attributes org.gtk.Property.get=brk_tab_page_get_needs_attention org.gtk.Property.set=brk_tab_page_set_needs_attention)
   *
   * Whether the page needs attention.
   *
   * [class@TabBar] will display a line under the tab representing the page if
   * set to `TRUE`. If the tab is not visible, the corresponding edge of the tab
   * bar will be highlighted.
   *
   * [class@TabOverview] will display a dot in the corner of the thumbnail if set
   * to `TRUE`.
   *
   * [class@TabButton] will display a dot if any of the pages that aren't
   * selected have this property set to `TRUE`.
   */
  page_props[PAGE_PROP_NEEDS_ATTENTION] =
    g_param_spec_boolean ("needs-attention", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:keyword: (attributes org.gtk.Property.get=brk_tab_page_get_keyword org.gtk.Property.set=brk_tab_page_set_keyword)
   *
   * The search keyboard of the page.
   *
   * [class@TabOverview] can search pages by their keywords in addition to their
   * titles and tooltips.
   *
   * Keywords allow to include e.g. page URLs into tab search in a web browser.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_KEYWORD] =
    g_param_spec_string ("keyword", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:thumbnail-xalign: (attributes org.gtk.Property.get=brk_tab_page_get_thumbnail_xalign org.gtk.Property.set=brk_tab_page_set_thumbnail_xalign)
   *
   * The horizontal alignment of the page thumbnail.
   *
   * If the page is so wide that [class@TabOverview] can't display it completely
   * and has to crop it, horizontal alignment will determine which part of the
   * page will be visible.
   *
   * For example, 0.5 means the center of the page will be visible, 0 means the
   * start edge will be visible and 1 means the end edge will be visible.
   *
   * The default horizontal alignment is 0.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_THUMBNAIL_XALIGN] =
    g_param_spec_float ("thumbnail-xalign", NULL, NULL,
                        0.0, 1.0,
                        0.0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:thumbnail-yalign: (attributes org.gtk.Property.get=brk_tab_page_get_thumbnail_yalign org.gtk.Property.set=brk_tab_page_set_thumbnail_yalign)
   *
   * The vertical alignment of the page thumbnail.
   *
   * If the page is so tall that [class@TabOverview] can't display it completely
   * and has to crop it, vertical alignment will determine which part of the
   * page will be visible.
   *
   * For example, 0.5 means the center of the page will be visible, 0 means the
   * top edge will be visible and 1 means the bottom edge will be visible.
   *
   * The default vertical alignment is 0.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_THUMBNAIL_YALIGN] =
    g_param_spec_float ("thumbnail-yalign", NULL, NULL,
                        0.0, 1.0,
                        0.0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabPage:live-thumbnail: (attributes org.gtk.Property.get=brk_tab_page_get_live_thumbnail org.gtk.Property.set=brk_tab_page_set_live_thumbnail)
   *
   * Whether to enable live thumbnail for this page.
   *
   * When set to `TRUE`, the page's thumbnail in [class@TabOverview] will update
   * immediately when the page is redrawn or resized.
   *
   * If it's set to `FALSE`, the thumbnail will only be live when the page is
   * selected, and otherwise it will be static and will only update when
   * [method@TabPage.invalidate_thumbnail] or
   * [method@TabView.invalidate_thumbnails] is called.
   *
   * Since: 1.3
   */
  page_props[PAGE_PROP_LIVE_THUMBNAIL] =
    g_param_spec_boolean ("live-thumbnail", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);

  g_object_class_override_property (object_class, PAGE_PROP_ACCESSIBLE_ROLE, "accessible-role");
}

static void
brk_tab_page_init (BrkTabPage *self)
{
  self->title = g_strdup ("");
  self->tooltip = g_strdup ("");
  self->indicator_tooltip = g_strdup ("");
  self->thumbnail_xalign = 0;
  self->thumbnail_yalign = 0;
  self->bin = g_object_ref_sink (brk_bin_new ());
}

static GtkATContext *
brk_tab_page_accessible_get_at_context (GtkAccessible *accessible)
{
  BrkTabPage *self = BRK_TAB_PAGE (accessible);

  if (self->in_destruction)
    return NULL;

  if (self->at_context == NULL) {
    GtkAccessibleRole role = GTK_ACCESSIBLE_ROLE_TAB_PANEL;
    GdkDisplay *display;

    if (self->bin != NULL)
      display = gtk_widget_get_display (self->bin);
    else
      display = gdk_display_get_default ();

    self->at_context = gtk_at_context_create (role, accessible, display);

    if (self->at_context == NULL)
      return NULL;
  }

  return g_object_ref (self->at_context);
}

static gboolean
brk_tab_page_accessible_get_platform_state (GtkAccessible              *self,
                                            GtkAccessiblePlatformState  state)
{
  return FALSE;
}

static GtkAccessible *
brk_tab_page_accessible_get_accessible_parent (GtkAccessible *accessible)
{
  BrkTabPage *self = BRK_TAB_PAGE (accessible);
  GtkWidget *parent;

  if (!self->bin)
    return NULL;

  parent = gtk_widget_get_parent (self->bin);

  return GTK_ACCESSIBLE (g_object_ref (parent));
}

static GtkAccessible *
brk_tab_page_accessible_get_first_accessible_child (GtkAccessible *accessible)
{
  BrkTabPage *self = BRK_TAB_PAGE (accessible);

  if (self->bin)
    return GTK_ACCESSIBLE (g_object_ref (self->bin));

  return NULL;
}

static GtkAccessible *
brk_tab_page_accessible_get_next_accessible_sibling (GtkAccessible *accessible)
{
  BrkTabPage *self = BRK_TAB_PAGE (accessible);
  GtkWidget *view = gtk_widget_get_parent (self->bin);
  BrkTabPage *next_page;
  int pos;

  if (!BRK_TAB_VIEW (view))
    return NULL;

  pos = brk_tab_view_get_page_position (BRK_TAB_VIEW (view), self);

  if (pos >= brk_tab_view_get_n_pages (BRK_TAB_VIEW (view)) - 1)
    return NULL;

  next_page = brk_tab_view_get_nth_page (BRK_TAB_VIEW (view), pos + 1);

  return GTK_ACCESSIBLE (g_object_ref (next_page));
}

static gboolean
brk_tab_page_accessible_get_bounds (GtkAccessible *accessible,
                                    int           *x,
                                    int           *y,
                                    int           *width,
                                    int           *height)
{
  BrkTabPage *self = BRK_TAB_PAGE (accessible);

  if (self->bin)
    return gtk_accessible_get_bounds (GTK_ACCESSIBLE (self->bin), x, y, width, height);

  return FALSE;
}

static void
brk_tab_page_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_at_context = brk_tab_page_accessible_get_at_context;
  iface->get_platform_state = brk_tab_page_accessible_get_platform_state;
  iface->get_accessible_parent = brk_tab_page_accessible_get_accessible_parent;
  iface->get_first_accessible_child = brk_tab_page_accessible_get_first_accessible_child;
  iface->get_next_accessible_sibling = brk_tab_page_accessible_get_next_accessible_sibling;
  iface->get_bounds = brk_tab_page_accessible_get_bounds;
}

#define BRK_TYPE_TAB_PAINTABLE (brk_tab_paintable_get_type ())

G_DECLARE_FINAL_TYPE (BrkTabPaintable, brk_tab_paintable, BRK, TAB_PAINTABLE, GObject)

struct _BrkTabPaintable
{
  GObject parent_instance;

  GtkWidget *view;
  BrkTabPage *page;

  GdkPaintable *view_paintable;
  GdkPaintable *child_paintable;

  GdkPaintable *cached_paintable;
  double cached_aspect_ratio;

  gboolean frozen;

  double last_xalign;
  double last_yalign;
};

static void
get_background_color (BrkTabPaintable *self,
                      GdkRGBA         *rgba)
{
  GtkWidget *child = brk_tab_page_get_child (self->page);

  if (brk_widget_lookup_color (child, "window_bg_color", rgba))
    return;

  rgba->red = 1;
  rgba->green = 1;
  rgba->blue = 1;
  rgba->alpha = 1;
}

static void
transform_thumbnail (GtkSnapshot *snapshot,
                     double       width,
                     double       height,
                     double       child_ratio,
                     double       xalign,
                     double       yalign,
                     double      *adjusted_width,
                     double      *adjusted_height)
{
  double snapshot_ratio = width / height;
  double new_width, new_height;

  new_width = width;
  new_height = height;

  if (child_ratio > snapshot_ratio) {
    new_width = height * child_ratio;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT ((float) (width - new_width) * xalign, 0));
  } else if (child_ratio < snapshot_ratio) {
    new_height = width / child_ratio;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT (0, (float) (height - new_height) * yalign));
  }

  if (adjusted_width)
    *adjusted_width = new_width;
  if (adjusted_height)
    *adjusted_height = new_height;
}

static double
get_unclamped_aspect_ratio (BrkTabPaintable *self)
{
  if (!self->view_paintable)
    return self->cached_aspect_ratio;

  return gdk_paintable_get_intrinsic_aspect_ratio (self->view_paintable);
}

static GdkTexture *
render_contents (BrkTabPaintable *self,
                 gboolean         empty)
{
  GtkSnapshot *snapshot;
  GskRenderNode *node;
  double aspect_ratio;
  int scale_factor, width, height;
  GtkNative *native;
  GskRenderer *renderer;
  graphene_rect_t bounds;
  GdkTexture *ret;

  if (self->frozen)
    return NULL;

  aspect_ratio = get_unclamped_aspect_ratio (self);

  if (G_APPROX_VALUE (aspect_ratio, 0, DBL_EPSILON))
    return NULL;

  scale_factor = gtk_widget_get_scale_factor (self->view);
  snapshot = gtk_snapshot_new ();

  if (MAX_THUMBNAIL_BITMAP_WIDTH / aspect_ratio < MIN_THUMBNAIL_BITMAP_HEIGHT) {
    height = MIN_THUMBNAIL_BITMAP_HEIGHT * scale_factor;
    width = ceil (height * aspect_ratio) * scale_factor;
  } else if (MAX_THUMBNAIL_BITMAP_WIDTH / aspect_ratio > MAX_THUMBNAIL_BITMAP_HEIGHT) {
    height = MAX_THUMBNAIL_BITMAP_HEIGHT * scale_factor;
    width = ceil (height * aspect_ratio) * scale_factor;
  } else {
    width = MAX_THUMBNAIL_BITMAP_WIDTH * scale_factor;
    height = ceil (MAX_THUMBNAIL_BITMAP_WIDTH / aspect_ratio) * scale_factor;
  }

  if (width < MIN_THUMBNAIL_BITMAP_WIDTH * scale_factor) {
    width = MIN_THUMBNAIL_BITMAP_WIDTH * scale_factor;
    height = ceil (MIN_THUMBNAIL_BITMAP_WIDTH / aspect_ratio) * scale_factor;
  }

  if (!empty) {
    GdkPaintable *current_paintable;
    GdkRGBA background;

    get_background_color (self, &background);

    gtk_snapshot_append_color (snapshot, &background,
                               &GRAPHENE_RECT_INIT (0, 0, width, height));

    current_paintable = gdk_paintable_get_current_image (self->child_paintable);
    gdk_paintable_snapshot (current_paintable, snapshot, width, height);

    g_object_unref (current_paintable);
  }

  node = gtk_snapshot_free_to_node (snapshot);

  if (!node)
    return NULL;

  native = gtk_widget_get_native (self->view);
  renderer = gtk_native_get_renderer (native);

  graphene_rect_init (&bounds, 0, 0, width, height);
  ret = gsk_renderer_render_texture (renderer, node, &bounds);

  gsk_render_node_unref (node);

  return ret;
}

static void
invalidate_texture (BrkTabPaintable *self)
{
  GdkTexture *texture;
  double old_aspect_ratio;

  if (!self->page->bin || !gtk_widget_get_mapped (self->page->bin))
    return;

  if (self->view) {
    BrkTabView *view = BRK_TAB_VIEW (self->view);

    if (!view->overview_count) {
      brk_tab_page_invalidate_thumbnail (self->page);
      return;
    }
  }

  texture = render_contents (self, FALSE);

  if (!texture)
    return;

  g_clear_object (&self->cached_paintable);
  self->cached_paintable = GDK_PAINTABLE (texture);

  old_aspect_ratio = self->cached_aspect_ratio;
  self->cached_aspect_ratio = get_unclamped_aspect_ratio (self);

  if (G_APPROX_VALUE (old_aspect_ratio, self->cached_aspect_ratio, DBL_EPSILON))
    gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
  else
    gdk_paintable_invalidate_size (GDK_PAINTABLE (self));
}

static void
invalidate_size_cb (BrkTabPaintable *self)
{
  if (!self->cached_paintable)
    self->cached_aspect_ratio = get_unclamped_aspect_ratio (self);

  gdk_paintable_invalidate_size (GDK_PAINTABLE (self));
}

static void
connect_to_view (BrkTabPaintable *self)
{
  if (self->view || !gtk_widget_get_parent (self->page->bin))
    return;

  self->view = gtk_widget_get_parent (self->page->bin);
  self->view_paintable = gtk_widget_paintable_new (self->view);

  g_signal_connect_swapped (self->view_paintable, "invalidate-size",
                            G_CALLBACK (invalidate_size_cb), self);
}

static void
disconnect_from_view (BrkTabPaintable *self)
{
  g_clear_object (&self->view_paintable);
  self->view = NULL;
}

static void
child_parent_changed (BrkTabPaintable *self)
{
  disconnect_from_view (self);
  connect_to_view (self);
}

static double
brk_tab_paintable_get_intrinsic_aspect_ratio (GdkPaintable *paintable)
{
  BrkTabPaintable *self = BRK_TAB_PAINTABLE (paintable);
  double ratio;

  if (self->view_paintable)
    ratio = gdk_paintable_get_intrinsic_aspect_ratio (self->view_paintable);
  else
    ratio = self->cached_aspect_ratio;

  return CLAMP (ratio, MIN_ASPECT_RATIO, MAX_ASPECT_RATIO);
}

static GdkPaintable *
brk_tab_paintable_get_current_image (GdkPaintable *paintable)
{
  BrkTabPaintable *self = BRK_TAB_PAINTABLE (paintable);
  GtkSnapshot *snapshot = gtk_snapshot_new ();
  int width, height;

  if (!self->view)
    return NULL;

  width = gtk_widget_get_width (self->view);
  height = gtk_widget_get_height (self->view);

  gdk_paintable_snapshot (paintable, GDK_SNAPSHOT (snapshot), width, height);

  return gtk_snapshot_free_to_paintable (snapshot,
                                         &GRAPHENE_SIZE_INIT (width, height));
}

static void
brk_tab_paintable_snapshot (GdkPaintable *paintable,
                            GdkSnapshot  *snapshot,
                            double        width,
                            double        height)
{
  BrkTabPaintable *self = BRK_TAB_PAINTABLE (paintable);
  GtkWidget *child;
  double xalign, yalign;

  if (self->frozen) {
    xalign = self->last_xalign;
    yalign = self->last_yalign;
    child = NULL;
  } else {
    xalign = brk_tab_page_get_thumbnail_xalign (self->page);
    yalign = brk_tab_page_get_thumbnail_yalign (self->page);
    child = self->page->bin;

    if (gtk_widget_get_direction (child) == GTK_TEXT_DIR_RTL)
      xalign = 1 - xalign;
  }

  if (!self->cached_paintable) {
    return;
  }

  transform_thumbnail (snapshot, width, height, self->cached_aspect_ratio,
                       xalign, yalign, &width, &height);

  gdk_paintable_snapshot (self->cached_paintable, snapshot, width, height);
}

static void
brk_tab_paintable_iface_init (GdkPaintableInterface *iface)
{
  iface->get_intrinsic_aspect_ratio = brk_tab_paintable_get_intrinsic_aspect_ratio;
  iface->get_current_image = brk_tab_paintable_get_current_image;
  iface->snapshot = brk_tab_paintable_snapshot;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (BrkTabPaintable, brk_tab_paintable, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GDK_TYPE_PAINTABLE, brk_tab_paintable_iface_init))

static void
brk_tab_paintable_dispose (GObject *object)
{
  BrkTabPaintable *self = BRK_TAB_PAINTABLE (object);

  disconnect_from_view (self);

  g_clear_object (&self->child_paintable);
  g_clear_object (&self->cached_paintable);

  G_OBJECT_CLASS (brk_tab_paintable_parent_class)->dispose (object);
}

static void
brk_tab_paintable_class_init (BrkTabPaintableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = brk_tab_paintable_dispose;
}

static void
brk_tab_paintable_init (BrkTabPaintable *self)
{
}

static GdkPaintable *
brk_tab_paintable_new (BrkTabPage *page)
{
  BrkTabPaintable *self = g_object_new (BRK_TYPE_TAB_PAINTABLE, NULL);

  self->page = page;

  connect_to_view (self);

  self->child_paintable = gtk_widget_paintable_new (page->bin);

  g_signal_connect_swapped (self->child_paintable, "invalidate-contents",
                            G_CALLBACK (invalidate_texture), self);

  g_signal_connect_object (self->page, "notify::thumbnail-xalign",
                           G_CALLBACK (gdk_paintable_invalidate_contents), self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->page, "notify::thumbnail-yalign",
                           G_CALLBACK (gdk_paintable_invalidate_contents), self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (page->bin, "notify::parent",
                           G_CALLBACK (child_parent_changed), self,
                           G_CONNECT_SWAPPED);

  return GDK_PAINTABLE (self);
}

static void
brk_tab_paintable_freeze (BrkTabPaintable *self)
{
  self->last_xalign = brk_tab_page_get_thumbnail_xalign (self->page);
  self->last_yalign = brk_tab_page_get_thumbnail_yalign (self->page);

  if (!self->cached_paintable)
    self->cached_paintable = GDK_PAINTABLE (render_contents (self, TRUE));

  if (gtk_widget_get_direction (self->page->bin) == GTK_TEXT_DIR_RTL)
    self->last_xalign = 1 - self->last_xalign;

  self->frozen = TRUE;

  g_clear_object (&self->child_paintable);
}

#define BRK_TYPE_TAB_PAGES (brk_tab_pages_get_type ())

G_DECLARE_FINAL_TYPE (BrkTabPages, brk_tab_pages, BRK, TAB_PAGES, GObject)

struct _BrkTabPages
{
  GObject parent_instance;

  BrkTabView *view;
};

static GType
brk_tab_pages_get_item_type (GListModel *model)
{
  return BRK_TYPE_TAB_PAGE;
}

static guint
brk_tab_pages_get_n_items (GListModel *model)
{
  BrkTabPages *self = BRK_TAB_PAGES (model);

  if (G_UNLIKELY (!BRK_IS_TAB_VIEW (self->view)))
    return 0;

  return self->view->n_pages;
}

static gpointer
brk_tab_pages_get_item (GListModel *model,
                        guint       position)
{
  BrkTabPages *self = BRK_TAB_PAGES (model);
  BrkTabPage *page;

  if (G_UNLIKELY (!BRK_IS_TAB_VIEW (self->view)))
    return NULL;

  page = brk_tab_view_get_nth_page (self->view, position);

  if (!page)
    return NULL;

  return g_object_ref (page);
}

static void
brk_tab_pages_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = brk_tab_pages_get_item_type;
  iface->get_n_items = brk_tab_pages_get_n_items;
  iface->get_item = brk_tab_pages_get_item;
}

static gboolean
brk_tab_pages_is_selected (GtkSelectionModel *model,
                           guint              position)
{
  BrkTabPages *self = BRK_TAB_PAGES (model);
  BrkTabPage *page;

  if (G_UNLIKELY (!BRK_IS_TAB_VIEW (self->view)))
    return FALSE;

  page = brk_tab_view_get_nth_page (self->view, position);

  return page->selected;
}

static gboolean
brk_tab_pages_select_item (GtkSelectionModel *model,
                           guint              position,
                           gboolean           exclusive)
{
  BrkTabPages *self = BRK_TAB_PAGES (model);
  BrkTabPage *page;

  if (G_UNLIKELY (!BRK_IS_TAB_VIEW (self->view)))
    return FALSE;

  page = brk_tab_view_get_nth_page (self->view, position);

  brk_tab_view_set_selected_page (self->view, page);

  return TRUE;
}

static void
brk_tab_pages_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = brk_tab_pages_is_selected;
  iface->select_item = brk_tab_pages_select_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (BrkTabPages, brk_tab_pages, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, brk_tab_pages_list_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, brk_tab_pages_selection_model_init))

static void
brk_tab_pages_init (BrkTabPages *self)
{
}

static void
brk_tab_pages_dispose (GObject *object)
{
  BrkTabPages *self = BRK_TAB_PAGES (object);

  g_clear_weak_pointer (&self->view);

  G_OBJECT_CLASS (brk_tab_pages_parent_class)->dispose (object);
}

static void
brk_tab_pages_class_init (BrkTabPagesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = brk_tab_pages_dispose;
}

static GtkSelectionModel *
brk_tab_pages_new (BrkTabView *view)
{
  BrkTabPages *pages;

  pages = g_object_new (BRK_TYPE_TAB_PAGES, NULL);
  g_set_weak_pointer (&pages->view, view);

  return GTK_SELECTION_MODEL (pages);
}

static gboolean
object_handled_accumulator (GSignalInvocationHint *ihint,
                            GValue                *return_accu,
                            const GValue          *handler_return,
                            gpointer               data)
{
  GObject *object = g_value_get_object (handler_return);

  g_value_set_object (return_accu, object);

  return !object;
}

static void
begin_transfer_for_group (BrkTabView *self)
{
  GSList *l;

  for (l = tab_view_list; l; l = l->next) {
    BrkTabView *view = l->data;

    view->transfer_count++;

    if (view->transfer_count == 1)
      g_object_notify_by_pspec (G_OBJECT (view), props[PROP_IS_TRANSFERRING_PAGE]);
  }
}

static void
end_transfer_for_group (BrkTabView *self)
{
  GSList *l;

  for (l = tab_view_list; l; l = l->next) {
    BrkTabView *view = l->data;

    view->transfer_count--;

    if (view->transfer_count == 0)
      g_object_notify_by_pspec (G_OBJECT (view), props[PROP_IS_TRANSFERRING_PAGE]);
  }
}

static void
set_n_pages (BrkTabView *self,
             int         n_pages)
{
  if (n_pages == self->n_pages)
    return;

  self->n_pages = n_pages;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static inline gboolean
page_belongs_to_this_view (BrkTabView *self,
                           BrkTabPage *page)
{
  if (!page)
    return FALSE;

  return gtk_widget_get_parent (page->bin) == GTK_WIDGET (self);
}

static inline gboolean
child_belongs_to_this_view (BrkTabView *self,
                            GtkWidget  *child)
{
  GtkWidget *parent;

  if (!child)
    return FALSE;

  parent = gtk_widget_get_parent (child);

  if (!parent)
    return FALSE;

  return gtk_widget_get_parent (parent) == GTK_WIDGET (self);
}

static inline gboolean
is_descendant_of (BrkTabPage *page,
                  BrkTabPage *parent)
{
  while (page && page != parent)
    page = brk_tab_page_get_parent (page);

  return page == parent;
}

static void
attach_page (BrkTabView *self,
             BrkTabPage *page,
             int         position)
{
  BrkTabPage *parent;

  g_list_store_insert (self->children, position, page);

  gtk_widget_set_child_visible (page->bin,
                                page_should_be_visible (self, page));
  gtk_widget_set_parent (page->bin, GTK_WIDGET (self));
  page->transfer_binding =
    g_object_bind_property (self, "is-transferring-page",
                            page->bin, "can-target",
                            G_BINDING_SYNC_CREATE |
                            G_BINDING_INVERT_BOOLEAN);
  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_freeze_notify (G_OBJECT (self));

  set_n_pages (self, self->n_pages + 1);

  g_object_thaw_notify (G_OBJECT (self));

  parent = brk_tab_page_get_parent (page);

  if (parent && !page_belongs_to_this_view (self, parent))
    set_page_parent (page, NULL);

  g_signal_emit (self, signals[SIGNAL_PAGE_ATTACHED], 0, page, position);
}

static void
set_selected_page (BrkTabView *self,
                   BrkTabPage *selected_page,
                   gboolean    notify_pages)
{
  guint old_position = GTK_INVALID_LIST_POSITION;
  guint new_position = GTK_INVALID_LIST_POSITION;
  gboolean contains_focus = FALSE;

  if (self->selected_page == selected_page)
    return;

  if (self->selected_page) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
    GtkWidget *focus = root ? gtk_root_get_focus (root) : NULL;

    if (notify_pages && self->pages)
      old_position = brk_tab_view_get_page_position (self, self->selected_page);

    if (!gtk_widget_in_destruction (GTK_WIDGET (self)) &&
        focus &&
        self->selected_page &&
        self->selected_page->bin &&
        gtk_widget_is_ancestor (focus, self->selected_page->bin)) {
      contains_focus = TRUE;

      g_set_weak_pointer (&self->selected_page->last_focus, focus);
    }

    if (self->selected_page->bin && selected_page) {
      gtk_widget_set_child_visible (self->selected_page->bin,
                                    page_should_be_visible (self, self->selected_page));
    }

    set_page_selected (self->selected_page, FALSE);
  }

  self->selected_page = selected_page;

  if (self->selected_page) {
    if (notify_pages && self->pages)
      new_position = brk_tab_view_get_page_position (self, self->selected_page);

    if (!gtk_widget_in_destruction (GTK_WIDGET (self))) {
      gtk_widget_set_child_visible (selected_page->bin, TRUE);

      if (contains_focus) {
        if (selected_page->last_focus)
          gtk_widget_grab_focus (selected_page->last_focus);
        else
          gtk_widget_child_focus (selected_page->bin, GTK_DIR_TAB_FORWARD);
      }

      gtk_widget_queue_allocate (GTK_WIDGET (self));
    }

    set_page_selected (self->selected_page, TRUE);
  }

  if (notify_pages && self->pages) {
    if (old_position == GTK_INVALID_LIST_POSITION && new_position == GTK_INVALID_LIST_POSITION)
      ; /* nothing to do */
    else if (old_position == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, new_position, 1);
    else if (new_position == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, old_position, 1);
    else
      gtk_selection_model_selection_changed (self->pages,
                                             MIN (old_position, new_position),
                                             MAX (old_position, new_position) -
                                             MIN (old_position, new_position) + 1);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_PAGE]);
}

static void
select_previous_page (BrkTabView *self,
                      BrkTabPage *page)
{
  int pos = brk_tab_view_get_page_position (self, page);
  BrkTabPage *parent;

  if (page != self->selected_page)
    return;

  parent = brk_tab_page_get_parent (page);

  if (parent && pos > 0) {
    BrkTabPage *prev_page = brk_tab_view_get_nth_page (self, pos - 1);

    /* This usually means we opened a few pages from the same page in a row, or
     * the previous page is the parent. Switch there. */
    if (is_descendant_of (prev_page, parent)) {
      brk_tab_view_set_selected_page (self, prev_page);

      return;
    }
  }

  if (brk_tab_view_select_next_page (self))
    return;

  brk_tab_view_select_previous_page (self);
}

static void
detach_page (BrkTabView *self,
             BrkTabPage *page,
             gboolean    in_dispose)
{
  int pos = brk_tab_view_get_page_position (self, page);

  select_previous_page (self, page);

  g_object_ref (self);
  g_object_ref (page);
  g_object_ref (page->bin);

  if (self->n_pages == 1)
    set_selected_page (self, NULL, !in_dispose);

  g_list_store_remove (self->children, pos);

  g_object_freeze_notify (G_OBJECT (self));

  set_n_pages (self, self->n_pages - 1);

  g_object_thaw_notify (G_OBJECT (self));

  g_clear_pointer (&page->transfer_binding, g_binding_unbind);
  gtk_widget_unparent (page->bin);

  if (!in_dispose)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_signal_emit (self, signals[SIGNAL_PAGE_DETACHED], 0, page, pos);

  if (!in_dispose && self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), pos, 1, 0);

  g_object_unref (page->bin);
  g_object_unref (page);
  g_object_unref (self);
}

static void
insert_page (BrkTabView *self,
             BrkTabPage *page,
             int         position)
{
  attach_page (self, page, position);

  g_object_freeze_notify (G_OBJECT (self));

  if (!self->selected_page)
    set_selected_page (self, page, FALSE);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 0, 1);

  g_object_thaw_notify (G_OBJECT (self));
}

static BrkTabPage *
create_and_insert_page (BrkTabView *self,
                        GtkWidget  *child,
                        BrkTabPage *parent,
                        int         position)
{
  BrkTabPage *page =
    g_object_new (BRK_TYPE_TAB_PAGE,
                  "child", child,
                  "parent", parent,
                  NULL);

  insert_page (self, page, position);

  g_object_unref (page);

  return page;
}

static gboolean
close_page_cb (BrkTabView *self,
               BrkTabPage *page)
{
  brk_tab_view_close_page_finish (self, page, TRUE);

  return GDK_EVENT_STOP;
}

static gboolean
select_page_cb (GtkWidget  *widget,
                GVariant   *args,
                BrkTabView *self)
{
  BrkTabViewShortcuts mask;
  GtkDirectionType direction;
  gboolean last, success = FALSE;

  if (!brk_tab_view_get_selected_page (self))
    return GDK_EVENT_PROPAGATE;

  if (self->n_pages <= 1)
    return GDK_EVENT_PROPAGATE;

  g_variant_get (args, "(hhb)", &mask, &direction, &last);

  if (!(self->shortcuts & mask))
    return GDK_EVENT_PROPAGATE;

  if (direction == GTK_DIR_TAB_BACKWARD) {
    if (last)
      success = brk_tab_view_select_first_page (self);
    else
      success = brk_tab_view_select_previous_page (self);

    if (!success && !last) {
      BrkTabPage *page = brk_tab_view_get_nth_page (self, self->n_pages - 1);

      brk_tab_view_set_selected_page (self, page);

      success = TRUE;
    }
  } else if (direction == GTK_DIR_TAB_FORWARD) {
    if (last)
      success = brk_tab_view_select_last_page (self);
    else
      success = brk_tab_view_select_next_page (self);

    if (!success && !last) {
      BrkTabPage *page = brk_tab_view_get_nth_page (self, 0);

      brk_tab_view_set_selected_page (self, page);

      success = TRUE;
    }
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));

  return GDK_EVENT_STOP;
}

static inline void
add_switch_shortcut (BrkTabView          *self,
                     GtkEventController  *controller,
                     BrkTabViewShortcuts  mask,
                     guint                keysym,
                     guint                keypad_keysym,
                     GdkModifierType      modifiers,
                     GtkDirectionType     direction,
                     gboolean             last)
{
  GtkShortcutTrigger *trigger;
  GtkShortcutAction *action;
  GtkShortcut *shortcut;

  trigger = gtk_alternative_trigger_new (gtk_keyval_trigger_new (keysym, modifiers),
                                         gtk_keyval_trigger_new (keypad_keysym, modifiers));
  action = gtk_callback_action_new ((GtkShortcutFunc) select_page_cb, self, NULL);
  shortcut = gtk_shortcut_new (trigger, action);

  gtk_shortcut_set_arguments (shortcut, g_variant_new ("(hhb)", mask, direction, last));
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller),
                                        shortcut);
}

static gboolean
reorder_page_cb (GtkWidget  *widget,
                 GVariant   *args,
                 BrkTabView *self)
{
  BrkTabViewShortcuts mask;
  GtkDirectionType direction;
  gboolean last, success = FALSE;
  BrkTabPage *page = brk_tab_view_get_selected_page (self);

  if (!page)
    return GDK_EVENT_PROPAGATE;

  if (self->n_pages <= 1)
    return GDK_EVENT_PROPAGATE;

  g_variant_get (args, "(hhb)", &mask, &direction, &last);

  if (!(self->shortcuts & mask))
    return GDK_EVENT_PROPAGATE;

  if (direction == GTK_DIR_TAB_BACKWARD) {
    if (last)
      success = brk_tab_view_reorder_first (self, page);
    else
      success = brk_tab_view_reorder_backward (self, page);
  } else if (direction == GTK_DIR_TAB_FORWARD) {
    if (last)
      success = brk_tab_view_reorder_last (self, page);
    else
      success = brk_tab_view_reorder_forward (self, page);
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));

  return GDK_EVENT_STOP;
}

static inline void
add_reorder_shortcut (BrkTabView          *self,
                      GtkEventController  *controller,
                      BrkTabViewShortcuts  mask,
                      guint                keysym,
                      guint                keypad_keysym,
                      GtkDirectionType     direction,
                      gboolean             last)
{
  GtkShortcutTrigger *trigger;
  GtkShortcutAction *action;
  GtkShortcut *shortcut;

  trigger = gtk_alternative_trigger_new (gtk_keyval_trigger_new (keysym, GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                                         gtk_keyval_trigger_new (keypad_keysym, GDK_CONTROL_MASK | GDK_SHIFT_MASK));
  action = gtk_callback_action_new ((GtkShortcutFunc) reorder_page_cb, self, NULL);
  shortcut = gtk_shortcut_new (trigger, action);

  gtk_shortcut_set_arguments (shortcut, g_variant_new ("(hhb)", mask, direction, last));
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller),
                                        shortcut);
}

static gboolean
select_nth_page_cb (GtkWidget  *widget,
                    GVariant   *args,
                    BrkTabView *self)
{
  gint8 n_page = g_variant_get_byte (args);
  BrkTabViewShortcuts mask;
  BrkTabPage *page;

  if (n_page >= self->n_pages)
    return GDK_EVENT_PROPAGATE;

  /* Pages are counted from 0, so page 9 represents Alt+0 */
  if (n_page == 9)
    mask = BRK_TAB_VIEW_SHORTCUT_ALT_ZERO;
  else
    mask = BRK_TAB_VIEW_SHORTCUT_ALT_DIGITS;

  if (!(self->shortcuts & mask))
    return GDK_EVENT_PROPAGATE;

  page = brk_tab_view_get_nth_page (self, n_page);
  if (brk_tab_view_get_selected_page (self) == page)
    return GDK_EVENT_PROPAGATE;

  brk_tab_view_set_selected_page (self, page);

  return GDK_EVENT_STOP;
}

static inline void
add_switch_nth_page_shortcut (BrkTabView         *self,
                              GtkEventController *controller,
                              guint               keysym,
                              guint               keypad_keysym,
                              int                 n_page)
{
  GtkShortcutTrigger *trigger;
  GtkShortcutAction *action;
  GtkShortcut *shortcut;

  trigger = gtk_alternative_trigger_new (gtk_keyval_trigger_new (keysym, GDK_ALT_MASK),
                                         gtk_keyval_trigger_new (keypad_keysym, GDK_ALT_MASK));
  action = gtk_callback_action_new ((GtkShortcutFunc) select_nth_page_cb, self, NULL);
  shortcut = gtk_shortcut_new (trigger, action);

  gtk_shortcut_set_arguments (shortcut, g_variant_new_byte (n_page));
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (controller),
                                        shortcut);
}

static void
init_shortcuts (BrkTabView         *self,
                GtkEventController *controller)
{
  int i;

  add_switch_shortcut (self, controller,
                       BRK_TAB_VIEW_SHORTCUT_CONTROL_TAB,
                       GDK_KEY_Tab, GDK_KEY_KP_Tab, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_FORWARD, FALSE);
  add_switch_shortcut (self, controller,
                       BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_TAB,
                       GDK_KEY_Tab, GDK_KEY_KP_Tab, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                       GTK_DIR_TAB_BACKWARD, FALSE);
  add_switch_shortcut (self, controller,
                       BRK_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP,
                       GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_BACKWARD, FALSE);
  add_switch_shortcut (self, controller,
                       BRK_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN,
                       GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_FORWARD, FALSE);
  add_switch_shortcut (self, controller,
                       BRK_TAB_VIEW_SHORTCUT_CONTROL_HOME,
                       GDK_KEY_Home, GDK_KEY_KP_Home, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_BACKWARD, TRUE);
  add_switch_shortcut (self, controller,
                       BRK_TAB_VIEW_SHORTCUT_CONTROL_END,
                       GDK_KEY_End, GDK_KEY_KP_End, GDK_CONTROL_MASK,
                       GTK_DIR_TAB_FORWARD, TRUE);

  add_reorder_shortcut (self, controller,
                        BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP,
                        GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up,
                        GTK_DIR_TAB_BACKWARD, FALSE);
  add_reorder_shortcut (self, controller,
                        BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN,
                        GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down,
                        GTK_DIR_TAB_FORWARD, FALSE);
  add_reorder_shortcut (self, controller,
                        BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME,
                        GDK_KEY_Home, GDK_KEY_KP_Home,
                        GTK_DIR_TAB_BACKWARD, TRUE);
  add_reorder_shortcut (self, controller,
                        BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END,
                        GDK_KEY_End, GDK_KEY_KP_End,
                        GTK_DIR_TAB_FORWARD,  TRUE);

  for (i = 0; i < 10; i++)
    add_switch_nth_page_shortcut (self,
                                  controller,
                                  GDK_KEY_0 + i,
                                  GDK_KEY_KP_0 + i,
                                  (i + 9) % 10); /* Alt+0 means page 10, not 0 */
}

static void
brk_tab_view_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum,
                      int            *natural,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
  BrkTabView *self = BRK_TAB_VIEW (widget);
  int i;

  *minimum = 0;
  *natural = 0;

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);
    int child_min, child_nat;

    gtk_widget_measure (page->bin, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);

    *minimum = MAX (*minimum, child_min);
    *natural = MAX (*natural, child_nat);
  }
}

static void
brk_tab_view_size_allocate (GtkWidget *widget,
                            int        width,
                            int        height,
                            int        baseline)
{
  BrkTabView *self = BRK_TAB_VIEW (widget);
  int i;

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

    if (gtk_widget_get_child_visible (page->bin))
      gtk_widget_allocate (page->bin, width, height, baseline, NULL);
  }
}

static void
unmap_extra_pages (BrkTabView *self)
{
  int i;

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

    if (page == self->selected_page)
      continue;

    if (!gtk_widget_get_child_visible (page->bin))
      continue;

    if (page_should_be_visible (self, page))
      continue;

    gtk_widget_set_child_visible (page->bin, FALSE);
  }

  self->unmap_extra_pages_cb = 0;
}

static void
brk_tab_view_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
  BrkTabView *self = BRK_TAB_VIEW (widget);
  GtkSnapshot *child_snapshot;
  int i;

  if (self->selected_page)
    gtk_widget_snapshot_child (widget, self->selected_page->bin, snapshot);

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

    if (!gtk_widget_get_child_visible (page->bin))
      continue;

    if (page->paintable) {
      if (page == self->selected_page && page->invalidated)
        gtk_widget_queue_draw (page->bin);

      /* We don't want to actually draw the child, but we do need it
       * to redraw so that it can be displayed by its paintable */
      child_snapshot = gtk_snapshot_new ();

      gtk_widget_snapshot_child (widget, page->bin, child_snapshot);

      g_object_unref (child_snapshot);
    }

    page->invalidated = FALSE;

    if (!self->unmap_extra_pages_cb)
      self->unmap_extra_pages_cb =
        g_idle_add_once ((GSourceOnceFunc) unmap_extra_pages, self);
  }
}

static void
draw_overview_pages_after_map_cb (BrkTabView *self)
{
  int i;

  if (!self->overview_count)
    return;

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

    if (page->live_thumbnail || page->invalidated)
      gtk_widget_set_child_visible (page->bin, TRUE);
    else if (page == self->selected_page)
      gtk_widget_queue_draw (GTK_WIDGET (page->bin));
  }

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
brk_tab_view_map (GtkWidget *widget)
{
  BrkTabView *self = BRK_TAB_VIEW (widget);

  GTK_WIDGET_CLASS (brk_tab_view_parent_class)->map (widget);

  if (self->overview_count)
    g_idle_add_once ((GSourceOnceFunc) draw_overview_pages_after_map_cb, self);
}

static void
brk_tab_view_dispose (GObject *object)
{
  BrkTabView *self = BRK_TAB_VIEW (object);

  if (self->unmap_extra_pages_cb) {
    g_source_remove (self->unmap_extra_pages_cb);
    self->unmap_extra_pages_cb = 0;
  }

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), 0, self->n_pages, 0);

  while (self->n_pages) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, 0);

    detach_page (self, page, TRUE);
  }

  g_clear_object (&self->children);

  G_OBJECT_CLASS (brk_tab_view_parent_class)->dispose (object);
}

static void
brk_tab_view_finalize (GObject *object)
{
  BrkTabView *self = (BrkTabView *) object;

  g_clear_weak_pointer (&self->pages);
  g_clear_object (&self->menu_model);

  tab_view_list = g_slist_remove (tab_view_list, self);

  G_OBJECT_CLASS (brk_tab_view_parent_class)->finalize (object);
}

static void
brk_tab_view_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  BrkTabView *self = BRK_TAB_VIEW (object);

  switch (prop_id) {
  case PROP_N_PAGES:
    g_value_set_int (value, brk_tab_view_get_n_pages (self));
    break;

  case PROP_IS_TRANSFERRING_PAGE:
    g_value_set_boolean (value, brk_tab_view_get_is_transferring_page (self));
    break;

  case PROP_SELECTED_PAGE:
    g_value_set_object (value, brk_tab_view_get_selected_page (self));
    break;

  case PROP_MENU_MODEL:
    g_value_set_object (value, brk_tab_view_get_menu_model (self));
    break;

  case PROP_SHORTCUTS:
    g_value_set_flags (value, brk_tab_view_get_shortcuts (self));
    break;

  case PROP_PAGES:
    g_value_take_object (value, brk_tab_view_get_pages (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_view_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  BrkTabView *self = BRK_TAB_VIEW (object);

  switch (prop_id) {
  case PROP_SELECTED_PAGE:
    brk_tab_view_set_selected_page (self, g_value_get_object (value));
    break;

  case PROP_MENU_MODEL:
    brk_tab_view_set_menu_model (self, g_value_get_object (value));
    break;

  case PROP_SHORTCUTS:
    brk_tab_view_set_shortcuts (self, g_value_get_flags (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_view_class_init (BrkTabViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = brk_tab_view_dispose;
  object_class->finalize = brk_tab_view_finalize;
  object_class->get_property = brk_tab_view_get_property;
  object_class->set_property = brk_tab_view_set_property;

  widget_class->measure = brk_tab_view_measure;
  widget_class->size_allocate = brk_tab_view_size_allocate;
  widget_class->snapshot = brk_tab_view_snapshot;
  widget_class->map = brk_tab_view_map;
  widget_class->get_request_mode = brk_widget_get_request_mode;
  widget_class->compute_expand = brk_widget_compute_expand;

  /**
   * BrkTabView:n-pages: (attributes org.gtk.Property.get=brk_tab_view_get_n_pages)
   *
   * The number of pages in the tab view.
   */
  props[PROP_N_PAGES] =
    g_param_spec_int ("n-pages", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * BrkTabView:is-transferring-page: (attributes org.gtk.Property.get=brk_tab_view_get_is_transferring_page)
   *
   * Whether a page is being transferred.
   *
   * This property will be set to `TRUE` when a drag-n-drop tab transfer starts
   * on any `BrkTabView`, and to `FALSE` after it ends.
   *
   * During the transfer, children cannot receive pointer input and a tab can
   * be safely dropped on the tab view.
   */
  props[PROP_IS_TRANSFERRING_PAGE] =
    g_param_spec_boolean ("is-transferring-page", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * BrkTabView:selected-page: (attributes org.gtk.Property.get=brk_tab_view_get_selected_page org.gtk.Property.set=brk_tab_view_set_selected_page)
   *
   * The currently selected page.
   */
  props[PROP_SELECTED_PAGE] =
    g_param_spec_object ("selected-page", NULL, NULL,
                         BRK_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabView:menu-model: (attributes org.gtk.Property.get=brk_tab_view_get_menu_model org.gtk.Property.set=brk_tab_view_set_menu_model)
   *
   * Tab context menu model.
   *
   * When a context menu is shown for a tab, it will be constructed from the
   * provided menu model. Use the [signal@TabView::setup-menu] signal to set up
   * the menu actions for the particular tab.
   */
  props[PROP_MENU_MODEL] =
    g_param_spec_object ("menu-model", NULL, NULL,
                         G_TYPE_MENU_MODEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabView:shortcuts: (attributes org.gtk.Property.get=brk_tab_view_get_shortcuts org.gtk.Property.set=brk_tab_view_set_shortcuts)
   *
   * The enabled shortcuts.
   *
   * See [flags@TabViewShortcuts] for the list of the available shortcuts. All
   * of the shortcuts are enabled by default.
   *
   * [method@TabView.add_shortcuts] and [method@TabView.remove_shortcuts]
   * provide a convenient way to manage individual shortcuts.
   *
   * Since: 1.2
   */
  props[PROP_SHORTCUTS] =
    g_param_spec_flags ("shortcuts", NULL, NULL,
                        BRK_TYPE_TAB_VIEW_SHORTCUTS,
                        BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabView:pages: (attributes org.gtk.Property.get=brk_tab_view_get_pages)
   *
   * A selection model with the tab view's pages.
   *
   * This can be used to keep an up-to-date view. The model also implements
   * [iface@Gtk.SelectionModel] and can be used to track and change the selected
   * page.
   */
  props[PROP_PAGES] =
    g_param_spec_object ("pages", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * BrkTabView::page-attached:
   * @self: a tab view
   * @page: a page of @self
   * @position: the position of the page, starting from 0
   *
   * Emitted when a page has been created or transferred to @self.
   *
   * A typical reason to connect to this signal would be to connect to page
   * signals for things such as updating window title.
   */
  signals[SIGNAL_PAGE_ATTACHED] =
    g_signal_new ("page-attached",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE,
                  2,
                  BRK_TYPE_TAB_PAGE, G_TYPE_INT);
  g_signal_set_va_marshaller (signals[SIGNAL_PAGE_ATTACHED],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__OBJECT_INTv);

  /**
   * BrkTabView::page-detached:
   * @self: a tab view
   * @page: a page of @self
   * @position: the position of the removed page, starting from 0
   *
   * Emitted when a page has been removed or transferred to another view.
   *
   * A typical reason to connect to this signal would be to disconnect signal
   * handlers connected in the [signal@TabView::page-attached] handler.
   *
   * It is important not to try and destroy the page child in the handler of
   * this function as the child might merely be moved to another window; use
   * child dispose handler for that or do it in sync with your
   * [method@TabView.close_page_finish] calls.
   */
  signals[SIGNAL_PAGE_DETACHED] =
    g_signal_new ("page-detached",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE,
                  2,
                  BRK_TYPE_TAB_PAGE, G_TYPE_INT);
  g_signal_set_va_marshaller (signals[SIGNAL_PAGE_DETACHED],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__OBJECT_INTv);

  /**
   * BrkTabView::page-reordered:
   * @self: a tab view
   * @page: a page of @self
   * @position: the position @page was moved to, starting at 0
   *
   * Emitted after @page has been reordered to @position.
   */
  signals[SIGNAL_PAGE_REORDERED] =
    g_signal_new ("page-reordered",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__OBJECT_INT,
                  G_TYPE_NONE,
                  2,
                  BRK_TYPE_TAB_PAGE, G_TYPE_INT);
  g_signal_set_va_marshaller (signals[SIGNAL_PAGE_REORDERED],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__OBJECT_INTv);

  /**
   * BrkTabView::close-page:
   * @self: a tab view
   * @page: a page of @self
   *
   * Emitted after [method@TabView.close_page] has been called for @page.
   *
   * The handler is expected to call [method@TabView.close_page_finish] to
   * confirm or reject the closing.
   *
   * The default handler will immediately confirm closing, equivalent to the
   * following example:
   *
   * ```c
   * static gboolean
   * close_page_cb (BrkTabView *view,
   *                BrkTabPage *page,
   *                gpointer    user_data)
   * {
   *   brk_tab_view_close_page_finish (view, page, TRUE);
   *
   *   return GDK_EVENT_STOP;
   * }
   * ```
   *
   * The [method@TabView.close_page_finish] call doesn't have to happen inside
   * the handler, so can be used to do asynchronous checks before confirming the
   * closing.
   *
   * A typical reason to connect to this signal is to show a confirmation dialog
   * for closing a tab.
   *
   * The signal handler should return `GDK_EVENT_STOP` to stop propagation or
   * `GDK_EVENT_CONTINUE` to invoke the default handler.
   *
   * Returns: whether propagation should be stopped
   */
  signals[SIGNAL_CLOSE_PAGE] =
    g_signal_new ("close-page",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_true_handled,
                  NULL,
                  brk_marshal_BOOLEAN__OBJECT,
                  G_TYPE_BOOLEAN,
                  1,
                  BRK_TYPE_TAB_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSE_PAGE],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_BOOLEAN__OBJECTv);

  /**
   * BrkTabView::setup-menu:
   * @self: a tab view
   * @page: (nullable): a page of @self
   *
   * Emitted when a context menu is opened or closed for @page.
   *
   * If the menu has been closed, @page will be set to `NULL`.
   *
   * It can be used to set up menu actions before showing the menu, for example
   * disable actions not applicable to @page.
   */
  signals[SIGNAL_SETUP_MENU] =
    g_signal_new ("setup-menu",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  BRK_TYPE_TAB_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_SETUP_MENU],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__OBJECTv);

  /**
   * BrkTabView::create-window:
   * @self: a tab view
   *
   * Emitted when a tab should be transferred into a new window.
   *
   * This can happen after a tab has been dropped on desktop.
   *
   * The signal handler is expected to create a new window, position it as
   * needed and return its `BrkTabView` that the page will be transferred into.
   *
   * Returns: (transfer none) (nullable): the `BrkTabView` from the new window
   */
  signals[SIGNAL_CREATE_WINDOW] =
    g_signal_new ("create-window",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  object_handled_accumulator,
                  NULL,
                  brk_marshal_OBJECT__VOID,
                  BRK_TYPE_TAB_VIEW,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CREATE_WINDOW],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_OBJECT__VOIDv);

  /**
   * BrkTabView::indicator-activated:
   * @self: a tab view
   * @page: a page of @self
   *
   * Emitted after the indicator icon on @page has been activated.
   *
   * See [property@TabPage:indicator-icon] and
   * [property@TabPage:indicator-activatable].
   */
  signals[SIGNAL_INDICATOR_ACTIVATED] =
    g_signal_new ("indicator-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  BRK_TYPE_TAB_PAGE);
  g_signal_set_va_marshaller (signals[SIGNAL_INDICATOR_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__OBJECTv);

  g_signal_override_class_handler ("close-page",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (close_page_cb));

  gtk_widget_class_set_css_name (widget_class, "tabview");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
brk_tab_view_init (BrkTabView *self)
{
  GtkEventController *controller;

  self->children = g_list_store_new (BRK_TYPE_TAB_PAGE);
  self->shortcuts = BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS;

  tab_view_list = g_slist_prepend (tab_view_list, self);

  controller = gtk_shortcut_controller_new ();
  gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
  gtk_shortcut_controller_set_scope (GTK_SHORTCUT_CONTROLLER (controller),
                                     GTK_SHORTCUT_SCOPE_GLOBAL);

  init_shortcuts (self, controller);

  gtk_widget_add_controller (GTK_WIDGET (self), controller);
}

static void
brk_tab_view_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  BrkTabView *self = BRK_TAB_VIEW (buildable);

  if (!type && GTK_IS_WIDGET (child))
    brk_tab_view_append (self, GTK_WIDGET (child));
  else if (!type && BRK_IS_TAB_PAGE (child))
    insert_page (self, BRK_TAB_PAGE (child), self->n_pages);
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
brk_tab_view_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = brk_tab_view_buildable_add_child;
}

static GtkAccessible *
brk_tab_view_accessible_get_first_accessible_child (GtkAccessible *accessible)
{
  BrkTabView *self = BRK_TAB_VIEW (accessible);

  if (brk_tab_view_get_n_pages (self) > 0)
    return GTK_ACCESSIBLE (g_object_ref (brk_tab_view_get_nth_page (self, 0)));

  return NULL;
}

static void
brk_tab_view_accessible_init (GtkAccessibleInterface *iface)
{
  iface->get_first_accessible_child = brk_tab_view_accessible_get_first_accessible_child;
}

/**
 * brk_tab_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a tab page
 *
 * Gets the child of @self.
 *
 * Returns: (transfer none): the child of @self
 */
GtkWidget *
brk_tab_page_get_child (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->child;
}

/**
 * brk_tab_page_get_parent: (attributes org.gtk.Method.get_property=parent)
 * @self: a tab page
 *
 * Gets the parent page of @self.
 *
 * See [method@TabView.add_page] and [method@TabView.close_page].
 *
 * Returns: (transfer none) (nullable): the parent page
 */
BrkTabPage *
brk_tab_page_get_parent (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->parent;
}

/**
 * brk_tab_page_get_selected: (attributes org.gtk.Method.get_property=selected)
 * @self: a tab page
 *
 * Gets whether @self is selected.
 *
 * Returns: whether @self is selected
 */
gboolean
brk_tab_page_get_selected (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), FALSE);

  return self->selected;
}

/**
 * brk_tab_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a tab page
 *
 * Gets the title of @self.
 *
 * Returns: the title of @self
 */
const char *
brk_tab_page_get_title (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->title;
}

/**
 * brk_tab_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a tab page
 * @title: the title of @self
 *
 * [class@TabBar] will display it in the center of the tab and will use it as a
 * tooltip unless [property@TabPage:tooltip] is set.
 *
 * Sets the title of @self.
 */
void
brk_tab_page_set_title (BrkTabPage *self,
                        const char *title)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  if (!g_set_str (&self->title, title ? title : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TITLE]);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_LABEL, self->title,
                                  -1);
}

/**
 * brk_tab_page_get_tooltip: (attributes org.gtk.Method.get_property=tooltip)
 * @self: a tab page
 *
 * Gets the tooltip of @self.
 *
 * Returns: (nullable): the tooltip of @self
 */
const char *
brk_tab_page_get_tooltip (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->tooltip;
}

/**
 * brk_tab_page_set_tooltip: (attributes org.gtk.Method.set_property=tooltip)
 * @self: a tab page
 * @tooltip: the tooltip of @self
 *
 * Sets the tooltip of @self.
 *
 * The tooltip can be marked up with the Pango text markup language.
 *
 * If not set, [class@TabBar] and [class@TabOverview] will use
 * [property@TabPage:title] as a tooltip instead.
 */
void
brk_tab_page_set_tooltip (BrkTabPage *self,
                          const char *tooltip)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  if (!g_set_str (&self->tooltip, tooltip ? tooltip : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_TOOLTIP]);
}

/**
 * brk_tab_page_get_icon: (attributes org.gtk.Method.get_property=icon)
 * @self: a tab page
 *
 * Gets the icon of @self.
 *
 * Returns: (transfer none) (nullable): the icon of @self
 */
GIcon *
brk_tab_page_get_icon (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->icon;
}

/**
 * brk_tab_page_set_icon: (attributes org.gtk.Method.set_property=icon)
 * @self: a tab page
 * @icon: (nullable): the icon of @self
 *
 * Sets the icon of @self.
 *
 * [class@TabBar] and [class@TabOverview] display the icon next to the title,
 * unless [property@TabPage:loading] is set to `TRUE`.
 */
void
brk_tab_page_set_icon (BrkTabPage *self,
                       GIcon      *icon)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));
  g_return_if_fail (icon == NULL || G_IS_ICON (icon));

  if (!g_set_object (&self->icon, icon))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_ICON]);
}

/**
 * brk_tab_page_get_loading: (attributes org.gtk.Method.get_property=loading)
 * @self: a tab page
 *
 * Gets whether @self is loading.
 *
 * Returns: whether @self is loading
 */
gboolean
brk_tab_page_get_loading (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), FALSE);

  return self->loading;
}

/**
 * brk_tab_page_set_loading: (attributes org.gtk.Method.set_property=loading)
 * @self: a tab page
 * @loading: whether @self is loading
 *
 * Sets whether @self is loading.
 *
 * If set to `TRUE`, [class@TabBar] and [class@TabOverview] will display a
 * spinner in place of icon.
 */
void
brk_tab_page_set_loading (BrkTabPage *self,
                          gboolean    loading)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  loading = !!loading;

  if (self->loading == loading)
    return;

  self->loading = loading;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_LOADING]);
}

/**
 * brk_tab_page_get_indicator_icon: (attributes org.gtk.Method.get_property=indicator-icon)
 * @self: a tab page
 *
 * Gets the indicator icon of @self.
 *
 * Returns: (transfer none) (nullable): the indicator icon of @self
 */
GIcon *
brk_tab_page_get_indicator_icon (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->indicator_icon;
}

/**
 * brk_tab_page_set_indicator_icon: (attributes org.gtk.Method.set_property=indicator-icon)
 * @self: a tab page
 * @indicator_icon: (nullable): the indicator icon of @self
 *
 * Sets the indicator icon of @self.
 *
 * A common use case is an audio or camera indicator in a web browser.
 *
 * [class@TabBar] will show it at the beginning of the tab, alongside icon
 * representing [property@TabPage:icon] or loading spinner.
 *
 * [class@TabOverview] will show it at the at the top part of the thumbnail.
 *
 * [property@TabPage:indicator-tooltip] can be used to set the tooltip on the
 * indicator icon.
 *
 * If [property@TabPage:indicator-activatable] is set to `TRUE`, the
 * indicator icon can act as a button.
 */
void
brk_tab_page_set_indicator_icon (BrkTabPage *self,
                                 GIcon      *indicator_icon)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));
  g_return_if_fail (indicator_icon == NULL || G_IS_ICON (indicator_icon));

  if (!g_set_object (&self->indicator_icon, indicator_icon))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_INDICATOR_ICON]);
}

/**
 * brk_tab_page_get_indicator_tooltip: (attributes org.gtk.Method.get_property=indicator-tooltip)
 * @self: a tab page
 *
 * Gets the tooltip of the indicator icon of @self.
 *
 * Returns: (transfer none): the indicator tooltip of @self
 *
 * Since: 1.2
 */
const char *
brk_tab_page_get_indicator_tooltip (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->indicator_tooltip;
}

/**
 * brk_tab_page_set_indicator_tooltip: (attributes org.gtk.Method.set_property=indicator-tooltip)
 * @self: a tab page
 * @tooltip: the indicator tooltip of @self
 *
 * Sets the tooltip of the indicator icon of @self.
 *
 * The tooltip can be marked up with the Pango text markup language.
 *
 * See [property@TabPage:indicator-icon].
 *
 * Since: 1.2
 */
void
brk_tab_page_set_indicator_tooltip (BrkTabPage *self,
                                    const char *tooltip)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));
  g_return_if_fail (tooltip != NULL);

  if (!g_set_str (&self->indicator_tooltip, tooltip ? tooltip : ""))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_INDICATOR_TOOLTIP]);
}

/**
 * brk_tab_page_get_indicator_activatable: (attributes org.gtk.Method.get_property=indicator-activatable)
 * @self: a tab page
 *
 *
 * Gets whether the indicator of @self is activatable.
 *
 * Returns: whether the indicator is activatable
 */
gboolean
brk_tab_page_get_indicator_activatable (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), FALSE);

  return self->indicator_activatable;
}

/**
 * brk_tab_page_set_indicator_activatable: (attributes org.gtk.Method.set_property=indicator-activatable)
 * @self: a tab page
 * @activatable: whether the indicator is activatable
 *
 * Sets whether the indicator of @self is activatable.
 *
 * If set to `TRUE`, [signal@TabView::indicator-activated] will be emitted
 * when the indicator icon is clicked.
 *
 * If [property@TabPage:indicator-icon] is not set, does nothing.
 */
void
brk_tab_page_set_indicator_activatable (BrkTabPage *self,
                                        gboolean    activatable)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  activatable = !!activatable;

  if (self->indicator_activatable == activatable)
    return;

  self->indicator_activatable = activatable;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_INDICATOR_ACTIVATABLE]);
}

/**
 * brk_tab_page_get_needs_attention: (attributes org.gtk.Method.get_property=needs-attention)
 * @self: a tab page
 *
 * Gets whether @self needs attention.
 *
 * Returns: whether @self needs attention
 */
gboolean
brk_tab_page_get_needs_attention (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), FALSE);

  return self->needs_attention;
}

/**
 * brk_tab_page_set_needs_attention: (attributes org.gtk.Method.set_property=needs-attention)
 * @self: a tab page
 * @needs_attention: whether @self needs attention
 *
 * Sets whether @self needs attention.
 *
 * [class@TabBar] will display a line under the tab representing the page if
 * set to `TRUE`. If the tab is not visible, the corresponding edge of the tab
 * bar will be highlighted.
 *
 * [class@TabOverview] will display a dot in the corner of the thumbnail if set
 * to `TRUE`.
 *
 * [class@TabButton] will display a dot if any of the pages that aren't
 * selected have [property@TabPage:needs-attention] set to `TRUE`.
 */
void
brk_tab_page_set_needs_attention (BrkTabPage *self,
                                  gboolean    needs_attention)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  needs_attention = !!needs_attention;

  if (self->needs_attention == needs_attention)
    return;

  self->needs_attention = needs_attention;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NEEDS_ATTENTION]);
}

/**
 * brk_tab_page_get_keyword: (attributes org.gtk.Method.get_property=keyword)
 * @self: a tab page
 *
 * Gets the search keyword of @self.
 *
 * Returns: (nullable): the search keyword of @self
 *
 * Since: 1.3
 */
const char *
brk_tab_page_get_keyword (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  return self->keyword;
}

/**
 * brk_tab_page_set_keyword: (attributes org.gtk.Method.set_property=keyword)
 * @self: a tab page
 * @keyword: the search keyword
 *
 * Sets the search keyword for @self.
 *
 * [class@TabOverview] can search pages by their keywords in addition to their
 * titles and tooltips.
 *
 * Keywords allow to include e.g. page URLs into tab search in a web browser.
 *
 * Since: 1.3
 */
void
brk_tab_page_set_keyword (BrkTabPage *self,
                          const char *keyword)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  if (!g_set_str (&self->keyword, keyword))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_KEYWORD]);
}

/**
 * brk_tab_page_get_thumbnail_xalign: (attributes org.gtk.Method.get_property=thumbnail-xalign)
 * @self: a tab page
 *
 * Gets the horizontal alignment of the thumbnail for @self.
 *
 * Returns: the horizontal alignment
 *
 * Since: 1.3
 */
float
brk_tab_page_get_thumbnail_xalign (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), 0.0f);

  return self->thumbnail_xalign;
}

/**
 * brk_tab_page_set_thumbnail_xalign: (attributes org.gtk.Method.set_property=thumbnail-xalign)
 * @self: a tab page
 * @xalign: the new value
 *
 * Sets the horizontal alignment of the thumbnail for @self.
 *
 * If the page is so wide that [class@TabOverview] can't display it completely
 * and has to crop it, horizontal alignment will determine which part of the
 * page will be visible.
 *
 * For example, 0.5 means the center of the page will be visible, 0 means the
 * start edge will be visible and 1 means the end edge will be visible.
 *
 * The default horizontal alignment is 0.
 *
 * Since: 1.3
 */
void
brk_tab_page_set_thumbnail_xalign (BrkTabPage *self,
                                   float       xalign)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  xalign = CLAMP (xalign, 0.0, 1.0);

  if (G_APPROX_VALUE (self->thumbnail_xalign, xalign, FLT_EPSILON))
    return;

  self->thumbnail_xalign = xalign;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_THUMBNAIL_XALIGN]);
}

/**
 * brk_tab_page_get_thumbnail_yalign: (attributes org.gtk.Method.get_property=thumbnail-yalign)
 * @self: a tab overview
 *
 * Gets the vertical alignment of the thumbnail for @self.
 *
 * Returns: the vertical alignment
 *
 * Since: 1.3
 */
float
brk_tab_page_get_thumbnail_yalign (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), 0.0f);

  return self->thumbnail_yalign;
}

/**
 * brk_tab_page_set_thumbnail_yalign: (attributes org.gtk.Method.set_property=thumbnail-yalign)
 * @self: a tab page
 * @yalign: the new value
 *
 * Sets the vertical alignment of the thumbnail for @self.
 *
 * If the page is so tall that [class@TabOverview] can't display it completely
 * and has to crop it, vertical alignment will determine which part of the page
 * will be visible.
 *
 * For example, 0.5 means the center of the page will be visible, 0 means the
 * top edge will be visible and 1 means the bottom edge will be visible.
 *
 * The default vertical alignment is 0.
 *
 * Since: 1.3
 */
void
brk_tab_page_set_thumbnail_yalign (BrkTabPage *self,
                                   float       yalign)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  yalign = CLAMP (yalign, 0.0, 1.0);

  if (G_APPROX_VALUE (self->thumbnail_yalign, yalign, FLT_EPSILON))
    return;

  self->thumbnail_yalign = yalign;

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_THUMBNAIL_YALIGN]);
}

/**
 * brk_tab_page_get_live_thumbnail: (attributes org.gtk.Method.get_property=live-thumbnail)
 * @self: a tab overview
 *
 * Gets whether to live thumbnail is enabled @self.
 *
 * Returns: whether live thumbnail is enabled
 *
 * Since: 1.3
 */
gboolean
brk_tab_page_get_live_thumbnail (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), FALSE);

  return self->live_thumbnail;
}

/**
 * brk_tab_page_set_live_thumbnail: (attributes org.gtk.Method.set_property=live-thumbnail)
 * @self: a tab page
 * @live_thumbnail: whether to enable live thumbnail
 *
 * Sets whether to enable live thumbnail for @self.
 *
 * When set to `TRUE`, @self's thumbnail in [class@TabOverview] will update
 * immediately when @self is redrawn or resized.
 *
 * If it's set to `FALSE`, the thumbnail will only be live when the @self is
 * selected, and otherwise it will be static and will only update when
 * [method@TabPage.invalidate_thumbnail] or
 * [method@TabView.invalidate_thumbnails] is called.
 *
 * Since: 1.3
 */
void
brk_tab_page_set_live_thumbnail (BrkTabPage *self,
                                 gboolean    live_thumbnail)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  live_thumbnail = !!live_thumbnail;

  if (self->live_thumbnail == live_thumbnail)
    return;

  self->live_thumbnail = live_thumbnail;

  map_or_unmap_page (self);

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_LIVE_THUMBNAIL]);
}


/**
 * brk_tab_page_invalidate_thumbnail:
 * @self: a tab page
 *
 * Invalidates thumbnail for @self.
 *
 * If an [class@TabOverview] is open, the thumbnail representing @self will be
 * immediately updated. Otherwise it will be update when opening the overview.
 *
 * Does nothing if [property@TabPage:live-thumbnail] is set to `TRUE`.
 *
 * See also [method@TabView.invalidate_thumbnails].
 *
 * Since: 1.3
 */
void
brk_tab_page_invalidate_thumbnail (BrkTabPage *self)
{
  g_return_if_fail (BRK_IS_TAB_PAGE (self));

  if (self->invalidated)
    return;

  self->invalidated = TRUE;

  map_or_unmap_page (self);
}

GdkPaintable *
brk_tab_page_get_paintable (BrkTabPage *self)
{
  g_return_val_if_fail (BRK_IS_TAB_PAGE (self), NULL);

  if (!self->paintable)
    self->paintable = brk_tab_paintable_new (self);

  return self->paintable;
}

/**
 * brk_tab_view_new:
 *
 * Creates a new `BrkTabView`.
 *
 * Returns: the newly created `BrkTabView`
 */
BrkTabView *
brk_tab_view_new (void)
{
  return g_object_new (BRK_TYPE_TAB_VIEW, NULL);
}

/**
 * brk_tab_view_get_n_pages: (attributes org.gtk.Method.get_property=n-pages)
 * @self: a tab view
 *
 * Gets the number of pages in @self.
 *
 * Returns: the number of pages in @self
 */
int
brk_tab_view_get_n_pages (BrkTabView *self)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), 0);

  return self->n_pages;
}

/**
 * brk_tab_view_get_is_transferring_page: (attributes org.gtk.Method.get_property=is-transferring-page)
 * @self: a tab view
 *
 * Whether a page is being transferred.
 *
 * The corresponding property will be set to `TRUE` when a drag-n-drop tab
 * transfer starts on any `BrkTabView`, and to `FALSE` after it ends.
 *
 * During the transfer, children cannot receive pointer input and a tab can
 * be safely dropped on the tab view.
 *
 * Returns: whether a page is being transferred
 */
gboolean
brk_tab_view_get_is_transferring_page (BrkTabView *self)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);

  return self->transfer_count > 0;
}

/**
 * brk_tab_view_get_selected_page: (attributes org.gtk.Method.get_property=selected-page)
 * @self: a tab view
 *
 * Gets the currently selected page in @self.
 *
 * Returns: (transfer none) (nullable): the selected page
 */
BrkTabPage *
brk_tab_view_get_selected_page (BrkTabView *self)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);

  return self->selected_page;
}

/**
 * brk_tab_view_set_selected_page: (attributes org.gtk.Method.set_property=selected-page)
 * @self: a tab view
 * @selected_page: a page in @self
 *
 * Sets the currently selected page in @self.
 */
void
brk_tab_view_set_selected_page (BrkTabView *self,
                                BrkTabPage *selected_page)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));

  if (self->n_pages > 0) {
    g_return_if_fail (BRK_IS_TAB_PAGE (selected_page));
    g_return_if_fail (page_belongs_to_this_view (self, selected_page));
  } else {
    g_return_if_fail (selected_page == NULL);
  }

  set_selected_page (self, selected_page, TRUE);
}

/**
 * brk_tab_view_select_previous_page:
 * @self: a tab view
 *
 * Selects the page before the currently selected page.
 *
 * If the first page was already selected, this function does nothing.
 *
 * Returns: whether the selected page was changed
 */
gboolean
brk_tab_view_select_previous_page (BrkTabView *self)
{
  BrkTabPage *page;
  int pos;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  pos = brk_tab_view_get_page_position (self, self->selected_page);

  if (pos <= 0)
    return FALSE;

  page = brk_tab_view_get_nth_page (self, pos - 1);

  brk_tab_view_set_selected_page (self, page);

  return TRUE;
}

/**
 * brk_tab_view_select_next_page:
 * @self: a tab view
 *
 * Selects the page after the currently selected page.
 *
 * If the last page was already selected, this function does nothing.
 *
 * Returns: whether the selected page was changed
 */
gboolean
brk_tab_view_select_next_page (BrkTabView *self)
{
  BrkTabPage *page;
  int pos;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  pos = brk_tab_view_get_page_position (self, self->selected_page);

  if (pos >= self->n_pages - 1)
    return FALSE;

  page = brk_tab_view_get_nth_page (self, pos + 1);

  brk_tab_view_set_selected_page (self, page);

  return TRUE;
}

gboolean
brk_tab_view_select_first_page (BrkTabView *self)
{
  BrkTabPage *page;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  page = brk_tab_view_get_nth_page (self, 0);

  if (page == self->selected_page)
    return FALSE;

  brk_tab_view_set_selected_page (self, page);

  return TRUE;
}

gboolean
brk_tab_view_select_last_page (BrkTabView *self)
{
  BrkTabPage *page;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);

  if (!self->selected_page)
    return FALSE;

  page = brk_tab_view_get_nth_page (self, 0);

  if (page == self->selected_page)
    return FALSE;

  brk_tab_view_set_selected_page (self, page);

  return TRUE;
}

/**
 * brk_tab_view_get_menu_model: (attributes org.gtk.Method.get_property=menu-model)
 * @self: a tab view
 *
 * Gets the tab context menu model for @self.
 *
 * Returns: (transfer none) (nullable): the tab context menu model for @self
 */
GMenuModel *
brk_tab_view_get_menu_model (BrkTabView *self)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);

  return self->menu_model;
}

/**
 * brk_tab_view_set_menu_model: (attributes org.gtk.Method.set_property=menu-model)
 * @self: a tab view
 * @menu_model: (nullable): a menu model
 *
 * Sets the tab context menu model for @self.
 *
 * When a context menu is shown for a tab, it will be constructed from the
 * provided menu model. Use the [signal@TabView::setup-menu] signal to set up
 * the menu actions for the particular tab.
 */
void
brk_tab_view_set_menu_model (BrkTabView *self,
                             GMenuModel *menu_model)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (menu_model == NULL || G_IS_MENU_MODEL (menu_model));

  if (self->menu_model == menu_model)
    return;

  g_set_object (&self->menu_model, menu_model);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MENU_MODEL]);
}

/**
 * brk_tab_view_get_shortcuts: (attributes org.gtk.Method.get_property=shortcuts)
 * @self: a tab view
 *
 * Gets the enabled shortcuts for @self.
 *
 * Returns: the shortcut mask
 *
 * Since: 1.2
 */
BrkTabViewShortcuts
brk_tab_view_get_shortcuts (BrkTabView *self)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), 0);

  return self->shortcuts;
}

/**
 * brk_tab_view_set_shortcuts: (attributes org.gtk.Method.set_property=shortcuts)
 * @self: a tab view
 * @shortcuts: the new shortcuts
 *
 * Sets the enabled shortcuts for @self.
 *
 * See [flags@TabViewShortcuts] for the list of the available shortcuts. All of
 * the shortcuts are enabled by default.
 *
 * [method@TabView.add_shortcuts] and [method@TabView.remove_shortcuts] provide
 * a convenient way to manage individual shortcuts.
 *
 * Since: 1.2
 */
void
brk_tab_view_set_shortcuts (BrkTabView          *self,
                            BrkTabViewShortcuts  shortcuts)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (shortcuts <= BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);

  if (self->shortcuts == shortcuts)
    return;

  self->shortcuts = shortcuts;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHORTCUTS]);
}

/**
 * brk_tab_view_add_shortcuts:
 * @self: a tab view
 * @shortcuts: the shortcuts to add
 *
 * Adds @shortcuts for @self.
 *
 * See [property@TabView:shortcuts] for details.
 *
 * Since: 1.2
 */
void
brk_tab_view_add_shortcuts (BrkTabView          *self,
                            BrkTabViewShortcuts  shortcuts)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (shortcuts <= BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);

  brk_tab_view_set_shortcuts (self, self->shortcuts | shortcuts);
}

/**
 * brk_tab_view_remove_shortcuts:
 * @self: a tab view
 * @shortcuts: the shortcuts to reomve
 *
 * Removes @shortcuts from @self.
 *
 * See [property@TabView:shortcuts] for details.
 *
 * Since: 1.2
 */
void
brk_tab_view_remove_shortcuts (BrkTabView          *self,
                               BrkTabViewShortcuts  shortcuts)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (shortcuts <= BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS);

  brk_tab_view_set_shortcuts (self, self->shortcuts & ~shortcuts);
}

/**
 * brk_tab_view_get_page:
 * @self: a tab view
 * @child: a child in @self
 *
 * Gets the [class@TabPage] object representing @child.
 *
 * Returns: (transfer none): the page object for @child
 */
BrkTabPage *
brk_tab_view_get_page (BrkTabView *self,
                       GtkWidget  *child)
{
  int i;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (child_belongs_to_this_view (self, child), NULL);

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

    if (brk_tab_page_get_child (page) == child)
      return page;
  }

  g_assert_not_reached ();
}

/**
 * brk_tab_view_get_nth_page:
 * @self: a tab view
 * @position: the index of the page in @self, starting from 0
 *
 * Gets the [class@TabPage] representing the child at @position.
 *
 * Returns: (transfer none): the page object at @position
 */
BrkTabPage *
brk_tab_view_get_nth_page (BrkTabView *self,
                           int         position)
{
  BrkTabPage *page;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (position >= 0, NULL);
  g_return_val_if_fail (position < self->n_pages, NULL);

  page = g_list_model_get_item (G_LIST_MODEL (self->children), (guint) position);

  g_object_unref (page);

  return page;
}

/**
 * brk_tab_view_get_page_position:
 * @self: a tab view
 * @page: a page of @self
 *
 * Finds the position of @page in @self, starting from 0.
 *
 * Returns: the position of @page in @self
 */
int
brk_tab_view_get_page_position (BrkTabView *self,
                                BrkTabPage *page)
{
  int i;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), -1);
  g_return_val_if_fail (BRK_IS_TAB_PAGE (page), -1);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), -1);

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *p = brk_tab_view_get_nth_page (self, i);

    if (page == p)
      return i;
  }

  g_assert_not_reached ();
}

/**
 * brk_tab_view_add_page:
 * @self: a tab view
 * @child: a widget to add
 * @parent: (nullable): a parent page for @child
 *
 * Adds @child to @self with @parent as the parent.
 *
 * This function can be used to automatically position new pages, and to select
 * the correct page when this page is closed while being selected (see
 * [method@TabView.close_page]).
 *
 * If @parent is `NULL`, this function is equivalent to [method@TabView.append].
 *
 * Returns: (transfer none): the page object representing @child
 */
BrkTabPage *
brk_tab_view_add_page (BrkTabView *self,
                       GtkWidget  *child,
                       BrkTabPage *parent)
{
  int position;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (parent == NULL || BRK_IS_TAB_PAGE (parent), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  if (parent) {
    BrkTabPage *page;

    g_return_val_if_fail (page_belongs_to_this_view (self, parent), NULL);

    position = brk_tab_view_get_page_position (self, parent);

    do {
      position++;

      if (position >= self->n_pages)
        break;

      page = brk_tab_view_get_nth_page (self, position);
    } while (is_descendant_of (page, parent));
  } else {
    position = self->n_pages;
  }

  return create_and_insert_page (self, child, parent, position);
}

/**
 * brk_tab_view_insert:
 * @self: a tab view
 * @child: a widget to add
 * @position: the position to add @child at, starting from 0
 *
 * Inserts a page at @position.
 *
 * Returns: (transfer none): the page object representing @child
 */
BrkTabPage *
brk_tab_view_insert (BrkTabView *self,
                     GtkWidget  *child,
                     int         position)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);
  g_return_val_if_fail (position >= 0, NULL);
  g_return_val_if_fail (position <= self->n_pages, NULL);

  return create_and_insert_page (self, child, NULL, position);
}

/**
 * brk_tab_view_prepend:
 * @self: a tab view
 * @child: a widget to add
 *
 * Inserts @child as the first page.
 *
 * Returns: (transfer none): the page object representing @child
 */
BrkTabPage *
brk_tab_view_prepend (BrkTabView *self,
                      GtkWidget  *child)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return create_and_insert_page (self, child, NULL, 0);
}

/**
 * brk_tab_view_append:
 * @self: a tab view
 * @child: a widget to add
 *
 * Inserts @child as the last page.
 *
 * Returns: (transfer none): the page object representing @child
 */
BrkTabPage *
brk_tab_view_append (BrkTabView *self,
                     GtkWidget  *child)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return create_and_insert_page (self, child, NULL, self->n_pages);
}

/**
 * brk_tab_view_close_page:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close @page.
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
void
brk_tab_view_close_page (BrkTabView *self,
                         BrkTabPage *page)
{
  gboolean ret;

  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  if (page->closing)
    return;

  page->closing = TRUE;
  g_signal_emit (self, signals[SIGNAL_CLOSE_PAGE], 0, page, &ret);
}

/**
 * brk_tab_view_close_page_finish:
 * @self: a tab view
 * @page: a page of @self
 * @confirm: whether to confirm or deny closing @page
 *
 * Completes a [method@TabView.close_page] call for @page.
 *
 * If @confirm is `TRUE`, @page will be closed. If it's `FALSE`, it will be
 * reverted to its previous state and [method@TabView.close_page] can be called
 * for it again.
 *
 * This function should not be called unless a custom handler for
 * [signal@TabView::close-page] is used.
 */
void
brk_tab_view_close_page_finish (BrkTabView *self,
                                BrkTabPage *page,
                                gboolean    confirm)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));
  g_return_if_fail (page->closing);

  page->closing = FALSE;

  if (!confirm)
    return;

  if (page->paintable)
    brk_tab_paintable_freeze (BRK_TAB_PAINTABLE (page->paintable));

  detach_page (self, page, FALSE);
}

/**
 * brk_tab_view_close_other_pages:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close all pages other than @page.
 */
void
brk_tab_view_close_other_pages (BrkTabView *self,
                                BrkTabPage *page)
{
  int i;

  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  for (i = self->n_pages - 1; i >= 0; i--) {
    BrkTabPage *p = brk_tab_view_get_nth_page (self, i);

    if (p == page)
      continue;

    brk_tab_view_close_page (self, p);
  }
}

/**
 * brk_tab_view_close_pages_before:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close all pages before @page.
 */
void
brk_tab_view_close_pages_before (BrkTabView *self,
                                 BrkTabPage *page)
{
  int pos, i;

  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  pos = brk_tab_view_get_page_position (self, page);

  for (i = pos - 1; i >= 0; i--) {
    BrkTabPage *p = brk_tab_view_get_nth_page (self, i);

    brk_tab_view_close_page (self, p);
  }
}

/**
 * brk_tab_view_close_pages_after:
 * @self: a tab view
 * @page: a page of @self
 *
 * Requests to close all pages after @page.
 */
void
brk_tab_view_close_pages_after (BrkTabView *self,
                                BrkTabPage *page)
{
  int pos, i;

  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  pos = brk_tab_view_get_page_position (self, page);

  for (i = self->n_pages - 1; i > pos; i--) {
    BrkTabPage *p = brk_tab_view_get_nth_page (self, i);

    brk_tab_view_close_page (self, p);
  }
}

/**
 * brk_tab_view_reorder_page:
 * @self: a tab view
 * @page: a page of @self
 * @position: the position to insert the page at, starting at 0
 *
 * Reorders @page to @position.
 *
 * Returns: whether @page was moved
 */
gboolean
brk_tab_view_reorder_page (BrkTabView *self,
                           BrkTabPage *page,
                           int         position)
{
  int original_pos;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (BRK_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);
  g_return_val_if_fail (position >= 0, FALSE);
  g_return_val_if_fail (position < self->n_pages, FALSE);

  original_pos = brk_tab_view_get_page_position (self, page);

  if (original_pos == position)
    return FALSE;

  g_object_ref (page);

  g_list_store_remove (self->children, original_pos);
  g_list_store_insert (self->children, position, page);

  g_object_unref (page);

  g_signal_emit (self, signals[SIGNAL_PAGE_REORDERED], 0, page, position);

  if (self->pages) {
    int min = MIN (original_pos, position);
    int n_changed = MAX (original_pos, position) - min + 1;

    g_list_model_items_changed (G_LIST_MODEL (self->pages), min, n_changed, n_changed);
  }

  return TRUE;
}

/**
 * brk_tab_view_reorder_backward:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to before its previous page if possible.
 *
 * Returns: whether @page was moved
 */
gboolean
brk_tab_view_reorder_backward (BrkTabView *self,
                               BrkTabPage *page)
{
  int pos;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (BRK_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  pos = brk_tab_view_get_page_position (self, page);

  if (pos <= 0)
    return FALSE;

  return brk_tab_view_reorder_page (self, page, pos - 1);
}

/**
 * brk_tab_view_reorder_forward:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to after its next page if possible.
 *
 * Returns: whether @page was moved
 */
gboolean
brk_tab_view_reorder_forward (BrkTabView *self,
                              BrkTabPage *page)
{
  int pos;

  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (BRK_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  pos = brk_tab_view_get_page_position (self, page);

  if (pos >= self->n_pages - 1)
    return FALSE;

  return brk_tab_view_reorder_page (self, page, pos + 1);
}

/**
 * brk_tab_view_reorder_first:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to the first possible position.
 *
 * Returns: whether @page was moved
 */
gboolean
brk_tab_view_reorder_first (BrkTabView *self,
                            BrkTabPage *page)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (BRK_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  return brk_tab_view_reorder_page (self, page, 0);
}

/**
 * brk_tab_view_reorder_last:
 * @self: a tab view
 * @page: a page of @self
 *
 * Reorders @page to the last possible position.
 *
 * Returns: whether @page was moved
 */
gboolean
brk_tab_view_reorder_last (BrkTabView *self,
                           BrkTabPage *page)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), FALSE);
  g_return_val_if_fail (BRK_IS_TAB_PAGE (page), FALSE);
  g_return_val_if_fail (page_belongs_to_this_view (self, page), FALSE);

  return brk_tab_view_reorder_page (self, page, self->n_pages - 1);
}

void
brk_tab_view_detach_page (BrkTabView *self,
                          BrkTabPage *page)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (page_belongs_to_this_view (self, page));

  g_object_ref (page);

  begin_transfer_for_group (self);

  detach_page (self, page, FALSE);
}

void
brk_tab_view_attach_page (BrkTabView *self,
                          BrkTabPage *page,
                          int         position)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (!page_belongs_to_this_view (self, page));
  g_return_if_fail (position >= 0);
  g_return_if_fail (position <= self->n_pages);

  attach_page (self, page, position);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 0, 1);

  brk_tab_view_set_selected_page (self, page);

  end_transfer_for_group (self);

  g_object_unref (page);
}

/**
 * brk_tab_view_transfer_page:
 * @self: a tab view
 * @page: a page of @self
 * @other_view: the tab view to transfer the page to
 * @position: the position to insert the page at, starting at 0
 *
 * Transfers @page from @self to @other_view.
 *
 * The @page object will be reused.
 */
void
brk_tab_view_transfer_page (BrkTabView *self,
                            BrkTabPage *page,
                            BrkTabView *other_view,
                            int         position)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));
  g_return_if_fail (BRK_IS_TAB_PAGE (page));
  g_return_if_fail (BRK_IS_TAB_VIEW (other_view));
  g_return_if_fail (page_belongs_to_this_view (self, page));
  g_return_if_fail (position >= 0);
  g_return_if_fail (position <= other_view->n_pages);

  brk_tab_view_detach_page (self, page);
  brk_tab_view_attach_page (other_view, page, position);
}

/**
 * brk_tab_view_get_pages: (attributes org.gtk.Method.get_property=pages)
 * @self: a tab view
 *
 * Returns a [iface@Gio.ListModel] that contains the pages of @self.
 *
 * This can be used to keep an up-to-date view. The model also implements
 * [iface@Gtk.SelectionModel] and can be used to track and change the selected
 * page.
 *
 * Returns: (transfer full): a `GtkSelectionModel` for the pages of @self
 */
GtkSelectionModel *
brk_tab_view_get_pages (BrkTabView *self)
{
  g_return_val_if_fail (BRK_IS_TAB_VIEW (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  g_set_weak_pointer (&self->pages, brk_tab_pages_new (self));

  return self->pages;
}

/**
 * brk_tab_view_invalidate_thumbnails:
 * @self: a tab view
 *
 * Invalidates thumbnails for all pages in @self.
 *
 * This is a convenience method, equivalent to calling
 * [method@TabPage.invalidate_thumbnail] on each page.
 *
 * Since: 1.3
 */
void
brk_tab_view_invalidate_thumbnails (BrkTabView *self)
{
  int i;
  g_return_if_fail (BRK_IS_TAB_VIEW (self));

  for (i = 0; i < self->n_pages; i++) {
    BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

    brk_tab_page_invalidate_thumbnail (page);
  }
}

BrkTabView *
brk_tab_view_create_window (BrkTabView *self)
{
  BrkTabView *new_view = NULL;

  g_signal_emit (self, signals[SIGNAL_CREATE_WINDOW], 0, &new_view);

  if (!new_view) {
    g_critical ("BrkTabView::create-window handler must not return NULL");

    return NULL;
  }

  new_view->transfer_count = self->transfer_count;

  return new_view;
}

void
brk_tab_view_open_overview (BrkTabView *self)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));

  if (self->overview_count == 0 && gtk_widget_get_mapped (GTK_WIDGET (self))) {
    int i;

    for (i = 0; i < self->n_pages; i++) {
      BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

      if (page->live_thumbnail || page->invalidated)
        gtk_widget_set_child_visible (page->bin, TRUE);
    }

    gtk_widget_queue_allocate (GTK_WIDGET (self));
  }

  self->overview_count++;
}

void
brk_tab_view_close_overview (BrkTabView *self)
{
  g_return_if_fail (BRK_IS_TAB_VIEW (self));

  self->overview_count--;

  if (self->overview_count == 0) {
    int i;

    for (i = 0; i < self->n_pages; i++) {
      BrkTabPage *page = brk_tab_view_get_nth_page (self, i);

      if (page->live_thumbnail || page->invalidated)
        gtk_widget_set_child_visible (page->bin,
                                      page == self->selected_page);
    }

    gtk_widget_queue_allocate (GTK_WIDGET (self));
  }

  g_assert (self->overview_count >= 0);
}
