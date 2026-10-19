/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2024 EarPort Contributors
 *
 * Tells who closed the AirPods' link, from BlueZ's reason: they closed it
 * or stopped answering, or this computer closed it.
 */

#ifndef LINK_WATCH_H
#define LINK_WATCH_H

#include <stdbool.h>

typedef struct LinkWatch LinkWatch;

/* The link to the AirPods closed; left: they closed it or stopped
 * answering (they do both when another device takes them), rather than
 * this computer closing it */
typedef void (*LinkWatchClosedCallback)(bool left, void *user_data);

LinkWatch *link_watch_new(void);
void link_watch_free(LinkWatch *watch);

/* The AirPods followed: their BlueZ object path, NULL for none */
void link_watch_set_device(LinkWatch *watch, const char *device_path);

/* Needs BlueZ 5.73 or later, which tells why a link closed */
void link_watch_set_closed_callback(LinkWatch *watch, LinkWatchClosedCallback callback,
                                    void *user_data);

#endif /* LINK_WATCH_H */
