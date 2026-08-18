#include "platform/input.h"
#include "mrnutz/controls.h"
#include "common_rtl.h"

#include <string.h>
#include <stdio.h>

typedef struct { const char *name; int value; } NameVal;

static const NameVal kKeyNameTable[] = {
  {"A",SDL_SCANCODE_A},{"B",SDL_SCANCODE_B},{"C",SDL_SCANCODE_C},{"D",SDL_SCANCODE_D},
  {"E",SDL_SCANCODE_E},{"F",SDL_SCANCODE_F},{"G",SDL_SCANCODE_G},{"H",SDL_SCANCODE_H},
  {"I",SDL_SCANCODE_I},{"J",SDL_SCANCODE_J},{"K",SDL_SCANCODE_K},{"L",SDL_SCANCODE_L},
  {"M",SDL_SCANCODE_M},{"N",SDL_SCANCODE_N},{"O",SDL_SCANCODE_O},{"P",SDL_SCANCODE_P},
  {"Q",SDL_SCANCODE_Q},{"R",SDL_SCANCODE_R},{"S",SDL_SCANCODE_S},{"T",SDL_SCANCODE_T},
  {"U",SDL_SCANCODE_U},{"V",SDL_SCANCODE_V},{"W",SDL_SCANCODE_W},{"X",SDL_SCANCODE_X},
  {"Y",SDL_SCANCODE_Y},{"Z",SDL_SCANCODE_Z},
  {"SPACE",SDL_SCANCODE_SPACE},{"RETURN",SDL_SCANCODE_RETURN},{"ENTER",SDL_SCANCODE_RETURN},
  {"LSHIFT",SDL_SCANCODE_LSHIFT},{"RSHIFT",SDL_SCANCODE_RSHIFT},
  {"LCTRL",SDL_SCANCODE_LCTRL},{"RCTRL",SDL_SCANCODE_RCTRL},
  {"LALT",SDL_SCANCODE_LALT},{"RALT",SDL_SCANCODE_RALT},{"TAB",SDL_SCANCODE_TAB},
  {"UP",SDL_SCANCODE_UP},{"DOWN",SDL_SCANCODE_DOWN},
  {"LEFT",SDL_SCANCODE_LEFT},{"RIGHT",SDL_SCANCODE_RIGHT},
  {NULL,0}
};

static const NameVal kGamepadBtnTable[] = {
  {"DPAD_UP",SDL_GAMEPAD_BUTTON_DPAD_UP},
  {"DPAD_DOWN",SDL_GAMEPAD_BUTTON_DPAD_DOWN},
  {"DPAD_LEFT",SDL_GAMEPAD_BUTTON_DPAD_LEFT},
  {"DPAD_RIGHT",SDL_GAMEPAD_BUTTON_DPAD_RIGHT},
  {"A",SDL_GAMEPAD_BUTTON_SOUTH},
  {"B",SDL_GAMEPAD_BUTTON_EAST},
  {"X",SDL_GAMEPAD_BUTTON_WEST},
  {"Y",SDL_GAMEPAD_BUTTON_NORTH},
  {"BACK",SDL_GAMEPAD_BUTTON_BACK},
  {"START",SDL_GAMEPAD_BUTTON_START},
  {"LEFTSHOULDER",SDL_GAMEPAD_BUTTON_LEFT_SHOULDER},
  {"RIGHTSHOULDER",SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER},
  {NULL,0}
};

static const NameVal kButtonBits[] = {
  {"B",SN_B},{"Y",SN_Y},{"A",SN_A},{"X",SN_X},{"L",SN_L},{"R",SN_R},
  {"Select",SN_SELECT},{"Start",SN_START},
  {"Up",SN_UP},{"Down",SN_DOWN},{"Left",SN_LEFT},{"Right",SN_RIGHT},
  {NULL,0}
};

static int FindNameVal(const NameVal *table, const char *name) {
  for (int i = 0; table[i].name; i++)
    if (strcasecmp(name, table[i].name) == 0) return table[i].value;
  return -1;
}

#define MAX_BINDS 4
typedef struct { int count; int values[MAX_BINDS]; } BindList;

static BindList g_kb_binds[12];
static BindList g_pad_binds[12];
static SDL_Gamepad *g_gamepad = NULL;

uint32 g_input_state = 0;
bool g_paused = false;
bool g_turbo = false;
static int frame_count_global = 0;

