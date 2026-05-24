#ifndef TOOLS_H
#define TOOLS_H

#include <glib.h>
#include <stdbool.h>

typedef struct {
    gboolean export_settings_ini;
    gboolean export_gtkrc_20;
    gboolean export_index_theme;
    gboolean export_xsettingsd;
    gboolean export_gtk4_symlinks;
    gboolean flatpak_export_gtk_theme_override;
    gboolean flatpak_export_icon_theme_override;
    gboolean flatpak_install_current_gtk_theme;
} ProgramSettings;

typedef struct {
    gchar *theme_name;
    gchar *icon_theme_name;
    gchar *font_name;
    gchar *cursor_theme_name;
    int cursor_theme_size;
    gchar *toolbar_style;
    gchar *toolbar_icon_size;
    gboolean button_images;
    gboolean menu_images;
    gboolean enable_event_sounds;
    gboolean enable_input_feedback_sounds;
    int xft_antialias;
    gchar *font_antialiasing;
    int xft_dpi;
    int xft_hinting;
    gchar *xft_hintstyle;
    gchar *xft_rgba;
    gboolean application_prefer_dark_theme;
} GtkConfigProperties;

typedef struct {
    gchar *gtk_theme;
    gchar *icon_theme;
    gchar *font_name;
    gchar *cursor_theme;
    int cursor_size;
    gchar *toolbar_style;
    gchar *toolbar_icons_size;
    gchar *font_hinting;
    gchar *font_antialiasing;
    gchar *font_rgba_order;
    double text_scaling_factor;
    gchar *color_scheme;
    gboolean event_sounds;
    gboolean input_feedback_sounds;
} GsettingsValues;

extern ProgramSettings preferences;
extern GtkConfigProperties gtk_config;
extern GsettingsValues gsettings;

extern gchar **data_dirs;
extern GHashTable *gtk_theme_paths;
extern GHashTable *icon_theme_names_map; // displayName -> folderName
extern GHashTable *cursor_theme_names_map; // displayName -> folderName
extern GHashTable *cursor_theme_paths;

void load_preferences(void);
void save_preferences(void);
void load_gtk_config(void);
void read_gsettings(void);
void apply_gsettings(void);
void save_gsettings_backup(void);
void apply_gsettings_from_file(void);
void save_gtk_ini_3(void);
void save_gtk_rc_20(void);
void save_index_theme(void);
void save_xsettingsd(void);
void link_gtk4_stuff(void);
void clear_gtk4_symlinks(void);
void save_gtk_ini_4(void);

void get_data_dirs(void);
GList* get_theme_names(void);
GList* get_icon_theme_names(void);
GList* get_cursor_themes(void);

gchar* config_home(void);
gchar* data_home(void);
gboolean path_exists(const gchar *path);
void make_dir(const gchar *path);

#endif
