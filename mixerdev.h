#ifndef MIXERDEV_H_
#define MIXERDEV_H_

struct mixerdev;

struct mixerdev* mixerdev_new(char const *mixer, char const *device);
void mixerdev_del(struct mixerdev *md);
int mixerdev_get_vol(struct mixerdev const *md);
void mixerdev_set_vol(struct mixerdev const *md, int vol);
void mixerdev_inc_vol(struct mixerdev const *md);
void mixerdev_dec_vol(struct mixerdev const *md);
void mixerdev_toggle_mute(struct mixerdev *md);
const char* mixerdev_get_mixer_name(struct mixerdev const *md);
const char* mixerdev_get_dev_name(struct mixerdev const *md);

#endif
