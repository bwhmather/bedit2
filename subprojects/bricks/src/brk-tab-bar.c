/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "brk-tab-bar-private.h"

#include "brk-bin.h"
#include "brk-tab-box-private.h"
#include "brk-widget-utils-private.h"

/**
 * BrkTabBar:
 *
 * A tab bar for [class@TabView].
 *
 * <picture>
 *   <source srcset="tab-bar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="tab-bar.png" alt="tab-bar">
 * </picture>
 *
 * The `BrkTabBar` widget is a tab bar that can be used with conjunction with
 * `BrkTabView`. It is typically used as a top bar within [class@ToolbarView].
 *
 * `BrkTabBar` can autohide and can optionally contain action widgets on both
 * sides of the tabs.
 *
 * When there's not enough space to show all the tabs, `BrkTabBar` will scroll
 * them.
 *
 * ## CSS nodes
 *
 * `BrkTabBar` has a single CSS node with name `tabbar`.
 */

struct _BrkTabBar
{
  GtkWidget parent_instance;

  GtkRevealer *revealer;
  BrkBin *start_action_bin;
  BrkBin *end_action_bin;

  BrkTabBox *box;
  GtkScrolledWindow *scrolled_window;

  BrkTabView *view;
  gboolean autohide;

  GdkDragAction extra_drag_preferred_action;

  gboolean is_overflowing;
  gboolean resize_frozen;
};

static void brk_tab_bar_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (BrkTabBar, brk_tab_bar, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, brk_tab_bar_buildable_init))

enum {
  PROP_0,
  PROP_VIEW,
  PROP_START_ACTION_WIDGET,
  PROP_END_ACTION_WIDGET,
  PROP_AUTOHIDE,
  PROP_TABS_REVEALED,
  PROP_EXPAND_TABS,
  PROP_INVERTED,
  PROP_IS_OVERFLOWING,
  PROP_EXTRA_DRAG_PRELOAD,
  PROP_EXTRA_DRAG_PREFERRED_ACTION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_EXTRA_DRAG_VALUE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
set_tabs_revealed (BrkTabBar *self,
                   gboolean   tabs_revealed)
{
  if (tabs_revealed == brk_tab_bar_get_tabs_revealed (self))
    return;

  gtk_revealer_set_reveal_child (self->revealer, tabs_revealed);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TABS_REVEALED]);
}

static void
update_autohide_cb (BrkTabBar *self)
{
  int n_tabs = 0;
  gboolean is_transferring_page;

  if (!self->view) {
    set_tabs_revealed (self, FALSE);

    return;
  }

  if (!self->autohide) {
    set_tabs_revealed (self, TRUE);

    return;
  }

  n_tabs = brk_tab_view_get_n_pages (self->view);
  is_transferring_page = brk_tab_view_get_is_transferring_page (self->view);

  set_tabs_revealed (self, n_tabs > 1 || is_transferring_page);
}

static void
notify_selected_page_cb (BrkTabBar *self)
{
  BrkTabPage *page = brk_tab_view_get_selected_page (self->view);

  if (!page)
    return;

  brk_tab_box_select_page (self->box, page);
}

static inline gboolean
is_overflowing (GtkAdjustment *adj)
{
  double lower, upper, page_size;

  lower = gtk_adjustment_get_lower (adj);
  upper = gtk_adjustment_get_upper (adj);
  page_size = gtk_adjustment_get_page_size (adj);
  return upper - lower > page_size;
}

static void
update_is_overflowing (BrkTabBar *self)
{
  GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment (self->scrolled_window);
  gboolean overflowing = is_overflowing (adj);

  if (overflowing == self->is_overflowing)
    return;

  overflowing |= self->resize_frozen;

  if (overflowing == self->is_overflowing)
    return;

  self->is_overflowing = overflowing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_IS_OVERFLOWING]);
}

static void
notify_resize_frozen_cb (BrkTabBar *self)
{
  gboolean frozen;

  g_object_get (self->box, "resize-frozen", &frozen, NULL);

  self->resize_frozen = frozen;

  update_is_overflowing (self);
}

