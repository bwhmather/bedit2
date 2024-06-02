/*
 * Copyright (C) 2019 Alice Mikhaylenko <alicem@gnome.org>
 * Copyright (C) 2021-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "brk-tab-button.h"

#include "brk-indicator-bin-private.h"
#include "brk-marshalers.h"

/* Copied from GtkInspector code */
#define XFT_DPI_MULTIPLIER (96.0 * PANGO_SCALE)

/**
 * BrkTabButton:
 *
 * A button that displays the number of [class@TabView] pages.
 *
 * <picture>
 *   <source srcset="tab-button-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="tab-button.png" alt="tab-button">
 * </picture>
 *
 * `BrkTabButton` is a button that displays the number of pages in a given
 * `BrkTabView`, as well as whether one of the inactive pages needs attention.
 *
 * It's intended to be used as a visible indicator when there's no visible tab
 * bar, typically opening an [class@TabOverview] on click, e.g. via the
 * `overview.open` action name:
 *
 * ```xml
 * <object class="BrkTabButton">
 *   <property name="view">view</property>
 *   <property name="action-name">overview.open</property>
 * </object>
 * ```
 *
 * ## CSS nodes
 *
 * `BrkTabButton` has a main CSS node with name `tabbutton`.
 *
 * # Accessibility
 *
 * `BrkTabButton` uses the `GTK_ACCESSIBLE_ROLE_BUTTON` role.
 *
 * Since: 1.3
 */

struct _BrkTabButton
{
  GtkWidget parent_instance;

  GtkWidget *button;
  GtkLabel *label;
  GtkImage *icon;
  BrkIndicatorBin *indicator;

  BrkTabView *view;
};

static void brk_tab_button_actionable_init (GtkActionableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (BrkTabButton, brk_tab_button, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ACTIONABLE, brk_tab_button_actionable_init))

enum {
  PROP_0,
  PROP_VIEW,

  /* actionable properties */
  PROP_ACTION_NAME,
  PROP_ACTION_TARGET,
  LAST_PROP = PROP_ACTION_NAME
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_CLICKED,
  SIGNAL_ACTIVATE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
clicked_cb (BrkTabButton *self)
{
  g_signal_emit (self, signals[SIGNAL_CLICKED], 0);
}

static void
activate_cb (BrkTabButton *self)
{
  g_signal_emit_by_name (self->button, "activate");
}

static void
update_label_scale (BrkTabButton *self,
                    GtkSettings  *settings)
{
  int xft_dpi;
  PangoAttrList *attrs;
  PangoAttribute *scale_attribute;

  g_object_get (settings, "gtk-xft-dpi", &xft_dpi, NULL);

  if (xft_dpi == 0)
    xft_dpi = 96 * PANGO_SCALE;

  attrs = pango_attr_list_new ();

  scale_attribute = pango_attr_scale_new (XFT_DPI_MULTIPLIER / (double) xft_dpi);

  pango_attr_list_change (attrs, scale_attribute);

  gtk_label_set_attributes (self->label, attrs);

  pango_attr_list_unref (attrs);
}

static void
xft_dpi_changed (BrkTabButton *self,
                 GParamSpec   *pspec,
                 GtkSettings  *settings)
{
  update_label_scale (self, settings);
}

static void
update_icon (BrkTabButton *self)
{
  gboolean display_label = FALSE;
  gboolean small_label = FALSE;
  const char *icon_name = "brk-tab-counter-symbolic";
  char *label_text = NULL;

  if (self->view) {
    guint n_pages = brk_tab_view_get_n_pages (self->view);

    small_label = n_pages >= 10;

    if (n_pages < 100) {
      display_label = TRUE;
      label_text = g_strdup_printf ("%u", n_pages);
    } else {
      icon_name = "brk-tab-overflow-symbolic";
    }
  }

  if (small_label)
    gtk_widget_add_css_class (GTK_WIDGET (self->label), "small");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self->label), "small");

  gtk_widget_set_visible (GTK_WIDGET (self->label), display_label);
  gtk_label_set_text (self->label, label_text);
  gtk_image_set_from_icon_name (self->icon, icon_name);

  g_free (label_text);
}

