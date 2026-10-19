/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2024 EarPort Contributors
 */

#include "link_watch.h"

#include <gio/gio.h>

struct LinkWatch {
    GDBusConnection *bus;
    char *device_path;
    guint disconnected_id;
    LinkWatchClosedCallback closed_callback;
    void *closed_user_data;
};

/* org.bluez.Device1.Disconnected(name, message), BlueZ 5.73 and later */
static void on_disconnected(GDBusConnection *bus, const gchar *sender, const gchar *path,
                            const gchar *interface, const gchar *signal, GVariant *parameters,
                            gpointer user_data)
{
    (void)bus;
    (void)sender;
    (void)interface;
    (void)signal;
    LinkWatch *watch = user_data;
    if (g_strcmp0(path, watch->device_path) != 0)
        return;

    const gchar *name = NULL, *message = NULL;
    g_variant_get(parameters, "(&s&s)", &name, &message);
    g_message("BlueZ: AirPods link closed: %s (%s)", message, name);
    if (watch->closed_callback != NULL)
        watch->closed_callback(g_strcmp0(name, "org.bluez.Reason.Remote") == 0 ||
                                   g_strcmp0(name, "org.bluez.Reason.Timeout") == 0,
                               watch->closed_user_data);
}

LinkWatch *link_watch_new(void)
{
    GDBusConnection *bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if (bus == NULL)
        return NULL;

    LinkWatch *watch = g_new0(LinkWatch, 1);
    watch->bus = bus;
    watch->disconnected_id = g_dbus_connection_signal_subscribe(
        bus, "org.bluez", "org.bluez.Device1", "Disconnected", NULL, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, on_disconnected, watch, NULL);
    return watch;
}

void link_watch_free(LinkWatch *watch)
{
    if (watch == NULL)
        return;
    g_dbus_connection_signal_unsubscribe(watch->bus, watch->disconnected_id);
    g_object_unref(watch->bus);
    g_free(watch->device_path);
    g_free(watch);
}

void link_watch_set_device(LinkWatch *watch, const char *device_path)
{
    if (watch == NULL)
        return;
    g_free(watch->device_path);
    watch->device_path = g_strdup(device_path);
}

void link_watch_set_closed_callback(LinkWatch *watch, LinkWatchClosedCallback callback,
                                    void *user_data)
{
    if (watch == NULL)
        return;
    watch->closed_callback = callback;
    watch->closed_user_data = user_data;
}
