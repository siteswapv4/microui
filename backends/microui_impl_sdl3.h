#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "microui.h"

bool musdl3_init(mu_Context* ctx, SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font);

void musdl3_quit(mu_Context* ctx);

bool musdl3_set_scale(mu_Context* ctx, float scale);

bool musdl3_set_font(mu_Context* ctx, SDL_Renderer* renderer, TTF_Font* font);

int musdl3_get_text_width(mu_Context* ctx, const char* text);

void musdl3_process_event(mu_Context* ctx, SDL_Event* event);

void musdl3_render_commands(mu_Context* ctx, SDL_Renderer* renderer);