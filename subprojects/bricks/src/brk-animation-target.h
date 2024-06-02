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

G_BEGIN_DECLS

#define BRK_TYPE_ANIMATION_TARGET (brk_animation_target_get_type())

BRK_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (BrkAnimationTarget, brk_animation_target, BRK, ANIMATION_TARGET, GObject)


/**
 * BrkAnimationTargetFunc:
 * @value: The animation value
 * @user_data: (nullable): The user data provided when creating the target
 *
 * Prototype for animation targets based on user callbacks.
 */
typedef void (*BrkAnimationTargetFunc) (double   value,
                                        gpointer user_data);

#define BRK_TYPE_CALLBACK_ANIMATION_TARGET (brk_callback_animation_target_get_type())

BRK_AVAILABLE_IN_ALL
GDK_DECLARE_INTERNAL_TYPE (BrkCallbackAnimationTarget, brk_callback_animation_target, BRK, CALLBACK_ANIMATION_TARGET, BrkAnimationTarget)

BRK_AVAILABLE_IN_ALL
BrkAnimationTarget *brk_callback_animation_target_new (BrkAnimationTargetFunc callback,
                                                       gpointer               user_data,
                                                       GDestroyNotify         destroy) G_GNUC_WARN_UNUSED_RESULT;

#define BRK_TYPE_PROPERTY_ANIMATION_TARGET (brk_property_animation_target_get_type())

BRK_AVAILABLE_IN_1_2
GDK_DECLARE_INTERNAL_TYPE (BrkPropertyAnimationTarget, brk_property_animation_target, BRK, PROPERTY_ANIMATION_TARGET, BrkAnimationTarget)

BRK_AVAILABLE_IN_1_2
BrkAnimationTarget *brk_property_animation_target_new           (GObject    *object,
                                                                 const char *property_name) G_GNUC_WARN_UNUSED_RESULT;
BRK_AVAILABLE_IN_1_2
BrkAnimationTarget *brk_property_animation_target_new_for_pspec (GObject    *object,
                                                                 GParamSpec *pspec) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_1_2
GObject    *brk_property_animation_target_get_object (BrkPropertyAnimationTarget *self);
BRK_AVAILABLE_IN_1_2
GParamSpec *brk_property_animation_target_get_pspec  (BrkPropertyAnimationTarget *self);

G_END_DECLS
