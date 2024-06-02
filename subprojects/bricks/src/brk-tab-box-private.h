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

#define BRK_TYPE_TAB_BOX (brk_tab_box_get_type())

G_DECLARE_FINAL_TYPE (BrkTabBox, brk_tab_box, BRK, TAB_BOX, GtkWidget)

void brk_tab_box_set_view (BrkTabBox  *self,
                           BrkTabView *view);

void brk_tab_box_attach_page (BrkTabBox  *self,
                              BrkTabPage *page,
                              int         position);
void brk_tab_box_detach_page (BrkTabBox  *self,
                              BrkTabPage *page);
void brk_tab_box_select_page (BrkTabBox  *self,
                              BrkTabPage *page);

void brk_tab_box_try_focus_selected_tab (BrkTabBox  *self);
gboolean brk_tab_box_is_page_focused    (BrkTabBox  *self,
                                         BrkTabPage *page);

void brk_tab_box_setup_extra_drop_target (BrkTabBox     *self,
                                          GdkDragAction  actions,
                                          GType         *types,
                                          gsize          n_types);

gboolean brk_tab_box_get_extra_drag_preload (BrkTabBox *self);
void     brk_tab_box_set_extra_drag_preload (BrkTabBox *self,
                                             gboolean   preload);

gboolean brk_tab_box_get_expand_tabs (BrkTabBox *self);
void     brk_tab_box_set_expand_tabs (BrkTabBox *self,
                                      gboolean   expand_tabs);

gboolean brk_tab_box_get_inverted (BrkTabBox *self);
void     brk_tab_box_set_inverted (BrkTabBox *self,
                                   gboolean   inverted);

G_END_DECLS
