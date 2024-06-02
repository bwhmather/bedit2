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

#include <gtk/gtk.h>
#include "brk-tab-view.h"

G_BEGIN_DECLS

#define BRK_TYPE_TAB (brk_tab_get_type())

G_DECLARE_FINAL_TYPE (BrkTab, brk_tab, BRK, TAB, GtkWidget)

BrkTab *brk_tab_new (BrkTabView *view) G_GNUC_WARN_UNUSED_RESULT;

BrkTabPage *brk_tab_get_page (BrkTab     *self);
void        brk_tab_set_page (BrkTab     *self,
                              BrkTabPage *page);

gboolean brk_tab_get_dragging (BrkTab   *self);
void     brk_tab_set_dragging (BrkTab   *self,
                               gboolean  dragging);

gboolean brk_tab_get_inverted (BrkTab   *self);
void     brk_tab_set_inverted (BrkTab   *self,
                               gboolean  inverted);

void brk_tab_set_fully_visible (BrkTab   *self,
                                gboolean  fully_visible);

void brk_tab_setup_extra_drop_target (BrkTab        *self,
                                      GdkDragAction  actions,
                                      GType         *types,
                                      gsize          n_types);

void brk_tab_set_extra_drag_preload (BrkTab   *self,
                                     gboolean  preload);

G_END_DECLS
