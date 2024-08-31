/* panel-applications-menu.h
 *
 * Copyright 2023 Benjamin Montgomery
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gtk/gtk.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#include <gtk4-layer-shell.h>

#include "../taskbar/panel-taskbar-icon.h"

#define UNUSED(x) (void)(x)

G_BEGIN_DECLS

#define PANEL_TYPE_APPLICATIONS_MENU (panel_applications_menu_get_type())

G_DECLARE_FINAL_TYPE (PanelApplicationsMenu, panel_applications_menu, PANEL, APPLICATIONS_MENU, GtkApplicationWindow)

G_END_DECLS

void panel_applications_menu_set_bg (PanelApplicationsMenu *self, GdkPixbuf *bg);
void hide_applications_menu (PanelApplicationsMenu *self);
void show_applications_menu (PanelApplicationsMenu *self);

GtkWidget *panel_applications_menu_get_launcher_button (PanelApplicationsMenu *self);