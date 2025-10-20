#include "microui_impl_sdl3.h"

#define MUSDL3_MOUSE_INVALID -1
#define MUSDL3_KEY_INVALID -1
#define MUSDL3_X_SCROLL_FACTOR 30
#define MUSDL3_Y_SCROLL_FACTOR -30
#define MUSDL3_ICON_BORDER 7
#define MUSDL3_ICON_SIZE 24

typedef struct
{
    TTF_Font* font;
    TTF_Text* text;
    TTF_TextEngine* text_engine;
}musdl3_FontData;

int musdl3_text_width(mu_Font font, const char* text, int len)
{
    musdl3_FontData* font_data = font;
    int width;

    TTF_MeasureString(font_data->font, text, 0, 0, &width, NULL);
    return width;
}

int musdl3_text_height(mu_Font font)
{
    musdl3_FontData* font_data = font;

    return TTF_GetFontSize(font_data->font);
}

int musdl3_get_event_button(SDL_Event* event)
{
    switch (event->button.button)
    {
        case SDL_BUTTON_LEFT:
            return MU_MOUSE_LEFT;

        case SDL_BUTTON_RIGHT:
            return MU_MOUSE_RIGHT;

        case SDL_BUTTON_MIDDLE:
            return MU_MOUSE_MIDDLE;

        default:
            return MUSDL3_MOUSE_INVALID;
    }
}

int musdl3_get_event_key(SDL_Event* event)
{
    switch (event->key.scancode)
    {
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
            return MU_KEY_SHIFT;
        
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
            return MU_KEY_CTRL;

        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:
            return MU_KEY_ALT;

        case SDL_SCANCODE_BACKSPACE:
            return MU_KEY_BACKSPACE;

        case SDL_SCANCODE_RETURN:
            return MU_KEY_RETURN;

        default:
            return MUSDL3_KEY_INVALID;
    }
}

