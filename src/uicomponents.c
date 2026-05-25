#include "uicomponents.h"
#include "tools.h"
#include <stdio.h>
#include <X11/Xcursor/Xcursor.h>

extern GtkViewport *viewport_list;
extern GtkScrolledWindow *scrolled_window;
extern GtkGrid *grid;

static GtkWidget *listbox = NULL;
static GtkWidget *preview = NULL;
static GtkWidget *theme_settings_selector = NULL;
static GtkWidget *cursor_size_selector = NULL;

static gboolean on_theme_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean on_theme_focus(GtkWidget *widget, GdkEventFocus *event, gpointer data);

static GtkWidget* setup_widgets_preview(void) {
    GtkWidget *frame = gtk_frame_new("  Widget Style Preview  ");
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    g_object_set(frame, "margin", 6, "valign", GTK_ALIGN_START, NULL);

    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(g), 6);
    gtk_grid_set_column_spacing(GTK_GRID(g), 12);
    g_object_set(g, "margin", 6, NULL);
    gtk_container_add(GTK_CONTAINER(frame), g);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    g_object_set(box, "hexpand", TRUE, NULL);
    gtk_grid_attach(GTK_GRID(g), box, 0, 0, 3, 1);

    GtkWidget *btn1 = gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_can_focus(btn1, FALSE);
    gtk_box_pack_start(GTK_BOX(box), btn1, FALSE, FALSE, 0);

    GtkWidget *btn2 = gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_can_focus(btn2, FALSE);
    gtk_box_pack_start(GTK_BOX(box), btn2, FALSE, FALSE, 0);

    GtkWidget *btn3 = gtk_button_new_from_icon_name("process-stop", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_can_focus(btn3, FALSE);
    gtk_box_pack_start(GTK_BOX(box), btn3, FALSE, FALSE, 0);

    GtkWidget *entry = gtk_entry_new();
    gtk_widget_set_can_focus(entry, FALSE);
    gtk_box_pack_start(GTK_BOX(box), entry, TRUE, TRUE, 0);

    GtkWidget *cb = gtk_check_button_new_with_label("Check Button");
    gtk_widget_set_can_focus(cb, FALSE);
    gtk_grid_attach(GTK_GRID(g), cb, 0, 1, 1, 1);

    GtkWidget *rb = gtk_radio_button_new_with_label(NULL, "Radio Button");
    gtk_widget_set_can_focus(rb, FALSE);
    gtk_grid_attach(GTK_GRID(g), rb, 0, 2, 1, 1);

    GtkWidget *sb = gtk_spin_button_new_with_range(0, 1000, 10);
    gtk_widget_set_can_focus(sb, FALSE);
    gtk_grid_attach(GTK_GRID(g), sb, 0, 3, 1, 1);

    GtkWidget *btn4 = gtk_button_new_with_label("Button");
    GtkWidget *icon = gtk_image_new_from_icon_name("search", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn4), icon);
    gtk_widget_set_can_focus(btn4, FALSE);
    gtk_grid_attach(GTK_GRID(g), btn4, 1, 3, 1, 1);

    GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_widget_set_can_focus(scale, FALSE);
    gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
    gtk_range_set_value(GTK_RANGE(scale), 50);
    gtk_grid_attach(GTK_GRID(g), scale, 1, 1, 2, 1);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_can_focus(sep, FALSE);
    g_object_set(sep, "valign", GTK_ALIGN_CENTER, NULL);
    gtk_grid_attach(GTK_GRID(g), sep, 1, 2, 2, 1);

    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "1", "Entry 1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "2", "Entry 2");
    gtk_widget_set_can_focus(combo, FALSE);
    gtk_grid_attach(GTK_GRID(g), combo, 2, 3, 1, 1);

    GtkWidget *pb = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pb), 0.3);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pb), "30%");
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(pb), TRUE);
    g_object_set(pb, "margin-bottom", 6, NULL);
    gtk_grid_attach(GTK_GRID(g), pb, 0, 4, 3, 1);

    return frame;
}

