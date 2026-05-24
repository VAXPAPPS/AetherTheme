#include "tools.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <unistd.h>

ProgramSettings preferences;
GtkConfigProperties gtk_config;
GsettingsValues gsettings;

gchar **data_dirs = NULL;
GHashTable *gtk_theme_paths = NULL;
GHashTable *icon_theme_names_map = NULL;
GHashTable *cursor_theme_names_map = NULL;
GHashTable *cursor_theme_paths = NULL;

gchar* config_home(void) {
    const gchar *c_home = g_getenv("XDG_CONFIG_HOME");
    if (c_home && *c_home) {
        return g_strdup(c_home);
    }
    return g_build_filename(g_get_home_dir(), ".config", NULL);
}

gchar* data_home(void) {
    const gchar *d_home = g_getenv("XDG_DATA_HOME");
    if (d_home && *d_home) {
        return g_strdup(d_home);
    }
    return g_build_filename(g_get_home_dir(), ".local", "share", NULL);
}

gboolean path_exists(const gchar *path) {
    return g_file_test(path, G_FILE_TEST_EXISTS);
}

void make_dir(const gchar *path) {
    g_mkdir_with_parents(path, 0755);
}

static void program_settings_new_with_defaults(void) {
    preferences.export_settings_ini = TRUE;
    preferences.export_gtkrc_20 = TRUE;
    preferences.export_index_theme = TRUE;
    preferences.export_xsettingsd = TRUE;
    preferences.export_gtk4_symlinks = TRUE;
    preferences.flatpak_export_gtk_theme_override = FALSE;
    preferences.flatpak_export_icon_theme_override = FALSE;
    preferences.flatpak_install_current_gtk_theme = FALSE;
}

void load_preferences(void) {
    gchar *c_home = config_home();
    gchar *preferences_dir = g_build_filename(c_home, "AetherTheme", NULL);
    gchar *preferences_file = g_build_filename(preferences_dir, "config", NULL);

    if (!path_exists(preferences_file)) {
        g_print("%s file not found, creating\n", preferences_file);
        make_dir(preferences_dir);
        program_settings_new_with_defaults();
        save_preferences();
    } else {
        g_print(">>> Loading preferences\n");
        JsonParser *parser = json_parser_new();
        GError *error = NULL;
        if (json_parser_load_from_file(parser, preferences_file, &error)) {
            JsonNode *root = json_parser_get_root(parser);
            if (JSON_NODE_HOLDS_OBJECT(root)) {
                JsonObject *obj = json_node_get_object(root);
                preferences.export_settings_ini = json_object_get_boolean_member_with_default(obj, "export-settings-ini", TRUE);
                preferences.export_gtkrc_20 = json_object_get_boolean_member_with_default(obj, "export-gtkrc-20", TRUE);
                preferences.export_index_theme = json_object_get_boolean_member_with_default(obj, "export-index-theme", TRUE);
                preferences.export_xsettingsd = json_object_get_boolean_member_with_default(obj, "export-xsettingsd", TRUE);
                preferences.export_gtk4_symlinks = json_object_get_boolean_member_with_default(obj, "export-gtk4-symlinks", TRUE);
                preferences.flatpak_export_gtk_theme_override = json_object_get_boolean_member_with_default(obj, "flatpak-export-gtk-theme-override", FALSE);
                preferences.flatpak_export_icon_theme_override = json_object_get_boolean_member_with_default(obj, "flatpak-export-icon-theme-override", FALSE);
                preferences.flatpak_install_current_gtk_theme = json_object_get_boolean_member_with_default(obj, "flatpak-install-current-gtk-theme", FALSE);
            }
        } else {
            g_printerr("Error loading preferences: %s\n", error->message);
            g_clear_error(&error);
            program_settings_new_with_defaults();
        }
        g_object_unref(parser);
    }
    
    g_free(preferences_file);
    g_free(preferences_dir);
    g_free(c_home);
}