static void
stop_kinetic_scrolling_cb (GtkScrolledWindow *scrolled_window)
{
  /* HACK: Need to cancel kinetic scrolling. If only the built-in adjustment
   * animation API was public, we wouldn't have to do any of this... */
  gtk_scrolled_window_set_kinetic_scrolling (scrolled_window, FALSE);
  gtk_scrolled_window_set_kinetic_scrolling (scrolled_window, TRUE);
}

static void
set_extra_drag_preferred_action (BrkTabBar    *self,
                                 GdkDragAction preferred_action)
{
  self->extra_drag_preferred_action = preferred_action;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_DRAG_PREFERRED_ACTION]);
}

static gboolean
extra_drag_drop_cb (BrkTabBar    *self,
                    BrkTabPage   *page,
                    GValue       *value,
                    GdkDragAction preferred_action)
{
  gboolean ret = GDK_EVENT_PROPAGATE;

  set_extra_drag_preferred_action (self, preferred_action);
  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, page, value, &ret);
  set_extra_drag_preferred_action (self, 0);

  return ret;
}

static GdkDragAction
extra_drag_value_cb (BrkTabBar  *self,
                     BrkTabPage *page,
                     GValue     *value)
{
  GdkDragAction preferred_action;

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, page, value, &preferred_action);

  return preferred_action;
}

static GdkDragAction
extra_drag_value_notify (BrkTabBar *self,
                         GValue    *value)
{
  return GDK_ACTION_ALL;
}

static void
view_destroy_cb (BrkTabBar *self)
{
  brk_tab_bar_set_view (self, NULL);
}

static gboolean
brk_tab_bar_focus (GtkWidget        *widget,
                   GtkDirectionType  direction)
{
  BrkTabBar *self = BRK_TAB_BAR (widget);
  gboolean is_rtl;
  GtkDirectionType start, end;

  if (!brk_tab_bar_get_tabs_revealed (self))
    return GDK_EVENT_PROPAGATE;

  if (!gtk_widget_get_focus_child (widget))
    return gtk_widget_child_focus (GTK_WIDGET (self->box), direction);

  is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  start = is_rtl ? GTK_DIR_RIGHT : GTK_DIR_LEFT;
  end = is_rtl ? GTK_DIR_LEFT : GTK_DIR_RIGHT;

  if (direction == start) {
    if (brk_tab_view_select_previous_page (self->view))
      return GDK_EVENT_STOP;

    return gtk_widget_keynav_failed (widget, direction);
  }

  if (direction == end) {
    if (brk_tab_view_select_next_page (self->view))
      return GDK_EVENT_STOP;

    return gtk_widget_keynav_failed (widget, direction);
  }

  return GDK_EVENT_PROPAGATE;
}

static void
brk_tab_bar_dispose (GObject *object)
{
  BrkTabBar *self = BRK_TAB_BAR (object);

  brk_tab_bar_set_view (self, NULL);

  gtk_widget_dispose_template (GTK_WIDGET (self), BRK_TYPE_TAB_BAR);

  G_OBJECT_CLASS (brk_tab_bar_parent_class)->dispose (object);
}

