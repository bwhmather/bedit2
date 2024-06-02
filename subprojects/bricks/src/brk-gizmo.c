/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmo.c
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmo.c
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "brk-gizmo-private.h"

#include "brk-widget-utils-private.h"

struct _BrkGizmo
{
  GtkWidget parent_instance;

  BrkGizmoMeasureFunc   measure_func;
  BrkGizmoAllocateFunc  allocate_func;
  BrkGizmoSnapshotFunc  snapshot_func;
  BrkGizmoContainsFunc  contains_func;
  BrkGizmoFocusFunc     focus_func;
  BrkGizmoGrabFocusFunc grab_focus_func;
};

G_DEFINE_FINAL_TYPE (BrkGizmo, brk_gizmo, GTK_TYPE_WIDGET)

static void
brk_gizmo_measure (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  BrkGizmo *self = BRK_GIZMO (widget);

  if (self->measure_func)
    self->measure_func (self, orientation, for_size,
                        minimum, natural,
                        minimum_baseline, natural_baseline);
}

static void
brk_gizmo_size_allocate (GtkWidget *widget,
                         int        width,
                         int        height,
                         int        baseline)
{
  BrkGizmo *self = BRK_GIZMO (widget);

  if (self->allocate_func)
    self->allocate_func (self, width, height, baseline);
}

static void
brk_gizmo_snapshot (GtkWidget   *widget,
                    GtkSnapshot *snapshot)
{
  BrkGizmo *self = BRK_GIZMO (widget);

  if (self->snapshot_func)
    self->snapshot_func (self, snapshot);
  else
    GTK_WIDGET_CLASS (brk_gizmo_parent_class)->snapshot (widget, snapshot);
}

static gboolean
brk_gizmo_contains (GtkWidget *widget,
                    double     x,
                    double     y)
{
  BrkGizmo *self = BRK_GIZMO (widget);

  if (self->contains_func)
    return self->contains_func (self, x, y);
  else
    return GTK_WIDGET_CLASS (brk_gizmo_parent_class)->contains (widget, x, y);
}

static gboolean
brk_gizmo_focus (GtkWidget        *widget,
                 GtkDirectionType  direction)
{
  BrkGizmo *self = BRK_GIZMO (widget);

  if (self->focus_func)
    return self->focus_func (self, direction);

  return FALSE;
}

static gboolean
brk_gizmo_grab_focus (GtkWidget *widget)
{
  BrkGizmo *self = BRK_GIZMO (widget);

  if (self->grab_focus_func)
    return self->grab_focus_func (self);

  return FALSE;
}

static void
brk_gizmo_dispose (GObject *object)
{
  BrkGizmo *self = BRK_GIZMO (object);
  GtkWidget *widget = gtk_widget_get_first_child (GTK_WIDGET (self));

  while (widget) {
    GtkWidget *next = gtk_widget_get_next_sibling (widget);

    gtk_widget_unparent (widget);

    widget = next;
  }

  G_OBJECT_CLASS (brk_gizmo_parent_class)->dispose (object);
}

static void
brk_gizmo_class_init (BrkGizmoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = brk_gizmo_dispose;

  widget_class->measure = brk_gizmo_measure;
  widget_class->size_allocate = brk_gizmo_size_allocate;
  widget_class->snapshot = brk_gizmo_snapshot;
  widget_class->contains = brk_gizmo_contains;
  widget_class->grab_focus = brk_gizmo_grab_focus;
  widget_class->focus = brk_gizmo_focus;
  widget_class->compute_expand = brk_widget_compute_expand;
}

static void
brk_gizmo_init (BrkGizmo *self)
{
}

GtkWidget *
brk_gizmo_new (const char            *css_name,
               BrkGizmoMeasureFunc    measure_func,
               BrkGizmoAllocateFunc   allocate_func,
               BrkGizmoSnapshotFunc   snapshot_func,
               BrkGizmoContainsFunc   contains_func,
               BrkGizmoFocusFunc      focus_func,
               BrkGizmoGrabFocusFunc  grab_focus_func)
{
  BrkGizmo *gizmo = g_object_new (BRK_TYPE_GIZMO,
                                  "css-name", css_name,
                                  NULL);

  gizmo->measure_func  = measure_func;
  gizmo->allocate_func = allocate_func;
  gizmo->snapshot_func = snapshot_func;
  gizmo->contains_func = contains_func;
  gizmo->focus_func = focus_func;
  gizmo->grab_focus_func = grab_focus_func;

  return GTK_WIDGET (gizmo);
}

GtkWidget *
brk_gizmo_new_with_role (const char            *css_name,
                         GtkAccessibleRole      role,
                         BrkGizmoMeasureFunc    measure_func,
                         BrkGizmoAllocateFunc   allocate_func,
                         BrkGizmoSnapshotFunc   snapshot_func,
                         BrkGizmoContainsFunc   contains_func,
                         BrkGizmoFocusFunc      focus_func,
                         BrkGizmoGrabFocusFunc  grab_focus_func)
{
  BrkGizmo *gizmo = BRK_GIZMO (g_object_new (BRK_TYPE_GIZMO,
                                             "css-name", css_name,
                                             "accessible-role", role,
                                             NULL));

  gizmo->measure_func  = measure_func;
  gizmo->allocate_func = allocate_func;
  gizmo->snapshot_func = snapshot_func;
  gizmo->contains_func = contains_func;
  gizmo->focus_func = focus_func;
  gizmo->grab_focus_func = grab_focus_func;

  return GTK_WIDGET (gizmo);
}

void
brk_gizmo_set_measure_func (BrkGizmo            *self,
                            BrkGizmoMeasureFunc  measure_func)
{
  self->measure_func = measure_func;

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

void
brk_gizmo_set_allocate_func (BrkGizmo             *self,
                             BrkGizmoAllocateFunc  allocate_func)
{
  self->allocate_func = allocate_func;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

void
brk_gizmo_set_snapshot_func (BrkGizmo             *self,
                             BrkGizmoSnapshotFunc  snapshot_func)
{
  self->snapshot_func = snapshot_func;

  gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
brk_gizmo_set_contains_func (BrkGizmo             *self,
                             BrkGizmoContainsFunc  contains_func)
{
  self->contains_func = contains_func;

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

void
brk_gizmo_set_focus_func (BrkGizmo          *self,
                          BrkGizmoFocusFunc  focus_func)
{
  self->focus_func = focus_func;
}

void
brk_gizmo_set_grab_focus_func (BrkGizmo              *self,
                               BrkGizmoGrabFocusFunc  grab_focus_func)
{
  self->grab_focus_func = grab_focus_func;
}
