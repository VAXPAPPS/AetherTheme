#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include "tools.h"
#include "uicomponents.h"
#include "stylepak.h"
#include "theme_manager.h"

// Global pointers to widgets
GtkViewport *viewport_list = NULL;
GtkScrolledWindow *scrolled_window = NULL;
GtkGrid *grid = NULL;
GtkMenuBar *menubar = NULL;

static void on_window_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

static gboolean on_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if (event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
        return TRUE;
    }
    return FALSE;
}

static void on_btn_close_clicked(GtkButton *button, gpointer data) {
    gtk_main_quit();
}

static void on_btn_apply_clicked(GtkButton *button, gpointer data) {
    apply_gsettings();
    save_gsettings_backup();

    if (preferences.export_settings_ini) save_gtk_ini_3();
    if (preferences.export_gtkrc_20) save_gtk_rc_20();
    if (preferences.export_index_theme) save_index_theme();
    save_xresources();
    if (preferences.export_xsettingsd) {
        save_xsettingsd();
        system("killall -HUP xsettingsd 2>/dev/null");
    }
    if (preferences.export_gtk4_symlinks) {
        link_gtk4_stuff();
        save_gtk_ini_4();
    }
    
    if (preferences.flatpak_export_gtk_theme_override) {
        override_flatpak_gtk_theme();
    } else if (flatpak_available()) {
        unset_flatpak_gtk_theme();
    }
    
    if (preferences.flatpak_export_icon_theme_override) {
        override_flatpak_icon_theme();
    } else if (flatpak_available()) {
        unset_flatpak_icon_theme();
    }
    
    if (preferences.flatpak_install_current_gtk_theme) {
        install_user_theme("", NULL);
    }
    
    save_preferences();
    
    // Update DBus and systemd activation environment for future processes
    if (gsettings.cursor_theme) {
        gchar *env_cmd = g_strdup_printf(
            "dbus-update-activation-environment --systemd XCURSOR_THEME=\"%s\" XCURSOR_SIZE=\"%d\" 2>/dev/null; "
            "systemctl --user import-environment XCURSOR_THEME XCURSOR_SIZE 2>/dev/null",
            gsettings.cursor_theme, gsettings.cursor_size
        );
        system(env_cmd);
        g_free(env_cmd);
    }
    
    // Signal X11 / Xwayland root window
    system("xsetroot -cursor_name left_ptr 2>/dev/null");
}

static gboolean on_nav_item_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    void (*func)(void) = (void (*)(void))data;
    if (func) func();
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    theme_manager_init();
    load_preferences();
    read_gsettings();

    GtkBuilder *builder = gtk_builder_new();
    GError *err = NULL;

    if (!gtk_builder_add_from_file(builder, "stuff/main.glade", &err)) {
        g_clear_error(&err);
        if (!gtk_builder_add_from_file(builder, "/usr/share/AetherTheme/main.glade", &err)) {
            g_printerr("Failed to load glade file: %s\n", err ? err->message : "unknown error");
            g_clear_error(&err);
            g_object_unref(builder);
            return 1;
        }
    }

    GtkWidget *win = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    
    // Enable transparency
    GdkScreen *screen = gtk_widget_get_screen(win);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual != NULL && gdk_screen_is_composited(screen)) {
        gtk_widget_set_visual(win, visual);
    }

    // Apply transparent black background
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "window, window.background, window.csd, headerbar, .titlebar { background-color: rgba(0, 0, 0, 0.3); background-image: none; box-shadow: none; border: none; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "AetherTheme");
    gtk_window_set_titlebar(GTK_WINDOW(win), header);

    g_signal_connect(win, "destroy", G_CALLBACK(on_window_destroy), NULL);
    g_signal_connect(win, "key-release-event", G_CALLBACK(on_key_release_event), NULL);

    viewport_list = GTK_VIEWPORT(gtk_builder_get_object(builder, "viewport-list"));
    scrolled_window = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "scrolled-window"));
    grid = GTK_GRID(gtk_builder_get_object(builder, "grid"));
    menubar = GTK_MENU_BAR(gtk_builder_get_object(builder, "menubar"));

    g_object_ref(menubar);
    gtk_container_remove(GTK_CONTAINER(grid), GTK_WIDGET(menubar));
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), GTK_WIDGET(menubar));
    g_object_unref(menubar);

    GtkWidget *item_widgets = GTK_WIDGET(gtk_builder_get_object(builder, "item-widgets"));
    g_signal_connect(item_widgets, "button-press-event", G_CALLBACK(on_nav_item_clicked), display_themes);

    GtkWidget *item_icons = GTK_WIDGET(gtk_builder_get_object(builder, "item-icons"));
    g_signal_connect(item_icons, "button-press-event", G_CALLBACK(on_nav_item_clicked), display_icon_themes);

    GtkWidget *item_cursors = GTK_WIDGET(gtk_builder_get_object(builder, "item-cursors"));
    g_signal_connect(item_cursors, "button-press-event", G_CALLBACK(on_nav_item_clicked), display_cursor_themes);

    GtkWidget *item_font = GTK_WIDGET(gtk_builder_get_object(builder, "item-font"));
    g_signal_connect(item_font, "button-press-event", G_CALLBACK(on_nav_item_clicked), display_font_settings_form);

    GtkWidget *item_other = GTK_WIDGET(gtk_builder_get_object(builder, "item-other"));
    g_signal_connect(item_other, "button-press-event", G_CALLBACK(on_nav_item_clicked), display_other_settings_form);

    GtkWidget *item_preferences = GTK_WIDGET(gtk_builder_get_object(builder, "item-preferences"));
    g_signal_connect(item_preferences, "button-press-event", G_CALLBACK(on_nav_item_clicked), display_program_settings_form);

    GtkWidget *btn_close = GTK_WIDGET(gtk_builder_get_object(builder, "btn-close"));
    g_signal_connect(btn_close, "clicked", G_CALLBACK(on_btn_close_clicked), NULL);

    GtkWidget *btn_apply = GTK_WIDGET(gtk_builder_get_object(builder, "btn-apply"));
    g_signal_connect(btn_apply, "clicked", G_CALLBACK(on_btn_apply_clicked), NULL);

    // Initial display
    display_themes();

    gtk_widget_show_all(win);

    gtk_main();

    g_object_unref(builder);
    return 0;
}
