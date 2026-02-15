#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#include <stdbool.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define ROWS 32

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static int text_begin = 0;
static int *buffer_start = NULL;
static int buffer_end = 0;
static int *text_end = NULL;
int line = 0;
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("text editor", "1.0", "com.example.CATEGORY-NAME");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("text editor", 640, 480,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, 640, 480,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

static char editor_buffer[1024] = "Type something...";

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  // Catch actual text characters
  else if (event->type == SDL_EVENT_KEY_DOWN) {
    const char key = event->key.key;

    int length = strlen(editor_buffer);

    if (length < sizeof(editor_buffer) - 1) {
      editor_buffer[length] = key;      // Place char at the end
      editor_buffer[length + 1] = '\0'; // Manually re-terminate
    }
    if (length >= 20) {
      line++;
    }
    SDL_Log("Buffer is now: %s", editor_buffer);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  // 1. Clear Screen
  SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
  SDL_RenderClear(renderer);

  // 2. Render Text
  // Note: SDL_RenderDebugText uses grid coordinates, not pixels.
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderDebugText(renderer, 0, line, editor_buffer);

  // 3. Present
  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}
/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
}
