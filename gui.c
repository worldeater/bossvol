#include <stdlib.h>
#include <gtk/gtk.h>

#include "gui.h"


#define TOOLTIP_MAX    64
#define POLL_INTERVAL  250  /* in milliseconds */

enum volicon_index {
  volicon_muted,
  volicon_low,
  volicon_medium,
  volicon_high,
  volicon_max,
};


struct ui {
  struct mixerdev *mixerdev;
  GtkStatusIcon   *trayobj;
  GtkWidget       *menu;
  GtkWidget       *menu_info;
  GtkWidget       *menu_sep;
  GtkWidget       *menu_exit;
  GIcon           *vol_icon[volicon_max];
  char             tooltip[TOOLTIP_MAX];
};


static gboolean trayobj_onclick(GtkStatusIcon *obj __unused, GdkEventButton *restrict ev, struct ui *restrict ui);
static gboolean trayobj_onscroll(GtkStatusIcon *obj __unused, GdkEventScroll *restrict ev, struct ui *restrict ui);
static gboolean trayobj_update(struct ui *restrict ui);


static gboolean
trayobj_onclick(GtkStatusIcon *obj __unused, GdkEventButton *restrict ev, struct ui *restrict ui)
{
  enum { LEFT = 1, MIDDLE, RIGHT };

  switch (ev->type) {
  case GDK_BUTTON_PRESS:
    switch (ev->button) {
    case MIDDLE:
      mixerdev_toggle_mute(ui->mixerdev);
      goto handled;
    case RIGHT:
      gtk_menu_popup(GTK_MENU(ui->menu), NULL, NULL, NULL, NULL, ev->button, ev->time);
      goto handled;
    default:
      break;
    }
  default:
    break;
  }
  return FALSE;

handled:
  trayobj_update(ui);
  return TRUE;
}


static gboolean
trayobj_onscroll(GtkStatusIcon *obj __unused, GdkEventScroll *restrict ev, struct ui *restrict ui)
{
  switch (ev->direction) {
  case GDK_SCROLL_UP:
    mixerdev_inc_vol(ui->mixerdev);
    goto handled;
  case GDK_SCROLL_DOWN:
    mixerdev_dec_vol(ui->mixerdev);
    goto handled;
  default:
    break;
  }
  return FALSE;

handled:
  trayobj_update(ui);
  return TRUE;
}


static gboolean
trayobj_update(struct ui *restrict ui)
{
  static int lastvol = -1; /* -1 enforces an update on the very first call */
  const char *mixer;
  const char *device;
  int idx;
  int vol;

  vol = mixerdev_get_vol(ui->mixerdev);

  if (lastvol != vol) {
    if (vol <= 0)  idx = volicon_muted;  else
    if (vol <= 33) idx = volicon_low;    else
    if (vol <= 66) idx = volicon_medium; else
                   idx = volicon_high;
    gtk_status_icon_set_from_gicon(ui->trayobj, ui->vol_icon[idx]);

    mixer = mixerdev_get_mixer_name(ui->mixerdev);
    device = mixerdev_get_dev_name(ui->mixerdev);
    snprintf(ui->tooltip, TOOLTIP_MAX, "%s - %s: %d%%", mixer, device, vol);
    gtk_status_icon_set_tooltip_text(ui->trayobj, ui->tooltip);

    lastvol = vol;
  }

  return TRUE;
}


void
trayobj_init(struct mixerdev *md)
{
  extern const char *const __progname;
  struct ui ui;

  ui.mixerdev = md;

  ui.vol_icon[volicon_muted]  = g_icon_new_for_string("audio-volume-muted",  NULL);
  ui.vol_icon[volicon_low]    = g_icon_new_for_string("audio-volume-low",    NULL);
  ui.vol_icon[volicon_medium] = g_icon_new_for_string("audio-volume-medium", NULL);
  ui.vol_icon[volicon_high]   = g_icon_new_for_string("audio-volume-high",   NULL);

  ui.trayobj = gtk_status_icon_new();

  ui.menu = gtk_menu_new();
  ui.menu_info = gtk_menu_item_new_with_label(__progname);
  ui.menu_sep  = gtk_separator_menu_item_new();
  ui.menu_exit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);

  gtk_status_icon_set_name(ui.trayobj, __progname);
  gtk_status_icon_set_title(ui.trayobj, __progname);

  gtk_widget_set_sensitive(ui.menu_info, 0);

  gtk_menu_shell_append(GTK_MENU_SHELL(ui.menu), ui.menu_info);
  gtk_menu_shell_append(GTK_MENU_SHELL(ui.menu), ui.menu_sep);
  gtk_menu_shell_append(GTK_MENU_SHELL(ui.menu), ui.menu_exit);

  gtk_widget_show(ui.menu_info);
  gtk_widget_show(ui.menu_sep);
  gtk_widget_show(ui.menu_exit);

  trayobj_update(&ui);

  g_signal_connect(GTK_STATUS_ICON(ui.trayobj), "scroll-event", (GCallback)trayobj_onscroll, &ui);
  g_signal_connect(GTK_STATUS_ICON(ui.trayobj), "button-press-event", (GCallback)trayobj_onclick, &ui);
  g_signal_connect(GTK_MENU_ITEM(ui.menu_exit), "activate", (GCallback)gtk_main_quit, NULL);
  g_timeout_add(POLL_INTERVAL, (GSourceFunc)trayobj_update, &ui); /* poll for external volume changes */

  gtk_main();
}

