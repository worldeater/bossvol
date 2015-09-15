#include <sys/soundcard.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "mixerdev.h"


#define  VOLSTEP  5


static char const *const devnames[SOUND_MIXER_NRDEVICES] = SOUND_DEVICE_NAMES;


struct mixerdev {
  char *mixer;
  char *device;
  int   index;
  int   oldvol;
  int   fd;
};


int
mixerdev_get_vol(struct mixerdev const *md)
{
  int raw = 0;
  int left, right;

  if (ioctl(md->fd, MIXER_READ(md->index), &raw) == -1)
    warn("ioctl(MIXER_READ(%d))", md->index);
  left  = (raw >> 0) & 0x7f;
  right = (raw >> 8) & 0x7f;
  return left > right ? left : right;
}


void
mixerdev_set_vol(struct mixerdev const *md, int vol)
{
  int raw;
  int left, right;

  if (vol < 0)
    vol = 0;
  else if (vol > 100)
    vol = 100;
  left = right = vol;
  raw = left | (right << 8);
  if (ioctl(md->fd, MIXER_WRITE(md->index), &raw) == 1)
    warn("ioctl(MIXER_WRITE(%d), %d)", md->index, raw);
}


void
mixerdev_inc_vol(struct mixerdev const *md)
{
  int vol = mixerdev_get_vol(md);
  vol += VOLSTEP;
  mixerdev_set_vol(md, vol);
}

void
mixerdev_dec_vol(struct mixerdev const *md)
{
  int vol = mixerdev_get_vol(md);
  vol -= VOLSTEP;
  mixerdev_set_vol(md, vol);
}


void
mixerdev_toggle_mute(struct mixerdev *md)
{
  int vol = mixerdev_get_vol(md);
  if (vol > 0) {
    md->oldvol = vol;
    mixerdev_set_vol(md, 0);
  } else {
    if (md->oldvol > 0) {
      mixerdev_set_vol(md, md->oldvol);
      md->oldvol = 0;
    }
  }
}


const char*
mixerdev_get_mixer_name(struct mixerdev const *md)
{
  return md->mixer;
}


const char*
mixerdev_get_dev_name(struct mixerdev const *md)
{
  return md->device;
}


struct mixerdev*
mixerdev_new(char const *mixer, char const *device)
{
  struct mixerdev *md;
  int devmask = 0;

  md = calloc(1, sizeof *md);
  if (md == NULL)
    err(EX_UNAVAILABLE, "calloc");

  if ((md->fd = open(mixer, O_RDWR)) == -1)
    err(EX_UNAVAILABLE, "open(%s)", mixer);

  if (ioctl(md->fd, SOUND_MIXER_READ_DEVMASK, &devmask) == -1)
    err(EX_IOERR, "ioctl(SOUND_MIXER_READ_DEVMASK)");

  for (int i = 0; i < SOUND_MIXER_NRDEVICES; ++i) {
    if (strcmp(device, devnames[i]) == 0) {
      int available = devmask & (1 << i);
      if (!available)
        break;
      md->index = i;
      if ((md->mixer = strdup(mixer)) == NULL)
        err(EX_UNAVAILABLE, "strdup");
      if ((md->device = strdup(device)) == NULL)
        err(EX_UNAVAILABLE, "strdup");
      goto done;
    }
  }
  errx(EX_UNAVAILABLE, "%s %s: not available", mixer, device);
done:
  return md;
}


void
mixerdev_del(struct mixerdev *md)
{
  close(md->fd);
  free(md->mixer);
  free(md->device);
  free(md);
}

