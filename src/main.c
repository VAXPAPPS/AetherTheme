#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include "tools.h"
#include "uicomponents.h"
#include "stylepak.h"

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
    if (preferences.export_xsettingsd) save_xsettingsd();
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
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // TODO: implement CLI flags, setup defaults, load preferences

    GtkBuilder *builder = gtk_builder_new_from_file("stuff/main.glade");
    if (!builder) {
        g_printerr("Failed to load glade file\n");
        return 1;
    }

    GtkWidget *win = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    g_signal_connect(win, "destroy", G_CALLBACK(on_window_destroy), NULL);
    g_signal_connect(win, "key-release-event", G_CALLBACK(on_key_release_event), NULL);

    viewport_list = GTK_VIEWPORT(gtk_builder_get_object(builder, "viewport-list"));
    scrolled_window = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "scrolled-window"));
    grid = GTK_GRID(gtk_builder_get_object(builder, "grid"));
    menubar = GTK_MENU_BAR(gtk_builder_get_object(builder, "menubar"));

    GtkWidget *item_widgets = GTK_WIDGET(gtk_builder_get_object(builder, "item-widgets"));
    g_signal_connect(item_widgets, "activate", G_CALLBACK(display_themes), NULL);

    GtkWidget *item_icons = GTK_WIDGET(gtk_builder_get_object(builder, "item-icons"));
    g_signal_connect(item_icons, "activate", G_CALLBACK(display_icon_themes), NULL);

    GtkWidget *item_cursors = GTK_WIDGET(gtk_builder_get_object(builder, "item-cursors"));
    g_signal_connect(item_cursors, "activate", G_CALLBACK(display_cursor_themes), NULL);

    GtkWidget *item_font = GTK_WIDGET(gtk_builder_get_object(builder, "item-font"));
    g_signal_connect(item_font, "activate", G_CALLBACK(display_font_settings_form), NULL);

    GtkWidget *item_other = GTK_WIDGET(gtk_builder_get_object(builder, "item-other"));
    g_signal_connect(item_other, "activate", G_CALLBACK(display_other_settings_form), NULL);

    GtkWidget *item_preferences = GTK_WIDGET(gtk_builder_get_object(builder, "item-preferences"));
    g_signal_connect(item_preferences, "activate", G_CALLBACK(display_program_settings_form), NULL);

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