void musdl3_draw_icon(SDL_Renderer* renderer, mu_Command* command)
{
    mu_Rect rect = command->icon.rect;
    mu_Color color = command->icon.color;

    float scale_x = MUSDL3_ICON_SIZE / (float)command->icon.rect.w;
    float scale_y = MUSDL3_ICON_SIZE / (float)command->icon.rect.h;

    SDL_SetRenderScale(renderer, 1.0f / scale_x, 1.0f / scale_y);
    
    rect.x *= scale_x;
    rect.y *= scale_y;
    rect.w *= scale_x;
    rect.h *= scale_y;

    rect.x += MUSDL3_ICON_BORDER;
    rect.y += MUSDL3_ICON_BORDER;
    rect.w -= MUSDL3_ICON_BORDER * 2;
    rect.h -= MUSDL3_ICON_BORDER * 2;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    if (command->icon.id == MU_ICON_CLOSE)
    {
        SDL_RenderLine(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
        SDL_RenderLine(renderer, rect.x + rect.w, rect.y, rect.x, rect.y + rect.h);
    }
    else if (command->icon.id == MU_ICON_CHECK)
    {
        SDL_FRect render_rect = {rect.x, rect.y, rect.w, rect.h};
        SDL_RenderFillRect(renderer, &render_rect);
    }
    else if (command->icon.id == MU_ICON_COLLAPSED)
    {
        SDL_FPoint points[3] = {{rect.x, rect.y}, {rect.x + rect.w, rect.y + rect.h / 2.0f}, {rect.x, rect.y + rect.h}};
        SDL_RenderLines(renderer, points, 3);
    }
    else if (command->icon.id == MU_ICON_EXPANDED)
    {
        SDL_FPoint points[3] = {{rect.x, rect.y}, {rect.x + rect.w / 2.0f, rect.y + rect.h}, {rect.x + rect.w, rect.y}};
        SDL_RenderLines(renderer, points, 3);
    }

    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

void musdl3_destroy_font_data(musdl3_FontData* font_data)
{
    if (font_data)
    {
        if (font_data->text)
        {
            TTF_DestroyText(font_data->text);
        }
        if (font_data->text_engine)
        {
            TTF_DestroyRendererTextEngine(font_data->text_engine);
        }
        SDL_free(font_data);
    }
}

bool musdl3_set_font(mu_Context* ctx, SDL_Renderer* renderer, TTF_Font* font)
{
    if (!ctx || !renderer || !font) { return false; }

    musdl3_destroy_font_data(ctx->style->font);
    ctx->style->font = NULL;

    musdl3_FontData* font_data = SDL_calloc(1, sizeof(musdl3_FontData));
    if (!font_data) { goto error; }

    font_data->font = font;

    font_data->text_engine = TTF_CreateRendererTextEngine(renderer);
    if (!font_data->text_engine) { goto error; }

    font_data->text = TTF_CreateText(font_data->text_engine, font, "", 0);
    if (!font_data->text) { goto error; }

    ctx->style->font = font_data;

    return true;

error:
    musdl3_destroy_font_data(font_data);
    if (ctx->style) { SDL_free(ctx->style); ctx->style = NULL; }
    return false;
}

bool musdl3_init(mu_Context* ctx, SDL_Renderer* renderer, TTF_Font* font)
{
    if (!ctx || !renderer || !font) { return false; }

    ctx->style = SDL_calloc(1, sizeof(mu_Style));
    if (!ctx->style) { return false; }
    *ctx->style = ctx->_style;

    if (!musdl3_set_font(ctx, renderer, font)) { return false; }

    ctx->text_width = musdl3_text_width;
    ctx->text_height = musdl3_text_height;

    return true;
}

bool musdl3_set_scale(mu_Context* ctx, float scale)
{
    if (!ctx) { return false; }

    ctx->style->size.x = ctx->_style.size.x * scale;
    ctx->style->size.y = ctx->_style.size.y * scale;
    ctx->style->padding = ctx->_style.padding * scale;
    ctx->style->spacing = ctx->_style.spacing * scale;
    ctx->style->indent = ctx->_style.indent * scale;
    ctx->style->title_height = ctx->_style.title_height * scale;
    ctx->style->scrollbar_size = ctx->_style.scrollbar_size * scale;
    ctx->style->thumb_size = ctx->_style.thumb_size * scale;

    return true;
}

int musdl3_get_text_width(mu_Context* ctx, const char* text)
{
    return musdl3_text_width(ctx->style->font, text, 0);
}

void musdl3_quit(mu_Context* ctx)
{
    if (ctx && ctx->style)
    {
        if (ctx->style->font)
        {
            musdl3_FontData* font_data = ctx->style->font;
            musdl3_destroy_font_data(font_data);
        }
        SDL_free(ctx->style);
        ctx->style = NULL;
    }
}

void musdl3_process_event(mu_Context* ctx, SDL_Event* event)
{
    if (!ctx || !event) { return; }

    if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        mu_input_mousemove(ctx, event->motion.x, event->motion.y);
    }
    else if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        mu_input_scroll(ctx, event->wheel.x * MUSDL3_X_SCROLL_FACTOR * -1, event->wheel.y * MUSDL3_Y_SCROLL_FACTOR);
    }
    else if (event->type == SDL_EVENT_TEXT_INPUT)
    {
        mu_input_text(ctx, event->text.text);
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        mu_input_mousedown(ctx, event->button.x, event->button.y, musdl3_get_event_button(event));
    }
    else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        mu_input_mouseup(ctx, event->button.x, event->button.y, musdl3_get_event_button(event));
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        const bool* keys = SDL_GetKeyboardState(NULL);
        if (((keys[SDL_SCANCODE_LCTRL]) || (keys[SDL_SCANCODE_RCTRL])) && (event->key.scancode == SDL_SCANCODE_V))
        {
            char* text = SDL_GetClipboardText();
            mu_input_text(ctx, text);
            SDL_free(text);
        }
        else
        {
            int key = musdl3_get_event_key(event);

            if (key != MUSDL3_KEY_INVALID)
            {
                mu_input_keydown(ctx, key);
            }
        }
    }
    else if ((event->type == SDL_EVENT_KEY_UP) && (!event->key.repeat))
    {
        int key = musdl3_get_event_key(event);

        if (key != MUSDL3_KEY_INVALID)
        {
            mu_input_keyup(ctx, key);
        }
    }
}

void musdl3_render_commands(mu_Context* ctx, SDL_Renderer* renderer)
{
    if (!ctx || !ctx->style->font || !renderer) { return; }
    
    SDL_Window* window = SDL_GetRenderWindow(renderer);
    if (window)
    {
        if ((ctx->textbox_focused) && (!SDL_TextInputActive(window)))
        {
            SDL_StartTextInput(window);
        }
    }

    mu_Command* command = NULL;
    while (mu_next_command(ctx, &command))
    {
        if (command->type == MU_COMMAND_TEXT)
        {
            musdl3_FontData* font_data = ctx->style->font;
            TTF_SetTextString(font_data->text, command->text.str, 0);
            TTF_DrawRendererText(font_data->text, command->text.pos.x, command->text.pos.y);
        }
        else if (command->type == MU_COMMAND_RECT)
        {
            SDL_FRect rect = {command->rect.rect.x, command->rect.rect.y, command->rect.rect.w, command->rect.rect.h};
            SDL_SetRenderDrawColor(renderer, command->rect.color.r, command->rect.color.g, command->rect.color.b, command->rect.color.a);
            SDL_RenderFillRect(renderer, &rect);
        }
        else if (command->type == MU_COMMAND_ICON)
        {
            musdl3_draw_icon(renderer, command);
        }
        else if (command->type == MU_COMMAND_CLIP)
        {
            if ((command->clip.rect.w > 0) && (command->clip.rect.h > 0))
            {
                SDL_Rect rect = {command->clip.rect.x, command->clip.rect.y, command->clip.rect.w, command->clip.rect.h};
                SDL_SetRenderClipRect(renderer, &rect);
            }
        }
    }
}
