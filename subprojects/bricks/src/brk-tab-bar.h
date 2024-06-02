/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include "brk-version.h"

#include <gtk/gtk.h>
#include "brk-enums.h"
#include "brk-tab-view.h"

G_BEGIN_DECLS

#define BRK_TYPE_TAB_BAR (brk_tab_bar_get_type())

BRK_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (BrkTabBar, brk_tab_bar, BRK, TAB_BAR, GtkWidget)

BRK_AVAILABLE_IN_ALL
BrkTabBar *brk_tab_bar_new (void) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_ALL
BrkTabView *brk_tab_bar_get_view (BrkTabBar  *self);
BRK_AVAILABLE_IN_ALL
void        brk_tab_bar_set_view (BrkTabBar  *self,
                                  BrkTabView *view);

BRK_AVAILABLE_IN_ALL
GtkWidget *brk_tab_bar_get_start_action_widget (BrkTabBar *self);
BRK_AVAILABLE_IN_ALL
void       brk_tab_bar_set_start_action_widget (BrkTabBar *self,
                                                GtkWidget *widget);

BRK_AVAILABLE_IN_ALL
GtkWidget *brk_tab_bar_get_end_action_widget (BrkTabBar *self);
BRK_AVAILABLE_IN_ALL
void       brk_tab_bar_set_end_action_widget (BrkTabBar *self,
                                              GtkWidget *widget);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_bar_get_autohide (BrkTabBar *self);
BRK_AVAILABLE_IN_ALL
void     brk_tab_bar_set_autohide (BrkTabBar *self,
                                   gboolean   autohide);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_bar_get_tabs_revealed (BrkTabBar *self);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_bar_get_expand_tabs (BrkTabBar *self);
BRK_AVAILABLE_IN_ALL
void     brk_tab_bar_set_expand_tabs (BrkTabBar *self,
                                      gboolean   expand_tabs);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_bar_get_inverted (BrkTabBar *self);
BRK_AVAILABLE_IN_ALL
void     brk_tab_bar_set_inverted (BrkTabBar *self,
                                   gboolean   inverted);

BRK_AVAILABLE_IN_ALL
void brk_tab_bar_setup_extra_drop_target (BrkTabBar     *self,
                                          GdkDragAction  actions,
                                          GType         *types,
                                          gsize          n_types);

BRK_AVAILABLE_IN_1_4
GdkDragAction brk_tab_bar_get_extra_drag_preferred_action (BrkTabBar *self);

BRK_AVAILABLE_IN_1_3
gboolean brk_tab_bar_get_extra_drag_preload (BrkTabBar *self);
BRK_AVAILABLE_IN_1_3
void     brk_tab_bar_set_extra_drag_preload (BrkTabBar *self,
                                             gboolean   preload);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_bar_get_is_overflowing (BrkTabBar *self);

G_END_DECLS