static GtkWidget* setup_icons_preview(void) {
    GtkWidget *frame = gtk_frame_new("  Icon Theme Preview  ");
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    g_object_set(frame, "margin", 6, "valign", GTK_ALIGN_START, NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    g_object_set(box, "hexpand", TRUE, NULL);
    gtk_container_add(GTK_CONTAINER(frame), box);

    GtkWidget *flowbox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flowbox), 7);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flowbox), 7);
    gtk_box_pack_start(GTK_BOX(box), flowbox, FALSE, FALSE, 0);

    const gchar *icons[] = {
        "user-home", "user-desktop", "folder", "folder-remote",
        "user-trash", "x-office-document", "text-x-generic",
        "audio-x-generic", "image-x-generic", "video-x-generic",
        "go-previous", "go-next", "go-up", "go-home",
        "system-search", "edit-copy", "edit-paste", "edit-cut",
        "edit-delete", "view-refresh", "list-add", "list-remove",
        "document-new", "document-open", "document-save", "document-print",
        "dialog-information", "dialog-warning", "dialog-error", "dialog-question",
        "weather-clear", "weather-clouds", "weather-showers", "weather-snow",
        NULL
    };

    for (int i = 0; icons[i]; i++) {
        GtkWidget *img = gtk_image_new_from_icon_name(icons[i], GTK_ICON_SIZE_DIALOG);
        GtkWidget *fb_child = gtk_flow_box_child_new();
        gtk_widget_set_can_focus(fb_child, FALSE);
        gtk_container_add(GTK_CONTAINER(fb_child), img);
        gtk_container_add(GTK_CONTAINER(flowbox), fb_child);
    }

    return frame;
}

static GdkPixbuf* load_xcursor_as_pixbuf(const char *filename, int size) {
    XcursorImage *img = XcursorFilenameLoadImage(filename, size);
    if (!img) return NULL;
    
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, img->width, img->height);
    if (!pixbuf) {
        XcursorImageDestroy(img);
        return NULL;
    }
    
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    
    for (int y = 0; y < img->height; y++) {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < img->width; x++) {
            XcursorPixel p = img->pixels[y * img->width + x];
            guchar a = (p >> 24) & 0xff;
            guchar r = (p >> 16) & 0xff;
            guchar g = (p >> 8) & 0xff;
            guchar b = p & 0xff;
            
            row[x * 4 + 0] = r;
            row[x * 4 + 1] = g;
            row[x * 4 + 2] = b;
            row[x * 4 + 3] = a;
        }
    }
    
    XcursorImageDestroy(img);
    return pixbuf;
}

static GtkWidget* setup_cursors_preview(void) {
    GtkWidget *frame = gtk_frame_new("  Cursor Theme Preview  ");
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    g_object_set(frame, "margin", 6, "valign", GTK_ALIGN_START, NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    g_object_set(box, "hexpand", TRUE, "margin", 12, NULL);
    gtk_container_add(GTK_CONTAINER(frame), box);

    GtkWidget *flowbox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flowbox), 6);
    gtk_box_pack_start(GTK_BOX(box), flowbox, FALSE, FALSE, 0);

    const gchar *cursor_names[] = {
        "left_ptr", "right_ptr", "pointer", "crosshair", "wait", "text", "vertical-text", "zoom-in", NULL
    };

    gchar *theme_dir = gsettings.cursor_theme ? g_hash_table_lookup(cursor_theme_paths, gsettings.cursor_theme) : NULL;

    gboolean any_loaded = FALSE;
    for (int i = 0; cursor_names[i]; i++) {
        gchar *cursor_path = NULL;
        if (theme_dir) {
            cursor_path = g_build_filename(theme_dir, cursor_names[i], NULL);
        } else {
            cursor_path = g_build_filename(".", cursor_names[i], NULL);
        }
        
        GdkPixbuf *pixbuf = load_xcursor_as_pixbuf(cursor_path, 32);
        
        // Try fallback to local dir just in case for testing
        if (!pixbuf && theme_dir) {
            gchar *local_path = g_build_filename(".", cursor_names[i], NULL);
            pixbuf = load_xcursor_as_pixbuf(local_path, 32);
            g_free(local_path);
        }

        g_free(cursor_path);

        if (pixbuf) {
            GtkWidget *img = gtk_image_new_from_pixbuf(pixbuf);
            g_object_unref(pixbuf);
            
            GtkWidget *fb_child = gtk_flow_box_child_new();
            gtk_widget_set_can_focus(fb_child, FALSE);
            gtk_container_add(GTK_CONTAINER(fb_child), img);
            gtk_container_add(GTK_CONTAINER(flowbox), fb_child);
            any_loaded = TRUE;
        }
    }

    if (!any_loaded) {
        GtkWidget *lbl = gtk_label_new("No standard cursors found for this theme.");
        gtk_container_add(GTK_CONTAINER(flowbox), lbl);
    }
    
    return frame;
}