void save_preferences(void) {
    gchar *c_home = config_home();
    gchar *preferences_file = g_build_filename(c_home, "AetherTheme", "config", NULL);

    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    
    json_builder_set_member_name(builder, "export-settings-ini");
    json_builder_add_boolean_value(builder, preferences.export_settings_ini);
    
    json_builder_set_member_name(builder, "export-gtkrc-20");
    json_builder_add_boolean_value(builder, preferences.export_gtkrc_20);

    json_builder_set_member_name(builder, "export-index-theme");
    json_builder_add_boolean_value(builder, preferences.export_index_theme);

    json_builder_set_member_name(builder, "export-xsettingsd");
    json_builder_add_boolean_value(builder, preferences.export_xsettingsd);

    json_builder_set_member_name(builder, "export-gtk4-symlinks");
    json_builder_add_boolean_value(builder, preferences.export_gtk4_symlinks);

    json_builder_set_member_name(builder, "flatpak-export-gtk-theme-override");
    json_builder_add_boolean_value(builder, preferences.flatpak_export_gtk_theme_override);

    json_builder_set_member_name(builder, "flatpak-export-icon-theme-override");
    json_builder_add_boolean_value(builder, preferences.flatpak_export_icon_theme_override);

    json_builder_set_member_name(builder, "flatpak-install-current-gtk-theme");
    json_builder_add_boolean_value(builder, preferences.flatpak_install_current_gtk_theme);

    json_builder_end_object(builder);

    JsonNode *root = json_builder_get_root(builder);
    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, root);
    json_generator_set_pretty(gen, TRUE);
    
    GError *error = NULL;
    json_generator_to_file(gen, preferences_file, &error);
    if (error) {
        g_printerr("Failed to save config: %s\n", error->message);
        g_clear_error(&error);
    }
    
    g_object_unref(gen);
    json_node_free(root);
    g_object_unref(builder);

    g_free(preferences_file);
    g_free(c_home);
}

void init_gsettings_defaults(void) {
    gsettings.gtk_theme = g_strdup("Adwaita");
    gsettings.icon_theme = g_strdup("Adwaita");
    gsettings.font_name = g_strdup("Sans 10");
    gsettings.cursor_theme = g_strdup("Adwaita");
    gsettings.cursor_size = 24;
    gsettings.toolbar_style = g_strdup("both-horiz");
    gsettings.toolbar_icons_size = g_strdup("large");
    gsettings.font_hinting = g_strdup("medium");
    gsettings.font_antialiasing = g_strdup("grayscale");
    gsettings.font_rgba_order = g_strdup("rgb");
    gsettings.text_scaling_factor = 1.0;
    gsettings.event_sounds = TRUE;
    gsettings.input_feedback_sounds = FALSE;
    gsettings.color_scheme = g_strdup("default");
    
    gtk_config.theme_name = g_strdup("Adwaita");
    gtk_config.icon_theme_name = g_strdup("Adwaita");
    gtk_config.font_name = g_strdup("Sans 10");
    gtk_config.cursor_theme_name = g_strdup("");
    gtk_config.cursor_theme_size = 0;
    gtk_config.toolbar_style = g_strdup("GTK_TOOLBAR_ICONS");
    gtk_config.toolbar_icon_size = g_strdup("GTK_ICON_SIZE_LARGE_TOOLBAR");
    gtk_config.button_images = FALSE;
    gtk_config.menu_images = FALSE;
    gtk_config.enable_event_sounds = TRUE;
    gtk_config.enable_input_feedback_sounds = TRUE;
    gtk_config.xft_antialias = -1;
    gtk_config.application_prefer_dark_theme = FALSE;
    gtk_config.font_antialiasing = g_strdup("grayscale");
    gtk_config.xft_hinting = -1;
    gtk_config.xft_hintstyle = g_strdup("hintmedium");
    gtk_config.xft_rgba = g_strdup("none");
}

