/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include "brk-version.h"

#include <gtk/gtk.h>
#include "brk-tab-view.h"

G_BEGIN_DECLS

#define BRK_TYPE_TAB_BUTTON (brk_tab_button_get_type())

BRK_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (BrkTabButton, brk_tab_button, BRK, TAB_BUTTON, GtkWidget)

BRK_AVAILABLE_IN_1_3
GtkWidget *brk_tab_button_new (void) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_1_3
BrkTabView *brk_tab_button_get_view (BrkTabButton *self);
BRK_AVAILABLE_IN_1_3
void        brk_tab_button_set_view (BrkTabButton *self,
                                     BrkTabView   *view);

G_END_DECLS