static void on_font_set(GtkFontButton *button, gpointer data) {
    g_free(gsettings.font_name);
    gsettings.font_name = g_strdup(gtk_font_chooser_get_font(GTK_FONT_CHOOSER(button)));
    g_object_set(gtk_settings_get_default(), "gtk-font-name", gsettings.font_name, NULL);
}

static void on_color_scheme_changed(GtkComboBoxText *combo, gpointer data) {
    g_free(gsettings.color_scheme);
    gsettings.color_scheme = g_strdup(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo)));
    gboolean prefer_dark = (g_strcmp0(gsettings.color_scheme, "prefer-dark") == 0);
    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", prefer_dark, NULL);
}

static GtkWidget* setup_theme_settings_form(void) {
    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g), 12);
    gtk_grid_set_row_spacing(GTK_GRID(g), 6);
    g_object_set(g, "margin", 12, NULL);

    GtkWidget *lbl1 = gtk_label_new("Default font:");
    gtk_widget_set_halign(lbl1, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl1, 0, 0, 1, 1);

    GtkWidget *fb = gtk_font_button_new();
    g_object_set(fb, "valign", GTK_ALIGN_CENTER, NULL);
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(fb), gsettings.font_name);
    g_signal_connect(fb, "font-set", G_CALLBACK(on_font_set), NULL);
    gtk_grid_attach(GTK_GRID(g), fb, 1, 0, 1, 1);

    GtkWidget *lbl2 = gtk_label_new("Color scheme:");
    gtk_widget_set_halign(lbl2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl2, 0, 1, 1, 1);

    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "default", "Default");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "prefer-dark", "Prefer Dark");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), "prefer-light", "Prefer Light");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), gsettings.color_scheme);
    gtk_widget_set_can_focus(combo, FALSE);
    g_signal_connect(combo, "changed", G_CALLBACK(on_color_scheme_changed), NULL);
    gtk_grid_attach(GTK_GRID(g), combo, 1, 1, 1, 1);

    return g;
}

static gboolean on_theme_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    const gchar *theme_name = (const gchar *)data;
    g_print("Selected theme: %s\n", theme_name);
    g_free(gsettings.gtk_theme);
    gsettings.gtk_theme = g_strdup(theme_name);
    g_object_set(gtk_settings_get_default(), "gtk-theme-name", theme_name, NULL);
    return FALSE;
}

static gboolean on_theme_focus(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
    const gchar *theme_name = (const gchar *)data;
    g_free(gsettings.gtk_theme);
    gsettings.gtk_theme = g_strdup(theme_name);
    g_object_set(gtk_settings_get_default(), "gtk-theme-name", theme_name, NULL);
    return FALSE;
}