void load_gtk_config(void) {}
void read_gsettings(void) {
    g_print(">>> Reading gsettings\n");
    init_gsettings_defaults();
    
    GSettings *interface_settings = g_settings_new("org.gnome.desktop.interface");
    GSettings *sound_settings = g_settings_new("org.gnome.desktop.sound");

    gsettings.gtk_theme = g_settings_get_string(interface_settings, "gtk-theme");
    gsettings.icon_theme = g_settings_get_string(interface_settings, "icon-theme");
    gsettings.font_name = g_settings_get_string(interface_settings, "font-name");
    gsettings.cursor_theme = g_settings_get_string(interface_settings, "cursor-theme");
    gsettings.cursor_size = g_settings_get_int(interface_settings, "cursor-size");
    gsettings.toolbar_style = g_settings_get_string(interface_settings, "toolbar-style");
    gsettings.toolbar_icons_size = g_settings_get_string(interface_settings, "toolbar-icons-size");
    gsettings.font_hinting = g_settings_get_string(interface_settings, "font-hinting");
    gsettings.font_antialiasing = g_settings_get_string(interface_settings, "font-antialiasing");
    gsettings.font_rgba_order = g_settings_get_string(interface_settings, "font-rgba-order");
    gsettings.text_scaling_factor = g_settings_get_double(interface_settings, "text-scaling-factor");
    gsettings.color_scheme = g_settings_get_string(interface_settings, "color-scheme");

    gsettings.event_sounds = g_settings_get_boolean(sound_settings, "event-sounds");
    gsettings.input_feedback_sounds = g_settings_get_boolean(sound_settings, "input-feedback-sounds");

    g_object_unref(interface_settings);
    g_object_unref(sound_settings);
}

void apply_gsettings(void) {
    g_print(">>> Applying gsettings\n");
    GSettings *interface_settings = g_settings_new("org.gnome.desktop.interface");
    GSettings *sound_settings = g_settings_new("org.gnome.desktop.sound");

    g_settings_set_string(interface_settings, "gtk-theme", gsettings.gtk_theme ? gsettings.gtk_theme : "");
    g_settings_set_string(interface_settings, "icon-theme", gsettings.icon_theme ? gsettings.icon_theme : "");
    g_settings_set_string(interface_settings, "font-name", gsettings.font_name ? gsettings.font_name : "");
    g_settings_set_string(interface_settings, "cursor-theme", gsettings.cursor_theme ? gsettings.cursor_theme : "");
    g_settings_set_int(interface_settings, "cursor-size", gsettings.cursor_size);
    g_settings_set_string(interface_settings, "toolbar-style", gsettings.toolbar_style ? gsettings.toolbar_style : "");
    g_settings_set_string(interface_settings, "toolbar-icons-size", gsettings.toolbar_icons_size ? gsettings.toolbar_icons_size : "");
    g_settings_set_string(interface_settings, "font-hinting", gsettings.font_hinting ? gsettings.font_hinting : "");
    g_settings_set_string(interface_settings, "font-antialiasing", gsettings.font_antialiasing ? gsettings.font_antialiasing : "");
    g_settings_set_string(interface_settings, "font-rgba-order", gsettings.font_rgba_order ? gsettings.font_rgba_order : "");
    g_settings_set_double(interface_settings, "text-scaling-factor", gsettings.text_scaling_factor);
    g_settings_set_string(interface_settings, "color-scheme", gsettings.color_scheme ? gsettings.color_scheme : "");

    g_settings_set_boolean(sound_settings, "event-sounds", gsettings.event_sounds);
    g_settings_set_boolean(sound_settings, "input-feedback-sounds", gsettings.input_feedback_sounds);

    g_settings_sync();

    g_object_unref(interface_settings);
    g_object_unref(sound_settings);
}
void save_gsettings_backup(void) {}
void apply_gsettings_from_file(void) {}
void save_gtk_ini_3(void) {
    gchar *config_dir = g_build_filename(config_home(), "gtk-3.0", NULL);
    make_dir(config_dir);
    gchar *config_file = g_build_filename(config_dir, "settings.ini", NULL);
    
    GString *content = g_string_new("[Settings]\n");
    g_string_append_printf(content, "gtk-theme-name=%s\n", gsettings.gtk_theme);
    g_string_append_printf(content, "gtk-icon-theme-name=%s\n", gsettings.icon_theme);
    g_string_append_printf(content, "gtk-font-name=%s\n", gsettings.font_name);
    g_string_append_printf(content, "gtk-cursor-theme-name=%s\n", gsettings.cursor_theme);
    g_string_append_printf(content, "gtk-cursor-theme-size=%d\n", gsettings.cursor_size);
    g_string_append_printf(content, "gtk-toolbar-style=%s\n", gtk_config.toolbar_style);
    g_string_append_printf(content, "gtk-toolbar-icon-size=%s\n", gtk_config.toolbar_icon_size);
    g_string_append_printf(content, "gtk-button-images=%d\n", gtk_config.button_images ? 1 : 0);
    g_string_append_printf(content, "gtk-menu-images=%d\n", gtk_config.menu_images ? 1 : 0);
    g_string_append_printf(content, "gtk-enable-event-sounds=%d\n", gsettings.event_sounds ? 1 : 0);
    g_string_append_printf(content, "gtk-enable-input-feedback-sounds=%d\n", gsettings.input_feedback_sounds ? 1 : 0);
    g_string_append_printf(content, "gtk-xft-antialias=%d\n", g_strcmp0(gsettings.font_antialiasing, "none") != 0 ? 1 : 0);
    g_string_append_printf(content, "gtk-xft-hinting=%d\n", g_strcmp0(gsettings.font_hinting, "none") != 0 ? 1 : 0);
    
    gchar *fh = "hintnone";
    if (g_strcmp0(gsettings.font_hinting, "slight") == 0) fh = "hintslight";
    else if (g_strcmp0(gsettings.font_hinting, "medium") == 0) fh = "hintmedium";
    else if (g_strcmp0(gsettings.font_hinting, "full") == 0) fh = "hintfull";
    g_string_append_printf(content, "gtk-xft-hintstyle=%s\n", fh);
    g_string_append_printf(content, "gtk-xft-rgba=%s\n", gsettings.font_rgba_order);
    g_string_append_printf(content, "gtk-application-prefer-dark-theme=%d\n", g_strcmp0(gsettings.color_scheme, "prefer-dark") == 0 ? 1 : 0);

    g_file_set_contents(config_file, content->str, -1, NULL);
    g_string_free(content, TRUE);
    g_free(config_file);
    g_free(config_dir);
}

