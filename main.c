#include <err.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "gui.h"


static struct {
  char *mixer;
  char *device;
} opt;


static void _Noreturn
usage(void)
{
  extern const char *const __progname;
  fprintf(stderr,
    "Usage: %s [-d mixerdev] [-m devnode]\n"
    "Defaults: %s -d %s -m %s\n",
    __progname,
    __progname, opt.device, opt.mixer);
  exit(EX_USAGE);
}


int
main(int argc, char **argv)
{
  opt.mixer = "/dev/mixer";
  opt.device = "vol";

  struct mixerdev *md;
  int ch;

  opterr = 0;
  while ((ch = getopt(argc, argv, ":d:m")) != -1) {
    switch(ch) {
    case 'd': opt.device = optarg; break;
    case 'm': opt.mixer  = optarg; break;
    case '?':   /* FALLTHROUGH */
    default:  usage();
    }
  }

  md = mixerdev_new(opt.mixer, opt.device);

  if (gtk_init_check(&argc, &argv) == 0)
    err(69, "gtk init failed");

  if (fork()) /* detach from terminal */
    exit(0);

  trayobj_init(md);
  mixerdev_del(md);
}

