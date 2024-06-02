/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif

#include "brk-version.h"

#include <gtk/gtk.h>

#include "brk-animation.h"
#include "brk-easing.h"

G_BEGIN_DECLS

#define BRK_TYPE_TIMED_ANIMATION (brk_timed_animation_get_type())

BRK_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (BrkTimedAnimation, brk_timed_animation, BRK, TIMED_ANIMATION, BrkAnimation)

BRK_AVAILABLE_IN_ALL
BrkAnimation *brk_timed_animation_new (GtkWidget          *widget,
                                       double              from,
                                       double              to,
                                       guint               duration,
                                       BrkAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_ALL
double brk_timed_animation_get_value_from (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void   brk_timed_animation_set_value_from (BrkTimedAnimation *self,
                                           double             value);

BRK_AVAILABLE_IN_ALL
double brk_timed_animation_get_value_to (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void   brk_timed_animation_set_value_to (BrkTimedAnimation *self,
                                         double             value);

BRK_AVAILABLE_IN_ALL
guint brk_timed_animation_get_duration (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void  brk_timed_animation_set_duration (BrkTimedAnimation *self,
                                        guint              duration);

BRK_AVAILABLE_IN_ALL
BrkEasing brk_timed_animation_get_easing (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void      brk_timed_animation_set_easing (BrkTimedAnimation *self,
                                          BrkEasing          easing);

BRK_AVAILABLE_IN_ALL
guint brk_timed_animation_get_repeat_count (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void  brk_timed_animation_set_repeat_count (BrkTimedAnimation *self,
                                            guint              repeat_count);

BRK_AVAILABLE_IN_ALL
gboolean brk_timed_animation_get_reverse (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void     brk_timed_animation_set_reverse (BrkTimedAnimation *self,
                                          gboolean           reverse);

BRK_AVAILABLE_IN_ALL
gboolean brk_timed_animation_get_alternate (BrkTimedAnimation *self);
BRK_AVAILABLE_IN_ALL
void     brk_timed_animation_set_alternate (BrkTimedAnimation *self,
                                            gboolean           alternate);

G_END_DECLS