void save_gtk_rc_20(void) {
    gchar *config_file = g_build_filename(g_get_home_dir(), ".gtkrc-2.0", NULL);
    
    GString *content = g_string_new("# DO NOT EDIT! This file will be overwritten by AetherTheme.\n");
    g_string_append(content, "# Any customization should be done in ~/.gtkrc-2.0.mine instead.\n\n");
    g_string_append_printf(content, "include \"%s/.gtkrc-2.0.mine\"\n", g_get_home_dir());
    g_string_append_printf(content, "gtk-theme-name=\"%s\"\n", gsettings.gtk_theme);
    g_string_append_printf(content, "gtk-icon-theme-name=\"%s\"\n", gsettings.icon_theme);
    g_string_append_printf(content, "gtk-font-name=\"%s\"\n", gsettings.font_name);
    g_string_append_printf(content, "gtk-cursor-theme-name=\"%s\"\n", gsettings.cursor_theme);
    g_string_append_printf(content, "gtk-cursor-theme-size=%d\n", gsettings.cursor_size);
    g_string_append_printf(content, "gtk-toolbar-style=%s\n", gtk_config.toolbar_style);
    g_string_append_printf(content, "gtk-toolbar-icon-size=%s\n", gtk_config.toolbar_icon_size);
    g_string_append_printf(content, "gtk-button-images=%d\n", gtk_config.button_images ? 1 : 0);
    g_string_append_printf(content, "gtk-menu-images=%d\n", gtk_config.menu_images ? 1 : 0);
    g_string_append_printf(content, "gtk-enable-event-sounds=%d\n", gsettings.event_sounds ? 1 : 0);
    g_string_append_printf(content, "gtk-enable-input-feedback-sounds=%d\n", gsettings.input_feedback_sounds ? 1 : 0);
    g_string_append_printf(content, "gtk-xft-antialias=%d\n", g_strcmp0(gsettings.font_antialiasing, "none") != 0 ? 1 : 0);
    g_string_append_printf(content, "gtk-xft-hinting=%d\n", g_strcmp0(gsettings.font_hinting, "none") != 0 ? 1 : 0);
    
    gchar *fh = "hintnone";
    if (g_strcmp0(gsettings.font_hinting, "slight") == 0) fh = "hintslight";
    else if (g_strcmp0(gsettings.font_hinting, "medium") == 0) fh = "hintmedium";
    else if (g_strcmp0(gsettings.font_hinting, "full") == 0) fh = "hintfull";
    g_string_append_printf(content, "gtk-xft-hintstyle=\"%s\"\n", fh);
    g_string_append_printf(content, "gtk-xft-rgba=\"%s\"\n", gsettings.font_rgba_order);

    g_file_set_contents(config_file, content->str, -1, NULL);
    g_string_free(content, TRUE);
    g_free(config_file);
}