static void
brk_tab_bar_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  BrkTabBar *self = BRK_TAB_BAR (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, brk_tab_bar_get_view (self));
    break;

  case PROP_START_ACTION_WIDGET:
    g_value_set_object (value, brk_tab_bar_get_start_action_widget (self));
    break;

  case PROP_END_ACTION_WIDGET:
    g_value_set_object (value, brk_tab_bar_get_end_action_widget (self));
    break;

  case PROP_AUTOHIDE:
    g_value_set_boolean (value, brk_tab_bar_get_autohide (self));
    break;

  case PROP_TABS_REVEALED:
    g_value_set_boolean (value, brk_tab_bar_get_tabs_revealed (self));
    break;

  case PROP_EXPAND_TABS:
    g_value_set_boolean (value, brk_tab_bar_get_expand_tabs (self));
    break;

  case PROP_INVERTED:
    g_value_set_boolean (value, brk_tab_bar_get_inverted (self));
    break;

  case PROP_IS_OVERFLOWING:
    g_value_set_boolean (value, brk_tab_bar_get_is_overflowing (self));
    break;

  case PROP_EXTRA_DRAG_PREFERRED_ACTION:
    g_value_set_flags (value, brk_tab_bar_get_extra_drag_preferred_action (self));
    break;

  case PROP_EXTRA_DRAG_PRELOAD:
    g_value_set_boolean (value, brk_tab_bar_get_extra_drag_preload (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_bar_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  BrkTabBar *self = BRK_TAB_BAR (object);

  switch (prop_id) {
  case PROP_VIEW:
    brk_tab_bar_set_view (self, g_value_get_object (value));
    break;

  case PROP_START_ACTION_WIDGET:
    brk_tab_bar_set_start_action_widget (self, g_value_get_object (value));
    break;

  case PROP_END_ACTION_WIDGET:
    brk_tab_bar_set_end_action_widget (self, g_value_get_object (value));
    break;

  case PROP_AUTOHIDE:
    brk_tab_bar_set_autohide (self, g_value_get_boolean (value));
    break;

  case PROP_EXPAND_TABS:
    brk_tab_bar_set_expand_tabs (self, g_value_get_boolean (value));
    break;

  case PROP_INVERTED:
    brk_tab_bar_set_inverted (self, g_value_get_boolean (value));
    break;

  case PROP_EXTRA_DRAG_PRELOAD:
    brk_tab_bar_set_extra_drag_preload (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_bar_class_init (BrkTabBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = brk_tab_bar_dispose;
  object_class->get_property = brk_tab_bar_get_property;
  object_class->set_property = brk_tab_bar_set_property;

  widget_class->focus = brk_tab_bar_focus;
  widget_class->compute_expand = brk_widget_compute_expand;

  /**
   * BrkTabBar:view: (attributes org.gtk.Property.get=brk_tab_bar_get_view org.gtk.Property.set=brk_tab_bar_set_view)
   *
   * The tab view the tab bar controls.
   */
  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         BRK_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:start-action-widget: (attributes org.gtk.Property.get=brk_tab_bar_get_start_action_widget org.gtk.Property.set=brk_tab_bar_set_start_action_widget)
   *
   * The widget shown before the tabs.
   */
  props[PROP_START_ACTION_WIDGET] =
    g_param_spec_object ("start-action-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:end-action-widget: (attributes org.gtk.Property.get=brk_tab_bar_get_end_action_widget org.gtk.Property.set=brk_tab_bar_set_end_action_widget)
   *
   * The widget shown after the tabs.
   */
  props[PROP_END_ACTION_WIDGET] =
    g_param_spec_object ("end-action-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:autohide: (attributes org.gtk.Property.get=brk_tab_bar_get_autohide org.gtk.Property.set=brk_tab_bar_set_autohide)
   *
   * Whether the tabs automatically hide.
   *
   * If set to `TRUE`, the tab bar disappears when [property@TabBar:view] has 0
   * or 1 tab and no tab is being transferred.
   *
   * See [property@TabBar:tabs-revealed].
   */
  props[PROP_AUTOHIDE] =
    g_param_spec_boolean ("autohide", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:tabs-revealed: (attributes org.gtk.Property.get=brk_tab_bar_get_tabs_revealed)
   *
   * Whether the tabs are currently revealed.
   *
   * See [property@TabBar:autohide].
   */
  props[PROP_TABS_REVEALED] =
    g_param_spec_boolean ("tabs-revealed", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * BrkTabBar:expand-tabs: (attributes org.gtk.Property.get=brk_tab_bar_get_expand_tabs org.gtk.Property.set=brk_tab_bar_set_expand_tabs)
   *
   * Whether tabs expand to full width.
   *
   * If set to `TRUE`, the tabs will always vary width filling the whole width
   * when possible, otherwise tabs will always have the minimum possible size.
   */
  props[PROP_EXPAND_TABS] =
    g_param_spec_boolean ("expand-tabs", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:inverted: (attributes org.gtk.Property.get=brk_tab_bar_get_inverted org.gtk.Property.set=brk_tab_bar_set_inverted)
   *
   * Whether tabs use inverted layout.
   *
   * If set to `TRUE`, tabs will have the close button at the  beginning and the
   * indicator at the end rather than the opposite.
   */
  props[PROP_INVERTED] =
    g_param_spec_boolean ("inverted", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:is-overflowing: (attributes org.gtk.Property.get=brk_tab_bar_get_is_overflowing)
   *
   * Whether the tab bar is overflowing.
   *
   * If `TRUE`, all tabs cannot be displayed at once and require scrolling.
   */
  props[PROP_IS_OVERFLOWING] =
    g_param_spec_boolean ("is-overflowing", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * BrkTabBar:extra-drag-preferred-action: (attributes org.gtk.Property.get=brk_tab_bar_get_extra_drag_preferred_action)
   *
   * The unique action on the `current-drop` of the
   * [signal@TabBar::extra-drag-drop].
   *
   * This property should only be used during a [signal@TabBar::extra-drag-drop]
   * and is always a subset of what was originally passed to
   * [method@TabBar.setup_extra_drop_target].
   *
   * Since: 1.4
   */
  props[PROP_EXTRA_DRAG_PREFERRED_ACTION] =
    g_param_spec_flags ("extra-drag-preferred-action", NULL, NULL,
                       GDK_TYPE_DRAG_ACTION, 0,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * BrkTabBar:extra-drag-preload: (attributes org.gtk.Property.get=brk_tab_bar_get_extra_drag_preload org.gtk.Property.set=brk_tab_bar_set_extra_drag_preload)
   *
   * Whether the drop data should be preloaded on hover.
   *
   * See [property@Gtk.DropTarget:preload].
   *
   * Since: 1.3
   */
  props[PROP_EXTRA_DRAG_PRELOAD] =
    g_param_spec_boolean ("extra-drag-preload", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * BrkTabBar::extra-drag-drop:
   * @self: a tab bar
   * @page: the page matching the tab the content was dropped onto
   * @value: the `GValue` being dropped
   *
   * This signal is emitted when content is dropped onto a tab.
   *
   * The content must be of one of the types set up via
   * [method@TabBar.setup_extra_drop_target].
   *
   * See [signal@Gtk.DropTarget::drop].
   *
   * Returns: whether the drop was accepted for @page
   */
  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  G_TYPE_BOOLEAN,
                  2,
                  BRK_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  /**
   * BrkTabBar::extra-drag-value:
   * @self: a tab bar
   * @page: the page matching the tab the content was dropped onto
   * @value: the `GValue` being dropped
   *
   * This signal is emitted when the dropped content is preloaded.
   *
   * In order for data to be preloaded, [property@TabBar:extra-drag-preload]
   * must be set to `TRUE`.
   *
   * The content must be of one of the types set up via
   * [method@TabBar.setup_extra_drop_target].
   *
   * See [property@Gtk.DropTarget:value].
   *
   * Returns: the preferred action for the drop on @page
   *
   * Since: 1.3
   */
  signals[SIGNAL_EXTRA_DRAG_VALUE] =
    g_signal_new ("extra-drag-value",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  GDK_TYPE_DRAG_ACTION,
                  2,
                  BRK_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/bwhmather/Bricks/ui/brk-tab-bar.ui");
  gtk_widget_class_bind_template_child (widget_class, BrkTabBar, revealer);
  gtk_widget_class_bind_template_child (widget_class, BrkTabBar, box);
  gtk_widget_class_bind_template_child (widget_class, BrkTabBar, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, BrkTabBar, start_action_bin);
  gtk_widget_class_bind_template_child (widget_class, BrkTabBar, end_action_bin);
  gtk_widget_class_bind_template_callback (widget_class, notify_resize_frozen_cb);
  gtk_widget_class_bind_template_callback (widget_class, stop_kinetic_scrolling_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_drop_cb);
  gtk_widget_class_bind_template_callback (widget_class, extra_drag_value_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "tabbar");

  g_signal_override_class_handler ("extra-drag-value", G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (extra_drag_value_notify));

  g_type_ensure (BRK_TYPE_TAB_BOX);
}

static void
brk_tab_bar_init (BrkTabBar *self)
{
  GtkAdjustment *adj;

  self->autohide = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  adj = gtk_scrolled_window_get_hadjustment (self->scrolled_window);
  g_signal_connect_object (adj, "changed", G_CALLBACK (update_is_overflowing),
                           self, G_CONNECT_SWAPPED);
}

static void
brk_tab_bar_buildable_add_child (GtkBuildable *buildable,
                                 GtkBuilder   *builder,
                                 GObject      *child,
                                 const char   *type)
{
  BrkTabBar *self = BRK_TAB_BAR (buildable);

  if (!self->revealer) {
    gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (self));

    return;
  }

  if (!type || !g_strcmp0 (type, "start"))
    brk_tab_bar_set_start_action_widget (self, GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "end"))
    brk_tab_bar_set_end_action_widget (self, GTK_WIDGET (child));
  else
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (BRK_TAB_BAR (self), type);
}

static void
brk_tab_bar_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = brk_tab_bar_buildable_add_child;
}

gboolean
brk_tab_bar_tabs_have_visible_focus (BrkTabBar *self)
{
  GtkWidget *scroll_focus_child;

  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  scroll_focus_child = gtk_widget_get_focus_child (GTK_WIDGET (self->box));

  if (scroll_focus_child) {
    GtkWidget *tab = gtk_widget_get_first_child (scroll_focus_child);

    if (gtk_widget_has_visible_focus (tab))
      return TRUE;
  }

  return FALSE;
}

/**
 * brk_tab_bar_new:
 *
 * Creates a new `BrkTabBar`.
 *
 * Returns: the newly created `BrkTabBar`
 */
BrkTabBar *
brk_tab_bar_new (void)
{
  return g_object_new (BRK_TYPE_TAB_BAR, NULL);
}

/**
 * brk_tab_bar_get_view: (attributes org.gtk.Method.get_property=view)
 * @self: a tab bar
 *
 * Gets the tab view @self controls.
 *
 * Returns: (transfer none) (nullable): the view @self controls
 */
BrkTabView *
brk_tab_bar_get_view (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), NULL);

  return self->view;
}

/**
 * brk_tab_bar_set_view: (attributes org.gtk.Method.set_property=view)
 * @self: a tab bar
 * @view: (nullable): a tab view
 *
 * Sets the tab view @self controls.
 */
void
brk_tab_bar_set_view (BrkTabBar  *self,
                      BrkTabView *view)
{
  g_return_if_fail (BRK_IS_TAB_BAR (self));
  g_return_if_fail (view == NULL || BRK_IS_TAB_VIEW (view));

  if (self->view == view)
    return;

  if (self->view) {
    g_signal_handlers_disconnect_by_func (self->view, update_autohide_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, notify_selected_page_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, view_destroy_cb, self);

    brk_tab_box_set_view (self->box, NULL);
  }

  g_set_object (&self->view, view);

  if (self->view) {
    brk_tab_box_set_view (self->box, view);

    g_signal_connect_object (self->view, "notify::is-transferring-page",
                             G_CALLBACK (update_autohide_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::n-pages",
                             G_CALLBACK (update_autohide_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::selected-page",
                             G_CALLBACK (notify_selected_page_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "destroy",
                             G_CALLBACK (view_destroy_cb), self,
                             G_CONNECT_SWAPPED);
  }

  update_autohide_cb (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW]);
}

/**
 * brk_tab_bar_get_start_action_widget: (attributes org.gtk.Method.get_property=start-action-widget)
 * @self: a tab bar
 *
 * Gets the widget shown before the tabs.
 *
 * Returns: (transfer none) (nullable): the widget shown before the tabs
 */
GtkWidget *
brk_tab_bar_get_start_action_widget (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), NULL);

  return self->start_action_bin ? brk_bin_get_child (self->start_action_bin) : NULL;
}

/**
 * brk_tab_bar_set_start_action_widget: (attributes org.gtk.Method.set_property=start-action-widget)
 * @self: a tab bar
 * @widget: (transfer none) (nullable): the widget to show before the tabs
 *
 * Sets the widget to show before the tabs.
 */
void
brk_tab_bar_set_start_action_widget (BrkTabBar *self,
                                     GtkWidget *widget)
{
  GtkWidget *old_widget;

  g_return_if_fail (BRK_IS_TAB_BAR (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  old_widget = brk_bin_get_child (self->start_action_bin);

  if (old_widget == widget)
    return;

  brk_bin_set_child (self->start_action_bin, widget);
  gtk_widget_set_visible (GTK_WIDGET (self->start_action_bin), widget != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_START_ACTION_WIDGET]);
}

/**
 * brk_tab_bar_get_end_action_widget: (attributes org.gtk.Method.get_property=end-action-widget)
 * @self: a tab bar
 *
 * Gets the widget shown after the tabs.
 *
 * Returns: (transfer none) (nullable): the widget shown after the tabs
 */
GtkWidget *
brk_tab_bar_get_end_action_widget (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), NULL);

  return self->end_action_bin ? brk_bin_get_child (self->end_action_bin) : NULL;
}

/**
 * brk_tab_bar_set_end_action_widget: (attributes org.gtk.Method.set_property=end-action-widget)
 * @self: a tab bar
 * @widget: (transfer none) (nullable): the widget to show after the tabs
 *
 * Sets the widget to show after the tabs.
 */
void
brk_tab_bar_set_end_action_widget (BrkTabBar *self,
                                   GtkWidget *widget)
{
  GtkWidget *old_widget;

  g_return_if_fail (BRK_IS_TAB_BAR (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  old_widget = brk_bin_get_child (self->end_action_bin);

  if (old_widget == widget)
    return;

  brk_bin_set_child (self->end_action_bin, widget);
  gtk_widget_set_visible (GTK_WIDGET (self->end_action_bin), widget != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_END_ACTION_WIDGET]);
}

/**
 * brk_tab_bar_get_autohide: (attributes org.gtk.Method.get_property=autohide)
 * @self: a tab bar
 *
 * Gets whether the tabs automatically hide.
 *
 * Returns: whether the tabs automatically hide
 */
gboolean
brk_tab_bar_get_autohide (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  return self->autohide;
}

/**
 * brk_tab_bar_set_autohide: (attributes org.gtk.Method.set_property=autohide)
 * @self: a tab bar
 * @autohide: whether the tabs automatically hide
 *
 * Sets whether the tabs automatically hide.
 *
 * If set to `TRUE`, the tab bar disappears when [property@TabBar:view] has 0
 * or 1 tab and no tab is being transferred.
 *
 * See [property@TabBar:tabs-revealed].
 */
void
brk_tab_bar_set_autohide (BrkTabBar *self,
                          gboolean   autohide)
{
  g_return_if_fail (BRK_IS_TAB_BAR (self));

  autohide = !!autohide;

  if (autohide == self->autohide)
    return;

  self->autohide = autohide;

  update_autohide_cb (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_AUTOHIDE]);
}

/**
 * brk_tab_bar_get_tabs_revealed: (attributes org.gtk.Method.get_property=tabs-revealed)
 * @self: a tab bar
 *
 * Gets whether the tabs are currently revealed.
 *
 * See [property@TabBar:autohide].
 *
 * Returns: whether the tabs are currently revealed
 */
gboolean
brk_tab_bar_get_tabs_revealed (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  return gtk_revealer_get_reveal_child (self->revealer);
}

/**
 * brk_tab_bar_get_expand_tabs: (attributes org.gtk.Method.get_property=expand-tabs)
 * @self: a tab bar
 *
 * Gets whether tabs expand to full width.
 *
 * Returns: whether tabs expand to full width.
 */
gboolean
brk_tab_bar_get_expand_tabs (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  return brk_tab_box_get_expand_tabs (self->box);
}

/**
 * brk_tab_bar_set_expand_tabs: (attributes org.gtk.Method.set_property=expand-tabs)
 * @self: a tab bar
 * @expand_tabs: whether to expand tabs
 *
 * Sets whether tabs expand to full width.
 *
 * If set to `TRUE`, the tabs will always vary width filling the whole width
 * when possible, otherwise tabs will always have the minimum possible size.
 */
void
brk_tab_bar_set_expand_tabs (BrkTabBar *self,
                             gboolean   expand_tabs)
{
  g_return_if_fail (BRK_IS_TAB_BAR (self));

  expand_tabs = !!expand_tabs;

  if (brk_tab_bar_get_expand_tabs (self) == expand_tabs)
    return;

  brk_tab_box_set_expand_tabs (self->box, expand_tabs);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXPAND_TABS]);
}

/**
 * brk_tab_bar_get_inverted: (attributes org.gtk.Method.get_property=inverted)
 * @self: a tab bar
 *
 * Gets whether tabs use inverted layout.
 *
 * Returns: whether tabs use inverted layout
 */
gboolean
brk_tab_bar_get_inverted (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  return brk_tab_box_get_inverted (self->box);
}

/**
 * brk_tab_bar_set_inverted: (attributes org.gtk.Method.set_property=inverted)
 * @self: a tab bar
 * @inverted: whether tabs use inverted layout
 *
 * Sets whether tabs tabs use inverted layout.
 *
 * If set to `TRUE`, tabs will have the close button at the beginning and the
 * indicator at the end rather than the opposite.
 */
void
brk_tab_bar_set_inverted (BrkTabBar *self,
                          gboolean   inverted)
{
  g_return_if_fail (BRK_IS_TAB_BAR (self));

  inverted = !!inverted;

  if (brk_tab_bar_get_inverted (self) == inverted)
    return;

  brk_tab_box_set_inverted (self->box, inverted);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INVERTED]);
}

/**
 * brk_tab_bar_setup_extra_drop_target:
 * @self: a tab bar
 * @actions: the supported actions
 * @types: (nullable) (transfer none) (array length=n_types):
 *   all supported `GType`s that can be dropped
 * @n_types: number of @types
 *
 * Sets the supported types for this drop target.
 *
 * Sets up an extra drop target on tabs.
 *
 * This allows to drag arbitrary content onto tabs, for example URLs in a web
 * browser.
 *
 * If a tab is hovered for a certain period of time while dragging the content,
 * it will be automatically selected.
 *
 * The [signal@TabBar::extra-drag-drop] signal can be used to handle the drop.
 */
void
brk_tab_bar_setup_extra_drop_target (BrkTabBar     *self,
                                     GdkDragAction  actions,
                                     GType         *types,
                                     gsize          n_types)
{
  g_return_if_fail (BRK_IS_TAB_BAR (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  brk_tab_box_setup_extra_drop_target (self->box, actions, types, n_types);
}

/**
 * brk_tab_bar_get_extra_drag_preferred_action:
 * @self: a tab bar
 *
 * Gets the current action during a drop on the extra_drop_target.
 *
 * Returns: the drag action of the current drop.
 *
 * Since: 1.4
 */
GdkDragAction
brk_tab_bar_get_extra_drag_preferred_action (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), 0);

  return self->extra_drag_preferred_action;
}

/**
 * brk_tab_bar_get_extra_drag_preload:
 * @self: a tab bar
 *
 * Gets whether drop data should be preloaded on hover.
 *
 * Returns: whether drop data should be preloaded on hover
 *
 * Since: 1.3
 */
gboolean
brk_tab_bar_get_extra_drag_preload (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  return brk_tab_box_get_extra_drag_preload (self->box);
}

/**
 * brk_tab_bar_set_extra_drag_preload:
 * @self: a tab bar
 * @preload: whether to preload drop data
 *
 * Sets whether drop data should be preloaded on hover.
 *
 * See [property@Gtk.DropTarget:preload].
 *
 * Since: 1.3
 */
void
brk_tab_bar_set_extra_drag_preload (BrkTabBar *self,
                                    gboolean   preload)
{
  g_return_if_fail (BRK_IS_TAB_BAR (self));

  if (brk_tab_bar_get_extra_drag_preload (self) == preload)
    return;

  brk_tab_box_set_extra_drag_preload (self->box, preload);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_DRAG_PRELOAD]);
}

/**
 * brk_tab_bar_get_is_overflowing: (attributes org.gtk.Method.get_property=is-overflowing)
 * @self: a tab bar
 *
 * Gets whether @self is overflowing.
 *
 * If `TRUE`, all tabs cannot be displayed at once and require scrolling.
 *
 * Returns: whether @self is overflowing
 */
gboolean
brk_tab_bar_get_is_overflowing (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), FALSE);

  return self->is_overflowing;
}

BrkTabBox *
brk_tab_bar_get_tab_box (BrkTabBar *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BAR (self), NULL);

  return self->box;
}