void display_themes(void) {
    destroy_content();

    listbox = gtk_list_box_new();
    GList *theme_names = get_theme_names();
    GtkWidget *row_to_select = NULL;

    for (GList *l = theme_names; l != NULL; l = l->next) {
        const gchar *name = (const gchar *)l->data;
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *event_box = gtk_event_box_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(event_box), box);

        GtkWidget *lbl = gtk_label_new(name);
        g_object_set(lbl, "margin-start", 6, "margin-end", 6, NULL);

        // We use g_strdup to pass the string, and maybe leak it, or pass the string from the list.
        // The strings in the list are allocated, but we should probably use g_object_set_data or similar to avoid leaks if it was complex.
        // For now just pass the pointer from the list.
        
        g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_theme_clicked), (gpointer)name);
        g_signal_connect(row, "focus-in-event", G_CALLBACK(on_theme_focus), (gpointer)name);

        if (gsettings.gtk_theme && g_strcmp0(name, gsettings.gtk_theme) == 0) {
            row_to_select = row;
        }

        gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(row), event_box);
        gtk_container_add(GTK_CONTAINER(listbox), row);
    }

    gtk_container_add(GTK_CONTAINER(viewport_list), listbox);

    preview = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(preview), setup_theme_settings_form(), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(preview), setup_widgets_preview(), FALSE, FALSE, 0);
    gtk_grid_attach(grid, preview, 1, 1, 1, 1);

    if (row_to_select) {
        gtk_list_box_select_row(GTK_LIST_BOX(listbox), GTK_LIST_BOX_ROW(row_to_select));
        gtk_widget_grab_focus(row_to_select);
    }

    gtk_widget_show_all(GTK_WIDGET(viewport_list));
    gtk_widget_show_all(GTK_WIDGET(grid));
    gtk_widget_show(GTK_WIDGET(scrolled_window));
}
static gboolean on_icon_theme_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    const gchar *theme_name = (const gchar *)data;
    g_print("Selected icon theme: %s\n", theme_name);
    gchar *folder_name = g_hash_table_lookup(icon_theme_names_map, theme_name);
    gchar *theme_val = folder_name ? folder_name : (gchar*)theme_name;
    g_free(gsettings.icon_theme);
    gsettings.icon_theme = g_strdup(theme_val);
    g_object_set(gtk_settings_get_default(), "gtk-icon-theme-name", theme_val, NULL);
    return FALSE;
}

static gboolean on_icon_theme_focus(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
    const gchar *theme_name = (const gchar *)data;
    gchar *folder_name = g_hash_table_lookup(icon_theme_names_map, theme_name);
    gchar *theme_val = folder_name ? folder_name : (gchar*)theme_name;
    g_free(gsettings.icon_theme);
    gsettings.icon_theme = g_strdup(theme_val);
    g_object_set(gtk_settings_get_default(), "gtk-icon-theme-name", theme_val, NULL);
    return FALSE;
}

void display_icon_themes(void) {
    destroy_content();

    listbox = gtk_list_box_new();
    GList *theme_names = get_icon_theme_names();
    GtkWidget *row_to_select = NULL;

    for (GList *l = theme_names; l != NULL; l = l->next) {
        const gchar *name = (const gchar *)l->data;
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *event_box = gtk_event_box_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(event_box), box);

        GtkWidget *lbl = gtk_label_new(name);
        g_object_set(lbl, "margin-start", 6, "margin-end", 6, NULL);
        
        g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_icon_theme_clicked), (gpointer)name);
        g_signal_connect(row, "focus-in-event", G_CALLBACK(on_icon_theme_focus), (gpointer)name);

        gchar *folder_name = g_hash_table_lookup(icon_theme_names_map, name);
        if (gsettings.icon_theme && folder_name && g_strcmp0(folder_name, gsettings.icon_theme) == 0) {
            row_to_select = row;
        }

        gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(row), event_box);
        gtk_container_add(GTK_CONTAINER(listbox), row);
    }

    gtk_container_add(GTK_CONTAINER(viewport_list), listbox);

    preview = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(preview), setup_icons_preview(), FALSE, FALSE, 0);
    gtk_grid_attach(grid, preview, 1, 1, 1, 1);

    if (row_to_select) {
        gtk_list_box_select_row(GTK_LIST_BOX(listbox), GTK_LIST_BOX_ROW(row_to_select));
        gtk_widget_grab_focus(row_to_select);
    }

    gtk_widget_show_all(GTK_WIDGET(viewport_list));
    gtk_widget_show_all(GTK_WIDGET(grid));
    gtk_widget_show(GTK_WIDGET(scrolled_window));
}