void save_index_theme(void) {
    gchar *icons_dir = g_build_filename(g_get_home_dir(), ".icons", "default", NULL);
    make_dir(icons_dir);
    gchar *config_file = g_build_filename(icons_dir, "index.theme", NULL);
    
    GString *content = g_string_new("# This file is written by AetherTheme. Do not edit.\n");
    g_string_append(content, "[Icon Theme]\n");
    g_string_append_printf(content, "Name=Default\n");
    g_string_append_printf(content, "Comment=Default Cursor Theme\n");
    g_string_append_printf(content, "Inherits=%s\n", gsettings.cursor_theme);

    g_file_set_contents(config_file, content->str, -1, NULL);
    g_string_free(content, TRUE);
    g_free(config_file);
    g_free(icons_dir);
}

void save_xsettingsd(void) {
    gchar *config_dir = g_build_filename(config_home(), "xsettingsd", NULL);
    make_dir(config_dir);
    gchar *config_file = g_build_filename(config_dir, "xsettingsd.conf", NULL);
    
    GString *content = g_string_new("");
    g_string_append_printf(content, "Net/ThemeName \"%s\"\n", gsettings.gtk_theme);
    g_string_append_printf(content, "Net/IconThemeName \"%s\"\n", gsettings.icon_theme);
    g_string_append_printf(content, "Gtk/CursorThemeName \"%s\"\n", gsettings.cursor_theme);
    g_string_append_printf(content, "Net/EnableEventSounds %d\n", gsettings.event_sounds ? 1 : 0);
    g_string_append_printf(content, "EnableInputFeedbackSounds %d\n", gsettings.input_feedback_sounds ? 1 : 0);
    g_string_append_printf(content, "Xft/Antialias %d\n", g_strcmp0(gsettings.font_antialiasing, "none") != 0 ? 1 : 0);
    g_string_append_printf(content, "Xft/Hinting %d\n", g_strcmp0(gsettings.font_hinting, "none") != 0 ? 1 : 0);
    
    gchar *fh = "hintnone";
    if (g_strcmp0(gsettings.font_hinting, "slight") == 0) fh = "hintslight";
    else if (g_strcmp0(gsettings.font_hinting, "medium") == 0) fh = "hintmedium";
    else if (g_strcmp0(gsettings.font_hinting, "full") == 0) fh = "hintfull";
    g_string_append_printf(content, "Xft/HintStyle \"%s\"\n", fh);
    g_string_append_printf(content, "Xft/RGBA \"%s\"\n", gsettings.font_rgba_order);

    g_file_set_contents(config_file, content->str, -1, NULL);
    g_string_free(content, TRUE);
    g_free(config_file);
    g_free(config_dir);
}

void clear_gtk4_symlinks(void) {
    gchar *config_dir = g_build_filename(config_home(), "gtk-4.0", NULL);
    const gchar *files[] = {"gtk.css", "gtk-dark.css", "assets"};
    for (int i = 0; i < 3; i++) {
        gchar *fpath = g_build_filename(config_dir, files[i], NULL);
        if (g_file_test(fpath, G_FILE_TEST_IS_SYMLINK)) {
            g_unlink(fpath);
        }
        g_free(fpath);
    }
    g_free(config_dir);
}