static void
update_needs_attention (BrkTabButton *self)
{
  gboolean needs_attention = FALSE;

  if (self->view) {
    int i, n;

    n = brk_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++) {
      BrkTabPage *page = brk_tab_view_get_nth_page (self->view, i);

      if (brk_tab_page_get_selected (page))
        continue;

      if (!brk_tab_page_get_needs_attention (page))
        continue;

      needs_attention = TRUE;
      break;
    }
  }

  brk_indicator_bin_set_needs_attention (BRK_INDICATOR_BIN (self->indicator),
                                         needs_attention);
}

static void
page_attached_cb (BrkTabButton *self,
                  BrkTabPage   *page)
{
  g_signal_connect_object (page, "notify::needs-attention",
                           G_CALLBACK (update_needs_attention), self,
                           G_CONNECT_SWAPPED);

  update_needs_attention (self);
}

static void
page_detached_cb (BrkTabButton *self,
                  BrkTabPage   *page)
{
  g_signal_handlers_disconnect_by_func (page, update_needs_attention, self);

  update_needs_attention (self);
}

static void
brk_tab_button_dispose (GObject *object)
{
  BrkTabButton *self = BRK_TAB_BUTTON (object);

  brk_tab_button_set_view (self, NULL);

  gtk_widget_dispose_template (GTK_WIDGET (self), BRK_TYPE_TAB_BUTTON);

  G_OBJECT_CLASS (brk_tab_button_parent_class)->dispose (object);
}

