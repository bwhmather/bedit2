/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.> See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BRK_CRITICAL_CANNOT_REMOVE_CHILD(parent, child) \
G_STMT_START { \
  g_critical ("%s:%d: tried to remove non-child %p of type '%s' from %p of type '%s'", \
              __FILE__, __LINE__, \
              (child), \
              G_OBJECT_TYPE_NAME ((GObject*) (child)), \
              (parent), \
              G_OBJECT_TYPE_NAME ((GObject*) (parent))); \
} G_STMT_END

gboolean brk_widget_focus_child (GtkWidget        *widget,
                                 GtkDirectionType  direction);

gboolean brk_widget_grab_focus_self          (GtkWidget *widget);
gboolean brk_widget_grab_focus_child         (GtkWidget *widget);
gboolean brk_widget_grab_focus_child_or_self (GtkWidget *widget);

void brk_widget_compute_expand (GtkWidget *widget,
                                gboolean  *hexpand_p,
                                gboolean  *vexpand_p);

void brk_widget_compute_expand_horizontal_only (GtkWidget *widget,
                                                gboolean  *hexpand_p,
                                                gboolean  *vexpand_p);

GtkSizeRequestMode brk_widget_get_request_mode (GtkWidget *widget);

gboolean brk_widget_contains_passthrough (GtkWidget *widget,
                                          double     x,
                                          double     y);

gboolean brk_widget_lookup_color (GtkWidget  *widget,
                                  const char *name,
                                  GdkRGBA    *rgba);

gboolean brk_decoration_layout_prefers_start (const char *layout);

G_END_DECLS