void link_gtk4_stuff(void) {
    if (!gsettings.gtk_theme || !gtk_theme_paths) return;
    gchar *theme_path = g_hash_table_lookup(gtk_theme_paths, gsettings.gtk_theme);
    if (!theme_path) return;
    
    gchar *theme_gtk4 = g_build_filename(theme_path, "gtk-4.0", NULL);
    if (!path_exists(theme_gtk4)) {
        g_free(theme_gtk4);
        return;
    }
    
    clear_gtk4_symlinks();
    
    gchar *config_dir = g_build_filename(config_home(), "gtk-4.0", NULL);
    make_dir(config_dir);
    
    const gchar *files[] = {"gtk.css", "gtk-dark.css", "assets"};
    for (int i = 0; i < 3; i++) {
        gchar *src = g_build_filename(theme_gtk4, files[i], NULL);
        if (path_exists(src)) {
            gchar *dst = g_build_filename(config_dir, files[i], NULL);
            symlink(src, dst);
            g_free(dst);
        }
        g_free(src);
    }
    
    g_free(config_dir);
    g_free(theme_gtk4);
}

void save_gtk_ini_4(void) {
    gchar *config_dir = g_build_filename(config_home(), "gtk-4.0", NULL);
    make_dir(config_dir);
    gchar *config_file = g_build_filename(config_dir, "settings.ini", NULL);
    
    GString *content = g_string_new("[Settings]\n");
    g_string_append_printf(content, "gtk-theme-name=%s\n", gsettings.gtk_theme);
    g_string_append_printf(content, "gtk-icon-theme-name=%s\n", gsettings.icon_theme);
    g_string_append_printf(content, "gtk-font-name=%s\n", gsettings.font_name);
    g_string_append_printf(content, "gtk-cursor-theme-name=%s\n", gsettings.cursor_theme);
    g_string_append_printf(content, "gtk-cursor-theme-size=%d\n", gsettings.cursor_size);
    g_string_append_printf(content, "gtk-application-prefer-dark-theme=%d\n", g_strcmp0(gsettings.color_scheme, "prefer-dark") == 0 ? 1 : 0);

    g_file_set_contents(config_file, content->str, -1, NULL);
    g_string_free(content, TRUE);
    g_free(config_file);
    g_free(config_dir);
}

void get_data_dirs(void) {
    if (data_dirs) return;
    
    GPtrArray *dirs = g_ptr_array_new();
    gchar *d_home = data_home();
    g_ptr_array_add(dirs, d_home);

    const gchar *xdg_data_dirs = g_getenv("XDG_DATA_DIRS");
    if (!xdg_data_dirs || !*xdg_data_dirs) {
        xdg_data_dirs = "/usr/local/share/:/usr/share/";
    }

    gchar **split_dirs = g_strsplit(xdg_data_dirs, ":", -1);
    for (int i = 0; split_dirs[i] != NULL; i++) {
        if (split_dirs[i][0] != '\0') {
            g_ptr_array_add(dirs, g_strdup(split_dirs[i]));
        }
    }
    g_strfreev(split_dirs);

    GPtrArray *confirmed = g_ptr_array_new();
    for (guint i = 0; i < dirs->len; i++) {
        gchar *d = g_ptr_array_index(dirs, i);
        if (path_exists(d)) {
            g_ptr_array_add(confirmed, g_strdup(d));
        }
    }
    g_ptr_array_add(confirmed, NULL); // NULL terminate

    g_ptr_array_free(dirs, TRUE);
    data_dirs = (gchar **)g_ptr_array_free(confirmed, FALSE);
}

static gint sort_string_cmp(gconstpointer a, gconstpointer b) {
    return g_strcmp0((const gchar *)a, (const gchar *)b);
}

