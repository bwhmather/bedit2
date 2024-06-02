/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmoprivate.h
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmoprivate.h
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BRK_TYPE_GIZMO (brk_gizmo_get_type())

G_DECLARE_FINAL_TYPE (BrkGizmo, brk_gizmo, BRK, GIZMO, GtkWidget)

typedef void     (* BrkGizmoMeasureFunc)  (BrkGizmo       *self,
                                           GtkOrientation  orientation,
                                           int             for_size,
                                           int            *minimum,
                                           int            *natural,
                                           int            *minimum_baseline,
                                           int            *natural_baseline);
typedef void     (* BrkGizmoAllocateFunc) (BrkGizmo *self,
                                           int       width,
                                           int       height,
                                           int       baseline);
typedef void     (* BrkGizmoSnapshotFunc) (BrkGizmo    *self,
                                           GtkSnapshot *snapshot);
typedef gboolean (* BrkGizmoContainsFunc) (BrkGizmo *self,
                                           double    x,
                                           double    y);
typedef gboolean (* BrkGizmoFocusFunc)    (BrkGizmo         *self,
                                           GtkDirectionType  direction);
typedef gboolean (* BrkGizmoGrabFocusFunc)(BrkGizmo         *self);

GtkWidget *brk_gizmo_new (const char            *css_name,
                          BrkGizmoMeasureFunc    measure_func,
                          BrkGizmoAllocateFunc   allocate_func,
                          BrkGizmoSnapshotFunc   snapshot_func,
                          BrkGizmoContainsFunc   contains_func,
                          BrkGizmoFocusFunc      focus_func,
                          BrkGizmoGrabFocusFunc  grab_focus_func) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *brk_gizmo_new_with_role (const char            *css_name,
                                    GtkAccessibleRole      role,
                                    BrkGizmoMeasureFunc    measure_func,
                                    BrkGizmoAllocateFunc   allocate_func,
                                    BrkGizmoSnapshotFunc   snapshot_func,
                                    BrkGizmoContainsFunc   contains_func,
                                    BrkGizmoFocusFunc      focus_func,
                                    BrkGizmoGrabFocusFunc  grab_focus_func) G_GNUC_WARN_UNUSED_RESULT;

void brk_gizmo_set_measure_func    (BrkGizmo              *self,
                                    BrkGizmoMeasureFunc    measure_func);
void brk_gizmo_set_allocate_func   (BrkGizmo              *self,
                                    BrkGizmoAllocateFunc   allocate_func);
void brk_gizmo_set_snapshot_func   (BrkGizmo              *self,
                                    BrkGizmoSnapshotFunc   snapshot_func);
void brk_gizmo_set_contains_func   (BrkGizmo              *self,
                                    BrkGizmoContainsFunc   contains_func);
void brk_gizmo_set_focus_func      (BrkGizmo              *self,
                                    BrkGizmoFocusFunc      focus_func);
void brk_gizmo_set_grab_focus_func (BrkGizmo              *self,
                                    BrkGizmoGrabFocusFunc  grab_focus_func);

G_END_DECLS
