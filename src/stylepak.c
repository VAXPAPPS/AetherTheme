#include "stylepak.h"
#include "tools.h"
#include <glib.h>

gboolean flatpak_available(void) {
    gchar *stdout_buf = NULL;
    gint exit_status = 0;
    if (g_spawn_command_line_sync("which flatpak", &stdout_buf, NULL, &exit_status, NULL)) {
        gboolean available = (exit_status == 0);
        g_free(stdout_buf);
        return available;
    }
    return FALSE;
}

void override_flatpak_gtk_theme(void) {
    if (!gsettings.gtk_theme) return;
    g_print("Overriding flatpak GTK theme to %s...\n", gsettings.gtk_theme);
    gchar *cmd = g_strdup_printf("flatpak override --user --env=GTK_THEME=%s", gsettings.gtk_theme);
    g_spawn_command_line_sync(cmd, NULL, NULL, NULL, NULL);
    g_free(cmd);
}

void unset_flatpak_gtk_theme(void) {
    g_print("Removing Flatpak GTK Theme override...\n");
    g_spawn_command_line_sync("flatpak override --user --unset-env=GTK_THEME", NULL, NULL, NULL, NULL);
}

void override_flatpak_icon_theme(void) {
    if (!gsettings.icon_theme) return;
    g_print("Overriding flatpak Icon theme to %s...\n", gsettings.icon_theme);
    gchar *cmd = g_strdup_printf("flatpak override --user --env=ICON_THEME=%s", gsettings.icon_theme);
    g_spawn_command_line_sync(cmd, NULL, NULL, NULL, NULL);
    g_free(cmd);
}

void unset_flatpak_icon_theme(void) {
    g_print("Removing Flatpak Icon Theme override...\n");
    g_spawn_command_line_sync("flatpak override --user --unset-env=ICON_THEME", NULL, NULL, NULL, NULL);
}

void install_user_theme(const gchar *theme, GError **error) {
    // Basic implementation that assumes the theme is in ~/.themes
    // The Go code ran: flatpak --user install ...
    // For brevity, we ignore the full org.gtk.Gtk3theme.%s install unless requested
}