GList* get_theme_names(void) {
    get_data_dirs();
    
    if (gtk_theme_paths) {
        g_hash_table_destroy(gtk_theme_paths);
    }
    gtk_theme_paths = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    GPtrArray *theme_base_dirs = g_ptr_array_new();
    for (int i = 0; data_dirs[i] != NULL; i++) {
        gchar *theme_dir = g_build_filename(data_dirs[i], "themes", NULL);
        if (path_exists(theme_dir)) {
            g_ptr_array_add(theme_base_dirs, theme_dir);
        } else {
            g_free(theme_dir);
        }
    }

    gchar *home_themes = g_build_filename(g_get_home_dir(), ".themes", NULL);
    if (path_exists(home_themes)) {
        g_ptr_array_add(theme_base_dirs, home_themes);
    } else {
        g_free(home_themes);
    }

    GList *names = NULL;

    for (guint i = 0; i < theme_base_dirs->len; i++) {
        gchar *base_dir = g_ptr_array_index(theme_base_dirs, i);
        GDir *dir = g_dir_open(base_dir, 0, NULL);
        if (dir) {
            const gchar *name;
            while ((name = g_dir_read_name(dir)) != NULL) {
                if (g_strcmp0(name, "Default") == 0 || g_strcmp0(name, "Emacs") == 0) continue;

                gchar *theme_dir = g_build_filename(base_dir, name, NULL);
                if (g_file_test(theme_dir, G_FILE_TEST_IS_DIR)) {
                    // Check if it has a gtk- subfolder
                    GDir *subdir = g_dir_open(theme_dir, 0, NULL);
                    gboolean has_gtk = FALSE;
                    if (subdir) {
                        const gchar *subname;
                        while ((subname = g_dir_read_name(subdir)) != NULL) {
                            if (g_str_has_prefix(subname, "gtk-")) {
                                gchar *subpath = g_build_filename(theme_dir, subname, NULL);
                                if (g_file_test(subpath, G_FILE_TEST_IS_DIR)) {
                                    has_gtk = TRUE;
                                }
                                g_free(subpath);
                                if (has_gtk) break;
                            }
                        }
                        g_dir_close(subdir);
                    }
                    if (has_gtk) {
                        if (!g_hash_table_lookup(gtk_theme_paths, name)) {
                            names = g_list_append(names, g_strdup(name));
                            g_hash_table_insert(gtk_theme_paths, g_strdup(name), g_strdup(theme_dir));
                        }
                    }
                }
                g_free(theme_dir);
            }
            g_dir_close(dir);
        }
    }

    g_ptr_array_free(theme_base_dirs, TRUE);

    names = g_list_sort(names, sort_string_cmp);
    return names;
}

static gchar* read_icon_theme_name(const gchar *path) {
    gchar *index_path = g_build_filename(path, "index.theme", NULL);
    gchar *name = NULL;
    gchar *contents = NULL;
    gsize length = 0;

    if (g_file_get_contents(index_path, &contents, &length, NULL)) {
        gchar **lines = g_strsplit(contents, "\n", -1);
        for (int i = 0; lines[i]; i++) {
            gchar *line = g_strstrip(lines[i]);
            if (g_str_has_prefix(line, "Name=") || g_str_has_prefix(line, "Name =")) {
                gchar **parts = g_strsplit(line, "=", 2);
                if (parts[1]) {
                    name = g_strdup(g_strstrip(parts[1]));
                }
                g_strfreev(parts);
                break;
            }
        }
        g_strfreev(lines);
        g_free(contents);
    }
    g_free(index_path);
    return name;
}

static gboolean has_directories_key(const gchar *path) {
    gchar *index_path = g_build_filename(path, "index.theme", NULL);
    gboolean has_dirs = FALSE;
    gchar *contents = NULL;
    gsize length = 0;

    if (g_file_get_contents(index_path, &contents, &length, NULL)) {
        gchar **lines = g_strsplit(contents, "\n", -1);
        for (int i = 0; lines[i]; i++) {
            gchar *line = g_strstrip(lines[i]);
            if (g_str_has_prefix(line, "Directories=") || g_str_has_prefix(line, "Directories =")) {
                has_dirs = TRUE;
                break;
            }
        }
        g_strfreev(lines);
        g_free(contents);
    }
    g_free(index_path);
    return has_dirs;
}

