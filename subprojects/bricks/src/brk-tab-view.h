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

G_BEGIN_DECLS

typedef enum /*< flags >*/ {
  BRK_TAB_VIEW_SHORTCUT_NONE                    = 0,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_TAB             = 1 << 0,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_TAB       = 1 << 1,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_PAGE_UP         = 1 << 2,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_PAGE_DOWN       = 1 << 3,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_HOME            = 1 << 4,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_END             = 1 << 5,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_UP   = 1 << 6,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_PAGE_DOWN = 1 << 7,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_HOME      = 1 << 8,
  BRK_TAB_VIEW_SHORTCUT_CONTROL_SHIFT_END       = 1 << 9,
  BRK_TAB_VIEW_SHORTCUT_ALT_DIGITS              = 1 << 10,
  BRK_TAB_VIEW_SHORTCUT_ALT_ZERO                = 1 << 11,
  BRK_TAB_VIEW_SHORTCUT_ALL_SHORTCUTS           = 0xFFF
} BrkTabViewShortcuts;

#define BRK_TYPE_TAB_PAGE (brk_tab_page_get_type())

BRK_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (BrkTabPage, brk_tab_page, BRK, TAB_PAGE, GObject)

BRK_AVAILABLE_IN_ALL
GtkWidget *brk_tab_page_get_child (BrkTabPage *self);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_page_get_parent (BrkTabPage *self);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_page_get_selected (BrkTabPage *self);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_page_get_pinned (BrkTabPage *self);

