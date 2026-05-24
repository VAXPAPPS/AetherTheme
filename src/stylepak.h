#ifndef STYLEPAK_H
#define STYLEPAK_H

#include <glib.h>

gboolean flatpak_available(void);
void override_flatpak_gtk_theme(void);
void unset_flatpak_gtk_theme(void);
void override_flatpak_icon_theme(void);
void unset_flatpak_icon_theme(void);
void install_user_theme(const gchar *theme, GError **error);

#endif