static void
brk_tab_button_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  BrkTabButton *self = BRK_TAB_BUTTON (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, brk_tab_button_get_view (self));
    break;
  case PROP_ACTION_NAME:
    g_value_set_string (value, gtk_actionable_get_action_name (GTK_ACTIONABLE (self)));
    break;
  case PROP_ACTION_TARGET:
    g_value_set_variant (value, gtk_actionable_get_action_target_value (GTK_ACTIONABLE (self)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_button_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  BrkTabButton *self = BRK_TAB_BUTTON (object);

  switch (prop_id) {
  case PROP_VIEW:
    brk_tab_button_set_view (self, g_value_get_object (value));
    break;
  case PROP_ACTION_NAME:
    gtk_actionable_set_action_name (GTK_ACTIONABLE (self), g_value_get_string (value));
    break;
  case PROP_ACTION_TARGET:
    gtk_actionable_set_action_target_value (GTK_ACTIONABLE (self), g_value_get_variant (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_tab_button_class_init (BrkTabButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = brk_tab_button_dispose;
  object_class->get_property = brk_tab_button_get_property;
  object_class->set_property = brk_tab_button_set_property;

  /**
   * BrkTabButton:view: (attributes org.gtk.Property.get=brk_tab_button_get_view org.gtk.Property.set=brk_tab_button_set_view)
   *
   * The view the tab button displays.
   *
   * Since: 1.3
   */
  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         BRK_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  g_object_class_override_property (object_class, PROP_ACTION_NAME, "action-name");
  g_object_class_override_property (object_class, PROP_ACTION_TARGET, "action-target");

  /**
   * BrkTabButton::clicked:
   * @self: the object that received the signal
   *
   * Emitted when the button has been activated (pressed and released).
   *
   * Since: 1.3
   */
  signals[SIGNAL_CLICKED] =
    g_signal_new ("clicked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLICKED],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__VOIDv);

  /**
   * BrkTabButton::activate:
   * @self: the object which received the signal.
   *
   * Emitted to animate press then release.
   *
   * This is an action signal. Applications should never connect to this signal,
   * but use the [signal@TabButton::clicked] signal.
   *
   * Since: 1.3
   */
  signals[SIGNAL_ACTIVATE] =
    g_signal_new ("activate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  0,
                  NULL, NULL,
                  brk_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATE],
                              G_TYPE_FROM_CLASS (klass),
                              brk_marshal_VOID__VOIDv);

  gtk_widget_class_set_activate_signal (widget_class, signals[SIGNAL_ACTIVATE]);

  g_signal_override_class_handler ("activate",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (activate_cb));

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/com/bwhmather/Bricks/ui/brk-tab-button.ui");

  gtk_widget_class_bind_template_child (widget_class, BrkTabButton, button);
  gtk_widget_class_bind_template_child (widget_class, BrkTabButton, label);
  gtk_widget_class_bind_template_child (widget_class, BrkTabButton, icon);
  gtk_widget_class_bind_template_child (widget_class, BrkTabButton, indicator);
  gtk_widget_class_bind_template_callback (widget_class, clicked_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "tabbutton");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_BUTTON);

  g_type_ensure (BRK_TYPE_INDICATOR_BIN);
}

static void
brk_tab_button_init (BrkTabButton *self)
{
  GtkSettings *settings;

  gtk_widget_init_template (GTK_WIDGET (self));

  update_icon (self);

  settings = gtk_widget_get_settings (GTK_WIDGET (self));

  update_label_scale (self, settings);
  g_signal_connect_object (settings, "notify::gtk-xft-dpi",
                           G_CALLBACK (xft_dpi_changed), self,
                           G_CONNECT_SWAPPED);
}

static const char *
brk_tab_button_get_action_name (GtkActionable *actionable)
{
  BrkTabButton *self = BRK_TAB_BUTTON (actionable);

  return gtk_actionable_get_action_name (GTK_ACTIONABLE (self->button));
}

static void
brk_tab_button_set_action_name (GtkActionable *actionable,
                                const char    *action_name)
{
  BrkTabButton *self = BRK_TAB_BUTTON (actionable);

  gtk_actionable_set_action_name (GTK_ACTIONABLE (self->button),
                                  action_name);
}

static GVariant *
brk_tab_button_get_action_target_value (GtkActionable *actionable)
{
  BrkTabButton *self = BRK_TAB_BUTTON (actionable);

  return gtk_actionable_get_action_target_value (GTK_ACTIONABLE (self->button));
}

static void
brk_tab_button_set_action_target_value (GtkActionable *actionable,
                                        GVariant      *action_target)
{
  BrkTabButton *self = BRK_TAB_BUTTON (actionable);

  gtk_actionable_set_action_target_value (GTK_ACTIONABLE (self->button),
                                          action_target);
}

static void
brk_tab_button_actionable_init (GtkActionableInterface *iface)
{
  iface->get_action_name = brk_tab_button_get_action_name;
  iface->set_action_name = brk_tab_button_set_action_name;
  iface->get_action_target_value = brk_tab_button_get_action_target_value;
  iface->set_action_target_value = brk_tab_button_set_action_target_value;
}

/**
 * brk_tab_button_new:
 *
 * Creates a new `BrkTabButton`.
 *
 * Returns: the newly created `BrkTabButton`
 *
 * Since: 1.3
 */
GtkWidget *
brk_tab_button_new (void)
{
  return g_object_new (BRK_TYPE_TAB_BUTTON, NULL);
}

/**
 * brk_tab_button_get_view: (attributes org.gtk.Method.get_property=view)
 * @self: a tab button
 *
 * Gets the tab view @self displays.
 *
 * Returns: (transfer none) (nullable): the tab view
 *
 * Since: 1.3
 */
BrkTabView *
brk_tab_button_get_view (BrkTabButton *self)
{
  g_return_val_if_fail (BRK_IS_TAB_BUTTON (self), NULL);

  return self->view;
}

/**
 * brk_tab_button_set_view: (attributes org.gtk.Method.set_property=view)
 * @self: a tab button
 * @view: (nullable): a tab view
 *
 * Sets the tab view to display.
 *
 * Since: 1.3
 */
void
brk_tab_button_set_view (BrkTabButton *self,
                         BrkTabView   *view)
{
  g_return_if_fail (BRK_IS_TAB_BUTTON (self));
  g_return_if_fail (view == NULL || BRK_IS_TAB_VIEW (view));

  if (self->view == view)
    return;

  if (self->view) {
    int i, n;

    g_signal_handlers_disconnect_by_func (self->view, update_icon, self);
    g_signal_handlers_disconnect_by_func (self->view, update_needs_attention, self);
    g_signal_handlers_disconnect_by_func (self->view, page_attached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_detached_cb, self);

    n = brk_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++)
      page_detached_cb (self, brk_tab_view_get_nth_page (self->view, i));
  }

  g_set_object (&self->view, view);

  if (self->view) {
    int i, n;

    g_signal_connect_object (self->view, "notify::n-pages",
                             G_CALLBACK (update_icon), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "notify::selected-page",
                             G_CALLBACK (update_needs_attention), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-attached",
                             G_CALLBACK (page_attached_cb), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-detached",
                             G_CALLBACK (page_detached_cb), self,
                             G_CONNECT_SWAPPED);

    n = brk_tab_view_get_n_pages (self->view);

    for (i = 0; i < n; i++)
      page_attached_cb (self, brk_tab_view_get_nth_page (self->view, i));
  }

  update_icon (self);
  update_needs_attention (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW]);
}