static void update_cursor_preview(void) {
    if (preview) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(preview));
        for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
        g_list_free(children);

        gtk_box_pack_start(GTK_BOX(preview), setup_cursors_preview(), FALSE, FALSE, 0);
        gtk_widget_show_all(preview);
    }
}

static gboolean on_cursor_theme_clicked(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    const gchar *theme_name = (const gchar *)data;
    gchar *folder_name = g_hash_table_lookup(cursor_theme_names_map, theme_name);
    gchar *theme_val = folder_name ? folder_name : (gchar*)theme_name;
    g_free(gsettings.cursor_theme);
    gsettings.cursor_theme = g_strdup(theme_val);
    g_object_set(gtk_settings_get_default(), "gtk-cursor-theme-name", theme_val, NULL);
    update_cursor_preview();
    return FALSE;
}

static gboolean on_cursor_theme_focus(GtkWidget *widget, GdkEventFocus *event, gpointer data) {
    const gchar *theme_name = (const gchar *)data;
    gchar *folder_name = g_hash_table_lookup(cursor_theme_names_map, theme_name);
    gchar *theme_val = folder_name ? folder_name : (gchar*)theme_name;
    g_free(gsettings.cursor_theme);
    gsettings.cursor_theme = g_strdup(theme_val);
    g_object_set(gtk_settings_get_default(), "gtk-cursor-theme-name", theme_val, NULL);
    update_cursor_preview();
    return FALSE;
}

static void on_cursor_size_changed(GtkSpinButton *spin_button, gpointer data) {
    gsettings.cursor_size = gtk_spin_button_get_value_as_int(spin_button);
    g_object_set(gtk_settings_get_default(), "gtk-cursor-theme-size", gsettings.cursor_size, NULL);
}

void display_cursor_themes(void) {
    destroy_content();

    listbox = gtk_list_box_new();
    GList *theme_names = get_cursor_themes();
    GtkWidget *row_to_select = NULL;

    for (GList *l = theme_names; l != NULL; l = l->next) {
        const gchar *name = (const gchar *)l->data;
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *event_box = gtk_event_box_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(event_box), box);

        GtkWidget *lbl = gtk_label_new(name);
        g_object_set(lbl, "margin-start", 6, "margin-end", 6, NULL);
        
        g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_cursor_theme_clicked), (gpointer)name);
        g_signal_connect(row, "focus-in-event", G_CALLBACK(on_cursor_theme_focus), (gpointer)name);

        gchar *folder_name = g_hash_table_lookup(cursor_theme_names_map, name);
        if (gsettings.cursor_theme && folder_name && g_strcmp0(folder_name, gsettings.cursor_theme) == 0) {
            row_to_select = row;
        }

        gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(row), event_box);
        gtk_container_add(GTK_CONTAINER(listbox), row);
    }

    gtk_container_add(GTK_CONTAINER(viewport_list), listbox);

    preview = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *size_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    g_object_set(size_box, "margin", 12, "hexpand", TRUE, "valign", GTK_ALIGN_START, NULL);
    GtkWidget *lbl_size = gtk_label_new("Cursor size:");
    gtk_box_pack_start(GTK_BOX(size_box), lbl_size, FALSE, FALSE, 0);
    GtkWidget *sb = gtk_spin_button_new_with_range(6, 1024, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb), gsettings.cursor_size);
    g_signal_connect(sb, "value-changed", G_CALLBACK(on_cursor_size_changed), NULL);
    gtk_box_pack_start(GTK_BOX(size_box), sb, FALSE, FALSE, 6);
    GtkWidget *lbl_def = gtk_label_new("(Default: 24)");
    gtk_box_pack_start(GTK_BOX(size_box), lbl_def, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(preview), size_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(preview), setup_cursors_preview(), FALSE, FALSE, 0);
    gtk_grid_attach(grid, preview, 1, 1, 1, 1);

    if (row_to_select) {
        gtk_list_box_select_row(GTK_LIST_BOX(listbox), GTK_LIST_BOX_ROW(row_to_select));
        gtk_widget_grab_focus(row_to_select);
    }

    gtk_widget_show_all(GTK_WIDGET(viewport_list));
    gtk_widget_show_all(GTK_WIDGET(grid));
    gtk_widget_show(GTK_WIDGET(scrolled_window));
}

