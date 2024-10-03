#pragma once

#include "hyprland/backend.h"

#define UPDATE_ALL_WORKSPACES -1

void updateWorkspace (int workspace_id, HyprBackend *self);
void onWorkspaceActivated (char *payload, HyprBackend *self);
void onSpecialWorkspaceActivated (char *payload, HyprBackend *self);
void onWorkspaceDestroyed (char *payload, HyprBackend *self);
void onWorkspaceCreated (char *payload, HyprBackend *self);
void onMonitorFocused (char *payload, HyprBackend *self);
void onWorkspaceMoved (char *payload, HyprBackend *self);
void onWindowOpened (char *payload, HyprBackend *self);
void onWindowClosed (char *payload, HyprBackend *self);
void onWindowMoved (char *payload, HyprBackend *self);
void setUrgentWorkspace (char *payload, HyprBackend *self);
void onWorkspaceRenamed (char *payload, HyprBackend *self);
void onWindowTitleEvent (char *payload, HyprBackend *self);
void onConfigReloaded (HyprBackend *self);