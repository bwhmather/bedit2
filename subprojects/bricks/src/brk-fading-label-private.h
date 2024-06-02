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

#define BRK_TYPE_FADING_LABEL (brk_fading_label_get_type())

G_DECLARE_FINAL_TYPE (BrkFadingLabel, brk_fading_label, BRK, FADING_LABEL, GtkWidget)

const char *brk_fading_label_get_label (BrkFadingLabel *self);
void        brk_fading_label_set_label (BrkFadingLabel *self,
                                        const char     *label);

float brk_fading_label_get_align (BrkFadingLabel *self);
void  brk_fading_label_set_align (BrkFadingLabel *self,
                                  float           align);

G_END_DECLS
