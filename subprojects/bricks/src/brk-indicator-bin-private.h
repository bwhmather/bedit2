/*
 * Copyright (C) 2021 Purism SPC
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

G_BEGIN_DECLS

#define BRK_TYPE_INDICATOR_BIN (brk_indicator_bin_get_type())

G_DECLARE_FINAL_TYPE (BrkIndicatorBin, brk_indicator_bin, BRK, INDICATOR_BIN, GtkWidget)

GtkWidget *brk_indicator_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *brk_indicator_bin_get_child (BrkIndicatorBin *self);
void       brk_indicator_bin_set_child (BrkIndicatorBin *self,
                                        GtkWidget       *child);

gboolean brk_indicator_bin_get_needs_attention (BrkIndicatorBin *self);
void     brk_indicator_bin_set_needs_attention (BrkIndicatorBin *self,
                                                gboolean         needs_attention);

const char *brk_indicator_bin_get_badge (BrkIndicatorBin *self);
void        brk_indicator_bin_set_badge (BrkIndicatorBin *self,
                                         const char      *badge);

G_END_DECLS
