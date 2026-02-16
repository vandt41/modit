#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINES 1024
#define MAX_LINE_LENGTH 1024
#define FONT_SIZE 20
#define OFFSET_Y 20
#define OFFSET_X 10

/* Global state */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL; // Make font global
static char buffer[MAX_LINES][MAX_LINE_LENGTH];
static int current_line = 0;
static int current_col = 0;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("text editor", "1.0", "com.example.texteditor");

  // Initialize all lines to empty
  for (int i = 0; i < MAX_LINES; i++) {
    buffer[i][0] = '\0';
  }

  // Set initial text
  strcpy(buffer[current_line++], "Text under here!");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("text editor", 800, 600,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!TTF_Init()) {
    SDL_Log("TTF_Init Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Load font (make it global)
  font = TTF_OpenFont(
      "/usr/local/share/fonts/JetBrainsMono/JetBrainsMonoNerdFont-Regular.ttf",
      FONT_SIZE);
  if (!font) {
    SDL_Log("Couldn't initialize font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_StartTextInput(window)) {
    SDL_Log("Couldn't start text input: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }

  else if (event->type == SDL_EVENT_KEY_DOWN) {
    switch (event->key.key) {
    case SDLK_RETURN:
      SDL_Log("Enter pressed!");
      current_line++;
      current_col = 0;
      if (current_line >= MAX_LINES) {
        current_line = MAX_LINES - 1; // Prevent overflow
      }
      break;

    case SDLK_BACKSPACE:
      if (current_col > 0) {
        // Remove last character
        int len = strlen(buffer[current_line]);
        if (len > 0) {
          buffer[current_line][len - 1] = '\0';
          current_col--;
        }
      }
      break;
    }
  }
  // Catch actual text characters
  else if (event->type == SDL_EVENT_TEXT_INPUT) {
    const char *input = event->text.text;
    int current_len = strlen(buffer[current_line]);

    // Check if we have space
    if (current_len + strlen(input) < MAX_LINE_LENGTH - 1) {
      strcat(buffer[current_line], input);
      current_col += strlen(input);
    }

    SDL_Log("Line %d: '%s'", current_line, buffer[current_line]);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  // Clear screen
  SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
  SDL_RenderClear(renderer);

  // Render all lines of text using SDL_ttf
  int y_offset = OFFSET_Y;
  int x_offset;
  for (int i = 0; i <= current_line; i++) {
    if (buffer[i][0] != '\0') { // Only render non-empty lines
      SDL_Color white = {255, 255, 255, 255};

      // Render text to surface
      SDL_Surface *surface =
          TTF_RenderText_Blended(font, buffer[i], strlen(buffer[i]), white);
      if (surface) {
        // Convert surface to texture
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
          // Get texture dimensions
          float w, h;
          SDL_GetTextureSize(texture, &w, &h);

          // Set destination rectangle
          SDL_FRect dst_rect = {OFFSET_X, (float)y_offset, w, h};

          // Render the texture
          SDL_RenderTexture(renderer, texture, NULL, &dst_rect);

          // Cleanup
          SDL_DestroyTexture(texture);
        }
        SDL_DestroySurface(surface);
      }

      // Move to next line position
      y_offset += FONT_SIZE + 4;
    }
  }

  // Simple cursor rendering (just a visual indicator)
  if (SDL_GetTicks() % 1000 < 500) { // Blink every 500ms
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Calculate cursor position (rough estimate)
    int cursor_x = FONT_SIZE * (current_col + OFFSET_X);
    int cursor_y = y_offset - FONT_SIZE;

    SDL_FRect cursor = {cursor_x, cursor_y, 2, FONT_SIZE};
    SDL_RenderFillRect(renderer, &cursor);
  }

  // Present
  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  SDL_StopTextInput(window);
  if (font) {
    TTF_CloseFont(font);
  }
  TTF_Quit();
  // SDL will clean up the window/renderer for us
}
