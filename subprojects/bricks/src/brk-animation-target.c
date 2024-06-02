/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "brk-animation-target-private.h"

/**
 * BrkAnimationTarget:
 *
 * Represents a value [class@Animation] can animate.
 */

/**
 * BrkCallbackAnimationTarget:
 *
 * An [class@AnimationTarget] that calls a given callback during the
 * animation.
 */

/**
 * BrkPropertyAnimationTarget:
 *
 * An [class@AnimationTarget] changing the value of a property of a
 * [class@GObject.Object] instance.
 *
 * Since: 1.2
 */

struct _BrkAnimationTarget
{
  GObject parent_instance;
};

struct _BrkAnimationTargetClass
{
  GObjectClass parent_class;

  void (*set_value) (BrkAnimationTarget *self,
                     double              value);
};

G_DEFINE_ABSTRACT_TYPE (BrkAnimationTarget, brk_animation_target, G_TYPE_OBJECT)

static void
brk_animation_target_class_init (BrkAnimationTargetClass *klass)
{
}

static void
brk_animation_target_init (BrkAnimationTarget *self)
{
}

void
brk_animation_target_set_value (BrkAnimationTarget *self,
                                double              value)
{
  g_return_if_fail (BRK_IS_ANIMATION_TARGET (self));

  BRK_ANIMATION_TARGET_GET_CLASS (self)->set_value (self, value);
}

struct _BrkCallbackAnimationTarget
{
  BrkAnimationTarget parent_instance;

  BrkAnimationTargetFunc callback;
  gpointer user_data;
  GDestroyNotify destroy_notify;
};

struct _BrkCallbackAnimationTargetClass
{
  BrkAnimationTargetClass parent_class;
};

G_DEFINE_FINAL_TYPE (BrkCallbackAnimationTarget, brk_callback_animation_target, BRK_TYPE_ANIMATION_TARGET)

static void
brk_callback_animation_target_set_value (BrkAnimationTarget *target,
                                         double              value)
{
  BrkCallbackAnimationTarget *self = BRK_CALLBACK_ANIMATION_TARGET (target);

  self->callback (value, self->user_data);
}

static void
brk_callback_animation_finalize (GObject *object)
{
  BrkCallbackAnimationTarget *self = BRK_CALLBACK_ANIMATION_TARGET (object);

  if (self->destroy_notify)
    self->destroy_notify (self->user_data);

  G_OBJECT_CLASS (brk_callback_animation_target_parent_class)->finalize (object);
}

static void
brk_callback_animation_target_class_init (BrkCallbackAnimationTargetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BrkAnimationTargetClass *target_class = BRK_ANIMATION_TARGET_CLASS (klass);

  object_class->finalize = brk_callback_animation_finalize;

  target_class->set_value = brk_callback_animation_target_set_value;
}

static void
brk_callback_animation_target_init (BrkCallbackAnimationTarget *self)
{
}

/**
 * brk_callback_animation_target_new:
 * @callback: (scope notified) (not nullable): the callback to call
 * @user_data: (closure callback): the data to be passed to @callback
 * @destroy: (destroy user_data): the function to be called when the
 *   callback action is finalized
 *
 * Creates a new `BrkAnimationTarget` that calls the given @callback during
 * the animation.
 *
 * Returns: the newly created callback target
 */
BrkAnimationTarget *
brk_callback_animation_target_new (BrkAnimationTargetFunc callback,
                                   gpointer               user_data,
                                   GDestroyNotify         destroy)
{
  BrkCallbackAnimationTarget *self;

  g_return_val_if_fail (callback != NULL, NULL);

  self = g_object_new (BRK_TYPE_CALLBACK_ANIMATION_TARGET, NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->destroy_notify = destroy;

  return BRK_ANIMATION_TARGET (self);
}

struct _BrkPropertyAnimationTarget
{
  BrkAnimationTarget parent_instance;

  GObject *object;
  GParamSpec *pspec;
};

struct _BrkPropertyAnimationTargetClass
{
  BrkAnimationTargetClass parent_class;
};

G_DEFINE_FINAL_TYPE (BrkPropertyAnimationTarget, brk_property_animation_target, BRK_TYPE_ANIMATION_TARGET)

enum {
  PROPERTY_PROP_0,
  PROPERTY_PROP_OBJECT,
  PROPERTY_PROP_PSPEC,
  LAST_PROPERTY_PROP
};

static GParamSpec *property_props[LAST_PROPERTY_PROP];

static void
object_weak_notify (gpointer  data,
                    GObject  *object)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (data);
  self->object = NULL;
}