static void on_font_hinting_changed(GtkComboBoxText *combo, gpointer data) {
    g_free(gsettings.font_hinting);
    gsettings.font_hinting = g_strdup(gtk_combo_box_text_get_active_text(combo));
}

static void on_font_rgba_changed(GtkComboBoxText *combo, gpointer data) {
    g_free(gsettings.font_rgba_order);
    gsettings.font_rgba_order = g_strdup(gtk_combo_box_text_get_active_text(combo));
}

static void on_font_antialiasing_changed(GtkComboBoxText *combo, gpointer data) {
    g_free(gsettings.font_antialiasing);
    gsettings.font_antialiasing = g_strdup(gtk_combo_box_text_get_active_text(combo));
    GtkWidget *combo_rgba = GTK_WIDGET(data);
    gtk_widget_set_sensitive(combo_rgba, g_strcmp0(gsettings.font_antialiasing, "rgba") == 0);
}

static void on_text_scaling_changed(GtkSpinButton *spin_button, gpointer data) {
    gsettings.text_scaling_factor = gtk_spin_button_get_value(spin_button);
}

void display_font_settings_form(void) {
    destroy_content();

    GtkWidget *frame = gtk_frame_new("  Font Settings  ");
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    g_object_set(frame, "margin", 6, NULL);

    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(g), 12);
    gtk_grid_set_column_spacing(GTK_GRID(g), 12);
    g_object_set(g, "margin", 6, "hexpand", TRUE, "vexpand", TRUE, NULL);
    gtk_container_add(GTK_CONTAINER(frame), g);

    GtkWidget *lbl1 = gtk_label_new("Hinting:");
    gtk_widget_set_halign(lbl1, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl1, 0, 0, 1, 1);

    GtkWidget *combo_hinting = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_hinting), "none", "none");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_hinting), "slight", "slight");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_hinting), "medium", "medium");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_hinting), "full", "full");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo_hinting), gsettings.font_hinting);
    g_signal_connect(combo_hinting, "changed", G_CALLBACK(on_font_hinting_changed), NULL);
    gtk_grid_attach(GTK_GRID(g), combo_hinting, 1, 0, 1, 1);

    GtkWidget *lbl2 = gtk_label_new("Antialiasing:");
    gtk_widget_set_halign(lbl2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl2, 0, 1, 1, 1);

    GtkWidget *combo_rgba = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_rgba), "rgb", "RGB");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_rgba), "bgr", "BGR");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_rgba), "vrgb", "VRGB");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_rgba), "vbgr", "VBGR");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo_rgba), gsettings.font_rgba_order);
    g_signal_connect(combo_rgba, "changed", G_CALLBACK(on_font_rgba_changed), NULL);
    
    GtkWidget *combo_anti = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_anti), "none", "none");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_anti), "grayscale", "grayscale");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_anti), "rgba", "rgba");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo_anti), gsettings.font_antialiasing);
    g_signal_connect(combo_anti, "changed", G_CALLBACK(on_font_antialiasing_changed), combo_rgba);
    gtk_grid_attach(GTK_GRID(g), combo_anti, 1, 1, 1, 1);

    GtkWidget *lbl3 = gtk_label_new("RGBA Order:");
    gtk_widget_set_halign(lbl3, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl3, 0, 2, 1, 1);

    gtk_widget_set_sensitive(combo_rgba, g_strcmp0(gsettings.font_antialiasing, "rgba") == 0);
    gtk_grid_attach(GTK_GRID(g), combo_rgba, 1, 2, 1, 1);

    GtkWidget *lbl4 = gtk_label_new("Text Scaling Factor:");
    gtk_widget_set_halign(lbl4, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl4, 0, 3, 1, 1);

    GtkWidget *sb = gtk_spin_button_new_with_range(0.5, 3.0, 0.01);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sb), gsettings.text_scaling_factor);
    g_signal_connect(sb, "value-changed", G_CALLBACK(on_text_scaling_changed), NULL);
    gtk_grid_attach(GTK_GRID(g), sb, 1, 3, 1, 1);

    preview = frame;
    gtk_grid_attach(grid, preview, 0, 1, 1, 1);
    
    gtk_widget_show_all(GTK_WIDGET(grid));
    gtk_widget_hide(GTK_WIDGET(scrolled_window));
}

