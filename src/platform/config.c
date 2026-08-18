#include "platform/config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

AppConfig g_app_config = {
  .window_scale = 3,
  .disable_frame_delay = true,
  .skip_launcher = true,
  .fullscreen = false,
  .enable_audio = true,
  .audio_freq = 44100,
  .audio_channels = 2,
  .audio_samples = 512,
  .music_mode = 0,
  .music_volume = 80,
  .sfx_volume = 100,
};

static bool ParseBool(const char *val) {
  return val[0] == '1' || strncasecmp(val, "true", 4) == 0 ||
         strncasecmp(val, "yes", 3) == 0;
}

void Config_Load(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) return;
  char line[256];
  int section = 0;
  while (fgets(line, sizeof(line), f)) {
    char *p = line; while (*p==' '||*p=='\t') p++;
    if (*p=='#'||*p=='\n'||*p=='\r'||*p=='\0') continue;
    if (*p=='[') {
      if (strncasecmp(p,"[General]",8)==0) section=1;
      else if (strncasecmp(p,"[Graphics]",10)==0) section=2;
      else if (strncasecmp(p,"[Sound]",7)==0) section=3;
      else if (strncasecmp(p,"[Music]",7)==0) section=4;
      else section=0;
      continue;
    }
    char *eq = strchr(p,'='); if (!eq) continue;
    *eq='\0'; char *val=eq+1; while(*val==' ') val++;
    char *nl=strpbrk(val,"\n\r"); if(nl) *nl='\0';
    char *sp=strchr(p,'\0')-1; while(sp>p&&(*sp==' '||*sp=='\t')) {*sp='\0';sp--;}
    while(*p==' ') p++;

    if (section==1) { /* General */
      if (strcasecmp(p,"WindowScale")==0) g_app_config.window_scale = atoi(val);
      else if (strcasecmp(p,"DisableFrameDelay")==0) g_app_config.disable_frame_delay = ParseBool(val);
      else if (strcasecmp(p,"SkipLauncher")==0) g_app_config.skip_launcher = ParseBool(val);
    } else if (section==2) { /* Graphics */
      if (strcasecmp(p,"Fullscreen")==0) g_app_config.fullscreen = ParseBool(val);
    } else if (section==3) { /* Sound */
      if (strcasecmp(p,"EnableAudio")==0) g_app_config.enable_audio = ParseBool(val);
      else if (strcasecmp(p,"AudioFreq")==0) g_app_config.audio_freq = atoi(val);
    } else if (section==4) { /* Music */
      if (strcasecmp(p,"MusicMode")==0) {
        if (strcasecmp(val,"MP3-only")==0) g_app_config.music_mode = 1;
        else if (strcasecmp(val,"Mixed")==0) g_app_config.music_mode = 2;
        else g_app_config.music_mode = 0;
      }
      else if (strcasecmp(p,"MusicVolume")==0) g_app_config.music_volume = atoi(val);
      else if (strcasecmp(p,"SfxVolume")==0) g_app_config.sfx_volume = atoi(val);
    }
  }
  fclose(f);
}