static void ParseBindList(const char *str, const NameVal *table, BindList *out) {
  out->count = 0;
  char buf[256];
  strncpy(buf, str, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0';
  char *tok = strtok(buf, ", \t");
  while (tok && out->count < MAX_BINDS) {
    int v = FindNameVal(table, tok);
    if (v >= 0) out->values[out->count++] = v;
    tok = strtok(NULL, ", \t");
  }
}

bool Input_Init(const char *config_file) {
  int def_kb[12] = {SDL_SCANCODE_RIGHT,SDL_SCANCODE_LEFT,SDL_SCANCODE_SPACE,
    SDL_SCANCODE_S,SDL_SCANCODE_Q,SDL_SCANCODE_W,SDL_SCANCODE_DOWN,
    SDL_SCANCODE_RETURN,SDL_SCANCODE_UP,SDL_SCANCODE_E,SDL_SCANCODE_A,SDL_SCANCODE_D};
  int def_pad[12] = {SDL_GAMEPAD_BUTTON_EAST,SDL_GAMEPAD_BUTTON_NORTH,SDL_GAMEPAD_BUTTON_SOUTH,
    SDL_GAMEPAD_BUTTON_WEST,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_BACK,SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_DPAD_UP,SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,SDL_GAMEPAD_BUTTON_DPAD_RIGHT};
  for (int i = 0; i < 12; i++) {
    g_kb_binds[i].count = 1; g_kb_binds[i].values[0] = def_kb[i];
    g_pad_binds[i].count = 1; g_pad_binds[i].values[0] = def_pad[i];
  }

  FILE *f = fopen(config_file, "r");
  if (!f) { fprintf(stderr, "[config] %s not found, using defaults\n", config_file); return true; }
  fprintf(stderr, "[config] loading %s\n", config_file);

  int section = 0;
  char line[256];
  while (fgets(line, sizeof(line), f)) {
    char *p = line; while (*p==' '||*p=='\t') p++;
    if (*p=='#'||*p=='\n'||*p=='\r'||*p=='\0') continue;
    if (*p=='[') {
      if (strncasecmp(p,"[Keyboard]",10)==0) section=1;
      else if (strncasecmp(p,"[Gamepad]",9)==0) section=2;
      else section=0;
      continue;
    }
    char *eq = strchr(p,'='); if (!eq) continue;
    *eq='\0'; char *key=eq+1; while(*key==' ') key++;
    char *nl=strpbrk(key,"\n\r"); if(nl) *nl='\0';
    char *sp=strchr(p,'\0')-1; while(sp>p&&(*sp==' '||*sp=='\t')) {*sp='\0';sp--;}
    while(*p==' ') p++;
    for (int i=0; kButtonBits[i].name; i++) {
      if (strcasecmp(p, kButtonBits[i].name)==0) {
        if (section==1) ParseBindList(key, kKeyNameTable, &g_kb_binds[i]);
        else if (section==2) ParseBindList(key, kGamepadBtnTable, &g_pad_binds[i]);
        break;
      }
    }
  }
  fclose(f);
  return true;
}

void Input_HandleEvent(int key, bool pressed) {
  switch (key) {
    case SDLK_p: g_paused = pressed; return;
    case SDLK_TAB: g_turbo = pressed; return;
    case SDLK_f: /* FPS toggle handled in overlay */ return;
    case SDLK_F1: if (pressed) RtlSaveLoad(1, 0); return;
    case SDLK_F2: if (pressed) RtlSaveLoad(2, 0); return;
  }
}

void Input_Poll(void) {
  g_input_state = 0;

  const bool *keys = SDL_GetKeyboardState(NULL);
  for (int i = 0; i < 12; i++)
    for (int j = 0; j < g_kb_binds[i].count; j++)
      if (keys[g_kb_binds[i].values[j]]) { g_input_state |= kButtonBits[i].value; break; }

  if (g_gamepad) {
    for (int i = 0; i < 12; i++)
      for (int j = 0; j < g_pad_binds[i].count; j++)
        if (SDL_GetGamepadButton(g_gamepad, (SDL_GamepadButton)g_pad_binds[i].values[j])) {
          g_input_state |= kButtonBits[i].value; break;
        }
  }

  if (g_input_state & SN_START)
    g_input_state |= SN_START_WORKAROUND;

  frame_count_global++;
}

void Input_OpenGamepad(void) {
  int n = 0;
#if SNESRECOMP_SDL3
  SDL_JoystickID *gamepads = SDL_GetGamepads(&n);
  if (n > 0) {
    g_gamepad = SDL_OpenGamepad(gamepads[0]);
    if (g_gamepad)
      fprintf(stderr, "[gamepad] opened: %s\n", SDL_GetGamepadName(g_gamepad));
  }
  SDL_free(gamepads);
#else
  n = SDL_NumJoysticks();
  if (n > 0 && SDL_IsGameController(0)) {
    g_gamepad = (SDL_Gamepad*)SDL_GameControllerOpen(0);
    if (g_gamepad) fprintf(stderr, "[gamepad] opened: %s\n", SDL_GameControllerName(0));
  }
#endif
}