static void on_toolbar_style_changed(GtkComboBoxText *combo, gpointer data) {
    g_free(gsettings.toolbar_style);
    gsettings.toolbar_style = g_strdup(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo)));
}

static void on_toolbar_icon_size_changed(GtkComboBoxText *combo, gpointer data) {
    g_free(gsettings.toolbar_icons_size);
    gsettings.toolbar_icons_size = g_strdup(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo)));
}

static void on_event_sounds_toggled(GtkToggleButton *btn, gpointer data) {
    gsettings.event_sounds = gtk_toggle_button_get_active(btn);
}

static void on_feedback_sounds_toggled(GtkToggleButton *btn, gpointer data) {
    gsettings.input_feedback_sounds = gtk_toggle_button_get_active(btn);
}

void display_other_settings_form(void) {
    destroy_content();
    GtkWidget *frame = gtk_frame_new("  Other Settings  ");
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    g_object_set(frame, "margin", 6, NULL);

    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(g), 12);
    gtk_grid_set_column_spacing(GTK_GRID(g), 12);
    g_object_set(g, "margin", 6, "hexpand", TRUE, "vexpand", TRUE, NULL);
    gtk_container_add(GTK_CONTAINER(frame), g);

    GtkWidget *lbl1 = gtk_label_new("Toolbar Style:");
    gtk_widget_set_halign(lbl1, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl1, 0, 1, 1, 1);

    GtkWidget *combo_style = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_style), "both", "Text below icons");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_style), "both-horiz", "Text next to icons");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_style), "icons", "Icons");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_style), "text", "Text");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo_style), gsettings.toolbar_style);
    g_signal_connect(combo_style, "changed", G_CALLBACK(on_toolbar_style_changed), NULL);
    gtk_grid_attach(GTK_GRID(g), combo_style, 1, 1, 1, 1);

    GtkWidget *lbl2 = gtk_label_new("Toolbar Icon Size:");
    gtk_widget_set_halign(lbl2, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g), lbl2, 0, 2, 1, 1);

    GtkWidget *combo_size = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_size), "small", "Small");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_size), "large", "Large");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo_size), gsettings.toolbar_icons_size);
    g_signal_connect(combo_size, "changed", G_CALLBACK(on_toolbar_icon_size_changed), NULL);
    gtk_grid_attach(GTK_GRID(g), combo_size, 1, 2, 1, 1);

    GtkWidget *cb1 = gtk_check_button_new_with_label("Enable Event Sounds");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb1), gsettings.event_sounds);
    g_signal_connect(cb1, "toggled", G_CALLBACK(on_event_sounds_toggled), NULL);
    gtk_grid_attach(GTK_GRID(g), cb1, 0, 4, 2, 1);

    GtkWidget *cb2 = gtk_check_button_new_with_label("Enable Input Feedback Sounds");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb2), gsettings.input_feedback_sounds);
    g_signal_connect(cb2, "toggled", G_CALLBACK(on_feedback_sounds_toggled), NULL);
    gtk_grid_attach(GTK_GRID(g), cb2, 0, 5, 2, 1);

    preview = frame;
    gtk_grid_attach(grid, preview, 0, 1, 1, 1);
    gtk_widget_show_all(GTK_WIDGET(grid));
    gtk_widget_hide(GTK_WIDGET(scrolled_window));
}