static void
set_object (BrkPropertyAnimationTarget *self,
            GObject                    *object)
{
  if (self->object)
    g_object_weak_unref (self->object, object_weak_notify, self);
  self->object = object;
  g_object_weak_ref (self->object, object_weak_notify, self);
}

static void
brk_property_animation_target_set_value (BrkAnimationTarget *target,
                                         double              value)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (target);
  GValue gvalue = G_VALUE_INIT;

  if (!self->object || !self->pspec)
    return;

  g_value_init (&gvalue, G_TYPE_DOUBLE);
  g_value_set_double (&gvalue, value);
  g_object_set_property (self->object, self->pspec->name, &gvalue);
}

static void
brk_property_animation_target_constructed (GObject *object)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (object);

  G_OBJECT_CLASS (brk_property_animation_target_parent_class)->constructed (object);

  if (!self->object)
    g_error ("BrkPropertyAnimationTarget constructed without specifying a value "
             "for the 'object' property");

  if (!self->pspec)
    g_error ("BrkPropertyAnimationTarget constructed without specifying a value "
             "for the 'pspec' property");

  if (!g_type_is_a (G_OBJECT_TYPE (self->object), self->pspec->owner_type))
    g_error ("Cannot create BrkPropertyAnimationTarget: %s doesn't have the "
             "%s:%s property",
             G_OBJECT_TYPE_NAME (self->object),
             g_type_name (self->pspec->owner_type),
             self->pspec->name);
}

static void
brk_property_animation_target_dispose (GObject *object)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (object);

  if (self->object)
    g_object_weak_unref (self->object, object_weak_notify, self);
  self->object = NULL;

  G_OBJECT_CLASS (brk_property_animation_target_parent_class)->dispose (object);
}

static void
brk_property_animation_target_finalize (GObject *object)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (object);

  g_clear_pointer (&self->pspec, g_param_spec_unref);

  G_OBJECT_CLASS (brk_property_animation_target_parent_class)->finalize (object);
}