BRK_AVAILABLE_IN_ALL
const char *brk_tab_page_get_title (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void        brk_tab_page_set_title (BrkTabPage *self,
                                    const char *title);

BRK_AVAILABLE_IN_ALL
const char *brk_tab_page_get_tooltip (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void        brk_tab_page_set_tooltip (BrkTabPage *self,
                                      const char *tooltip);

BRK_AVAILABLE_IN_ALL
GIcon *brk_tab_page_get_icon (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void   brk_tab_page_set_icon (BrkTabPage *self,
                              GIcon      *icon);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_page_get_loading (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void     brk_tab_page_set_loading (BrkTabPage *self,
                                   gboolean    loading);

BRK_AVAILABLE_IN_ALL
GIcon *brk_tab_page_get_indicator_icon (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void   brk_tab_page_set_indicator_icon (BrkTabPage *self,
                                        GIcon      *indicator_icon);

BRK_AVAILABLE_IN_1_2
const char *brk_tab_page_get_indicator_tooltip (BrkTabPage *self);
BRK_AVAILABLE_IN_1_2
void        brk_tab_page_set_indicator_tooltip (BrkTabPage *self,
                                                const char *tooltip);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_page_get_indicator_activatable (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void     brk_tab_page_set_indicator_activatable (BrkTabPage *self,
                                                 gboolean    activatable);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_page_get_needs_attention (BrkTabPage *self);
BRK_AVAILABLE_IN_ALL
void     brk_tab_page_set_needs_attention (BrkTabPage *self,
                                           gboolean    needs_attention);

BRK_AVAILABLE_IN_1_3
const char *brk_tab_page_get_keyword (BrkTabPage *self);
BRK_AVAILABLE_IN_1_3
void        brk_tab_page_set_keyword (BrkTabPage *self,
                                      const char *keyword);

BRK_AVAILABLE_IN_1_3
float brk_tab_page_get_thumbnail_xalign (BrkTabPage *self);
BRK_AVAILABLE_IN_1_3
void  brk_tab_page_set_thumbnail_xalign (BrkTabPage *self,
                                         float       xalign);

BRK_AVAILABLE_IN_1_3
float brk_tab_page_get_thumbnail_yalign (BrkTabPage *self);
BRK_AVAILABLE_IN_1_3
void  brk_tab_page_set_thumbnail_yalign (BrkTabPage *self,
                                         float       yalign);

BRK_AVAILABLE_IN_1_3
gboolean brk_tab_page_get_live_thumbnail (BrkTabPage *self);
BRK_AVAILABLE_IN_1_3
void     brk_tab_page_set_live_thumbnail (BrkTabPage *self,
                                          gboolean    live_thumbnail);

BRK_AVAILABLE_IN_1_3
void brk_tab_page_invalidate_thumbnail (BrkTabPage *self);

#define BRK_TYPE_TAB_VIEW (brk_tab_view_get_type())

BRK_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (BrkTabView, brk_tab_view, BRK, TAB_VIEW, GtkWidget)

BRK_AVAILABLE_IN_ALL
BrkTabView *brk_tab_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_ALL
int brk_tab_view_get_n_pages        (BrkTabView *self);
BRK_AVAILABLE_IN_ALL
int brk_tab_view_get_n_pinned_pages (BrkTabView *self);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_get_is_transferring_page (BrkTabView *self);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_get_selected_page (BrkTabView *self);
BRK_AVAILABLE_IN_ALL
void        brk_tab_view_set_selected_page (BrkTabView *self,
                                            BrkTabPage *selected_page);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_select_previous_page (BrkTabView *self);
BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_select_next_page     (BrkTabView *self);

BRK_AVAILABLE_IN_ALL
GIcon *brk_tab_view_get_default_icon (BrkTabView *self);
BRK_AVAILABLE_IN_ALL
void   brk_tab_view_set_default_icon (BrkTabView *self,
                                      GIcon      *default_icon);

BRK_AVAILABLE_IN_ALL
GMenuModel *brk_tab_view_get_menu_model (BrkTabView *self);
BRK_AVAILABLE_IN_ALL
void        brk_tab_view_set_menu_model (BrkTabView *self,
                                         GMenuModel *menu_model);

BRK_AVAILABLE_IN_1_2
BrkTabViewShortcuts brk_tab_view_get_shortcuts    (BrkTabView          *self);
BRK_AVAILABLE_IN_1_2
void                brk_tab_view_set_shortcuts    (BrkTabView          *self,
                                                   BrkTabViewShortcuts  shortcuts);
BRK_AVAILABLE_IN_1_2
void                brk_tab_view_add_shortcuts    (BrkTabView          *self,
                                                   BrkTabViewShortcuts  shortcuts);
BRK_AVAILABLE_IN_1_2
void                brk_tab_view_remove_shortcuts (BrkTabView          *self,
                                                   BrkTabViewShortcuts  shortcuts);

BRK_AVAILABLE_IN_ALL
void brk_tab_view_set_page_pinned (BrkTabView *self,
                                   BrkTabPage *page,
                                   gboolean    pinned);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_get_page (BrkTabView *self,
                                   GtkWidget  *child);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_get_nth_page (BrkTabView *self,
                                       int         position);

BRK_AVAILABLE_IN_ALL
int brk_tab_view_get_page_position (BrkTabView *self,
                                    BrkTabPage *page);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_add_page (BrkTabView *self,
                                   GtkWidget  *child,
                                   BrkTabPage *parent);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_insert  (BrkTabView *self,
                                  GtkWidget  *child,
                                  int         position);
BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_prepend (BrkTabView *self,
                                  GtkWidget  *child);
BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_append  (BrkTabView *self,
                                  GtkWidget  *child);

BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_insert_pinned  (BrkTabView *self,
                                         GtkWidget  *child,
                                         int         position);
BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_prepend_pinned (BrkTabView *self,
                                         GtkWidget  *child);
BRK_AVAILABLE_IN_ALL
BrkTabPage *brk_tab_view_append_pinned  (BrkTabView *self,
                                         GtkWidget  *child);

BRK_AVAILABLE_IN_ALL
void brk_tab_view_close_page        (BrkTabView *self,
                                     BrkTabPage *page);
BRK_AVAILABLE_IN_ALL
void brk_tab_view_close_page_finish (BrkTabView *self,
                                     BrkTabPage *page,
                                     gboolean    confirm);

BRK_AVAILABLE_IN_ALL
void brk_tab_view_close_other_pages  (BrkTabView *self,
                                      BrkTabPage *page);
BRK_AVAILABLE_IN_ALL
void brk_tab_view_close_pages_before (BrkTabView *self,
                                      BrkTabPage *page);
BRK_AVAILABLE_IN_ALL
void brk_tab_view_close_pages_after  (BrkTabView *self,
                                      BrkTabPage *page);

BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_reorder_page     (BrkTabView *self,
                                        BrkTabPage *page,
                                        int         position);
BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_reorder_backward (BrkTabView *self,
                                        BrkTabPage *page);
BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_reorder_forward  (BrkTabView *self,
                                        BrkTabPage *page);
BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_reorder_first    (BrkTabView *self,
                                        BrkTabPage *page);
BRK_AVAILABLE_IN_ALL
gboolean brk_tab_view_reorder_last     (BrkTabView *self,
                                        BrkTabPage *page);

BRK_AVAILABLE_IN_ALL
void brk_tab_view_transfer_page (BrkTabView *self,
                                 BrkTabPage *page,
                                 BrkTabView *other_view,
                                 int         position);

BRK_AVAILABLE_IN_ALL
GtkSelectionModel *brk_tab_view_get_pages (BrkTabView *self) G_GNUC_WARN_UNUSED_RESULT;

BRK_AVAILABLE_IN_1_3
void brk_tab_view_invalidate_thumbnails (BrkTabView *self);

G_END_DECLS