void display_program_settings_form(void) {
    destroy_content();

    GtkWidget *frame = gtk_frame_new("  Preferences  ");
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
    g_object_set(frame, "margin", 6, NULL);

    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(g), 12);
    gtk_grid_set_column_spacing(GTK_GRID(g), 12);
    g_object_set(g, "margin", 6, "hexpand", TRUE, "vexpand", TRUE, NULL);
    gtk_container_add(GTK_CONTAINER(frame), g);

    int row = 0;

    GtkWidget *lbl = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(lbl), "<b>Files to export</b>");
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(g), lbl, 0, row++, 1, 1);

    GtkWidget *cb1 = gtk_check_button_new_with_label("~/.config/gtk-3.0/settings.ini");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb1), preferences.export_settings_ini);
    // TODO: Connect signal
    gtk_grid_attach(GTK_GRID(g), cb1, 0, row++, 1, 1);

    GtkWidget *cb2 = gtk_check_button_new_with_label("~/.gtkrc-2.0");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb2), preferences.export_gtkrc_20);
    // TODO: Connect signal
    gtk_grid_attach(GTK_GRID(g), cb2, 0, row++, 1, 1);

    GtkWidget *cb3 = gtk_check_button_new_with_label("~/.icons/default/index.theme");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb3), preferences.export_index_theme);
    // TODO: Connect signal
    gtk_grid_attach(GTK_GRID(g), cb3, 0, row++, 1, 1);

    GtkWidget *cb4 = gtk_check_button_new_with_label("~/.config/xsettingsd/xsettingsd.conf");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb4), preferences.export_xsettingsd);
    // TODO: Connect signal
    gtk_grid_attach(GTK_GRID(g), cb4, 0, row++, 1, 1);

    GtkWidget *cb5 = gtk_check_button_new_with_label("~/.config/gtk-4.0/*");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb5), preferences.export_gtk4_symlinks);
    // TODO: Connect signal
    gtk_grid_attach(GTK_GRID(g), cb5, 0, row, 1, 1);

    GtkWidget *btn = gtk_button_new_with_label("Clear");
    // TODO: Connect clearGtk4Symlinks
    gtk_grid_attach(GTK_GRID(g), btn, 1, row++, 1, 1);

    preview = frame;
    gtk_grid_attach(grid, preview, 0, 1, 1, 1);
    
    // In Go code: menuBar.Deactivate() is used, we can omit it or use gtk_menu_shell_deactivate
    gtk_widget_show_all(GTK_WIDGET(grid));
    gtk_widget_hide(GTK_WIDGET(scrolled_window));
}
void destroy_content(void) {
    if (listbox) {
        gtk_widget_destroy(listbox);
        listbox = NULL;
    }
    if (preview) {
        gtk_widget_destroy(preview);
        preview = NULL;
    }
    if (theme_settings_selector) {
        gtk_widget_destroy(theme_settings_selector);
        theme_settings_selector = NULL;
    }
    if (cursor_size_selector) {
        gtk_widget_destroy(cursor_size_selector);
        cursor_size_selector = NULL;
    }
}