GList* get_icon_theme_names(void) {
    get_data_dirs();
    
    if (icon_theme_names_map) {
        g_hash_table_destroy(icon_theme_names_map);
    }
    icon_theme_names_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    GPtrArray *icon_base_dirs = g_ptr_array_new();
    for (int i = 0; data_dirs[i] != NULL; i++) {
        gchar *icon_dir = g_build_filename(data_dirs[i], "icons", NULL);
        if (path_exists(icon_dir)) {
            g_ptr_array_add(icon_base_dirs, icon_dir);
        } else {
            g_free(icon_dir);
        }
    }

    gchar *home_icons = g_build_filename(g_get_home_dir(), ".icons", NULL);
    if (path_exists(home_icons)) {
        g_ptr_array_add(icon_base_dirs, home_icons);
    } else {
        g_free(home_icons);
    }

    GList *names = NULL;

    for (guint i = 0; i < icon_base_dirs->len; i++) {
        gchar *base_dir = g_ptr_array_index(icon_base_dirs, i);
        GDir *dir = g_dir_open(base_dir, 0, NULL);
        if (dir) {
            const gchar *folder_name;
            while ((folder_name = g_dir_read_name(dir)) != NULL) {
                if (g_strcmp0(folder_name, "default") == 0 || 
                    g_strcmp0(folder_name, "hicolor") == 0 || 
                    g_strcmp0(folder_name, "locolor") == 0) continue;

                gchar *theme_dir = g_build_filename(base_dir, folder_name, NULL);
                if (g_file_test(theme_dir, G_FILE_TEST_IS_DIR)) {
                    if (has_directories_key(theme_dir)) {
                        gchar *display_name = read_icon_theme_name(theme_dir);
                        if (display_name && !g_hash_table_lookup(icon_theme_names_map, display_name)) {
                            names = g_list_append(names, g_strdup(display_name));
                            g_hash_table_insert(icon_theme_names_map, g_strdup(display_name), g_strdup(folder_name));
                        }
                        g_free(display_name);
                    }
                }
                g_free(theme_dir);
            }
            g_dir_close(dir);
        }
    }

    g_ptr_array_free(icon_base_dirs, TRUE);

    names = g_list_sort(names, sort_string_cmp);
    return names;
}

GList* get_cursor_themes(void) {
    get_data_dirs();
    
    if (cursor_theme_names_map) {
        g_hash_table_destroy(cursor_theme_names_map);
    }
    cursor_theme_names_map = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    if (cursor_theme_paths) {
        g_hash_table_destroy(cursor_theme_paths);
    }
    cursor_theme_paths = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    GPtrArray *icon_base_dirs = g_ptr_array_new();
    for (int i = 0; data_dirs[i] != NULL; i++) {
        gchar *icon_dir = g_build_filename(data_dirs[i], "icons", NULL);
        if (path_exists(icon_dir)) {
            g_ptr_array_add(icon_base_dirs, icon_dir);
        } else {
            g_free(icon_dir);
        }
    }

    gchar *home_icons = g_build_filename(g_get_home_dir(), ".icons", NULL);
    if (path_exists(home_icons)) {
        g_ptr_array_add(icon_base_dirs, home_icons);
    } else {
        g_free(home_icons);
    }

    GList *names = NULL;

    for (guint i = 0; i < icon_base_dirs->len; i++) {
        gchar *base_dir = g_ptr_array_index(icon_base_dirs, i);
        GDir *dir = g_dir_open(base_dir, 0, NULL);
        if (dir) {
            const gchar *folder_name;
            while ((folder_name = g_dir_read_name(dir)) != NULL) {
                if (g_strcmp0(folder_name, "default") == 0 || 
                    g_strcmp0(folder_name, "hicolor") == 0 || 
                    g_strcmp0(folder_name, "locolor") == 0) continue;

                gchar *theme_dir = g_build_filename(base_dir, folder_name, NULL);
                if (g_file_test(theme_dir, G_FILE_TEST_IS_DIR)) {
                    gchar *cursors_dir = g_build_filename(theme_dir, "cursors", NULL);
                    if (g_file_test(cursors_dir, G_FILE_TEST_IS_DIR)) {
                        gchar *display_name = read_icon_theme_name(theme_dir);
                        if (!display_name) {
                            display_name = g_strdup(folder_name);
                        }
                        if (!g_hash_table_lookup(cursor_theme_names_map, display_name)) {
                            names = g_list_append(names, g_strdup(display_name));
                            g_hash_table_insert(cursor_theme_names_map, g_strdup(display_name), g_strdup(folder_name));
                            g_hash_table_insert(cursor_theme_paths, g_strdup(folder_name), g_strdup(cursors_dir));
                        } else {
                            g_free(display_name);
                        }
                    }
                    g_free(cursors_dir);
                }
                g_free(theme_dir);
            }
            g_dir_close(dir);
        }
    }

    g_ptr_array_free(icon_base_dirs, TRUE);

    names = g_list_sort(names, sort_string_cmp);
    return names;
}