static void
brk_property_animation_target_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (object);

  switch (prop_id) {
  case PROPERTY_PROP_OBJECT:
    g_value_set_object (value, brk_property_animation_target_get_object (self));
    break;

  case PROPERTY_PROP_PSPEC:
    g_value_set_param (value, brk_property_animation_target_get_pspec (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_property_animation_target_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  BrkPropertyAnimationTarget *self = BRK_PROPERTY_ANIMATION_TARGET (object);

  switch (prop_id) {
  case PROPERTY_PROP_OBJECT:
    set_object (self, g_value_get_object (value));
    break;

  case PROPERTY_PROP_PSPEC:
    g_clear_pointer (&self->pspec, g_param_spec_unref);
    self->pspec = g_param_spec_ref (g_value_get_param (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
brk_property_animation_target_class_init (BrkPropertyAnimationTargetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  BrkAnimationTargetClass *target_class = BRK_ANIMATION_TARGET_CLASS (klass);

  object_class->constructed = brk_property_animation_target_constructed;
  object_class->dispose = brk_property_animation_target_dispose;
  object_class->finalize = brk_property_animation_target_finalize;
  object_class->get_property = brk_property_animation_target_get_property;
  object_class->set_property = brk_property_animation_target_set_property;

  target_class->set_value = brk_property_animation_target_set_value;

  /**
   * BrkPropertyAnimationTarget:object: (attributes org.gtk.Property.get=brk_property_animation_target_get_object)
   *
   * The object whose property will be animated.
   *
   * The `BrkPropertyAnimationTarget` instance does not hold a strong reference
   * on the object; make sure the object is kept alive throughout the target's
   * lifetime.
   *
   * Since: 1.2
   */
  property_props[PROPERTY_PROP_OBJECT] =
    g_param_spec_object ("object", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * BrkPropertyAnimationTarget:pspec: (attributes org.gtk.Property.get=brk_property_animation_target_get_pspec)
   *
   * The `GParamSpec` of the property to be animated.
   *
   * Since: 1.2
   */
  property_props[PROPERTY_PROP_PSPEC] =
    g_param_spec_param ("pspec", NULL, NULL,
                        G_TYPE_PARAM,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     LAST_PROPERTY_PROP,
                                     property_props);
}

static void
brk_property_animation_target_init (BrkPropertyAnimationTarget *self)
{
}

/**
 * brk_property_animation_target_new:
 * @object: an object to be animated
 * @property_name: the name of the property on @object to animate
 *
 * Creates a new `BrkPropertyAnimationTarget` for the @property_name property on
 * @object.
 *
 * Returns: the newly created `BrkPropertyAnimationTarget`
 *
 * Since: 1.2
 */
BrkAnimationTarget *
brk_property_animation_target_new (GObject    *object,
                                   const char *property_name)
{
  GParamSpec *pspec;

  g_return_val_if_fail (G_IS_OBJECT (object), NULL);
  g_return_val_if_fail (property_name != NULL, NULL);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), property_name);

  if (!pspec)
    g_error ("Type '%s' does not have a property named '%s'",
             G_OBJECT_TYPE_NAME (object), property_name);

  return brk_property_animation_target_new_for_pspec (object, pspec);
}

/**
 * brk_property_animation_target_new_for_pspec:
 * @object: an object to be animated
 * @pspec: the param spec of the property on @object to animate
 *
 * Creates a new `BrkPropertyAnimationTarget` for the @pspec property on
 * @object.
 *
 * Returns: new newly created `BrkPropertyAnimationTarget`
 *
 * Since: 1.2
 */
BrkAnimationTarget *
brk_property_animation_target_new_for_pspec (GObject    *object,
                                             GParamSpec *pspec)
{
  g_return_val_if_fail (G_IS_OBJECT (object), NULL);
  g_return_val_if_fail (G_IS_PARAM_SPEC (pspec), NULL);

  return g_object_new (BRK_TYPE_PROPERTY_ANIMATION_TARGET,
                       "object", object,
                       "pspec", pspec,
                       NULL);
}

/**
 * brk_property_animation_target_get_object: (attributes org.gtk.Method.get_property=object)
 * @self: a property animation target
 *
 * Gets the object animated by @self.
 *
 * The `BrkPropertyAnimationTarget` instance does not hold a strong reference on
 * the object; make sure the object is kept alive throughout the target's
 * lifetime.
 *
 * Returns: (transfer none): the animated object
 *
 * Since: 1.2
 */
GObject *
brk_property_animation_target_get_object (BrkPropertyAnimationTarget *self)
{
  g_return_val_if_fail (BRK_IS_PROPERTY_ANIMATION_TARGET (self), NULL);

  return self->object;
}

/**
 * brk_property_animation_target_get_pspec: (attributes org.gtk.Method.get_property=pspec)
 * @self: a property animation target
 *
 * Gets the `GParamSpec` of the property animated by @self.
 *
 * Returns: (transfer none): the animated property's `GParamSpec`
 *
 * Since: 1.2
 */
GParamSpec *
brk_property_animation_target_get_pspec (BrkPropertyAnimationTarget *self)
{
  g_return_val_if_fail (BRK_IS_PROPERTY_ANIMATION_TARGET (self), NULL);

  return self->pspec;
}
