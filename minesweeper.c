#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 680
#define WINDOW_HEIGHT 760
#define GRID_SIZE 14
#define CELL_SIZE 40
#define NUM_MINES_EASY 20
#define NUM_MINES_MEDIUM 30
#define NUM_MINES_HARD 40
#define INFO_HEIGHT 100
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 30
#define SHADOW_OFFSET 3

typedef enum { EASY, MEDIUM, HARD } Difficulty;

// Structure for each cell in the grid
typedef struct {
    bool revealed;
    bool mine;
    bool flagged;
    char neighbor_mines;
} Cell;

// Main game state structure
typedef struct {
    Cell grid[GRID_SIZE][GRID_SIZE];
    bool first_click;
    bool game_over;
    bool victory;
    time_t start_time;
    int elapsed_time;
    int score;
    int high_score;
    int num_mines;
    int flagged_mines;
    Difficulty difficulty;
    TTF_Font* font;
    int offset_x;
    int offset_y;
    bool hint_used;
    bool explosion_flash;
    int flash_timer;
} GameState;

// Initialize the game state
static void init_game(GameState* state, Difficulty diff) {
    state->difficulty = diff;
    state->num_mines = diff == EASY ? NUM_MINES_EASY : diff == MEDIUM ? NUM_MINES_MEDIUM : NUM_MINES_HARD;
    state->offset_x = (WINDOW_WIDTH - GRID_SIZE * CELL_SIZE) / 2;
    state->offset_y = INFO_HEIGHT;
    state->first_click = true;
    state->game_over = false;
    state->victory = false;
    state->score = 0;
    state->elapsed_time = 0;
    state->flagged_mines = 0;
    state->hint_used = false;
    state->explosion_flash = false;
    state->flash_timer = 0;
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        state->grid[i / GRID_SIZE][i % GRID_SIZE] = (Cell){false, false, false, 0};
    }
}

// Place mines on the grid
static void place_mines(GameState* state, int safe_x, int safe_y) {
    int placed = 0;
    while (placed < state->num_mines) {
        int x = rand() % GRID_SIZE, y = rand() % GRID_SIZE;
        if (!state->grid[x][y].mine && (x != safe_x || y != safe_y)) {
            state->grid[x][y].mine = true;
            placed++;
        }
    }
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (state->grid[x][y].mine) continue;
            char mines = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE && state->grid[nx][ny].mine) mines++;
                }
            }
            state->grid[x][y].neighbor_mines = mines;
        }
    }
}

// Recursively reveal cells
static void reveal_cell(GameState* state, int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE || state->grid[x][y].revealed || state->grid[x][y].flagged) return;
    state->grid[x][y].revealed = true;
    state->score += 10; // Increment score by 10 for each correct reveal
    if (!state->grid[x][y].mine && !state->grid[x][y].neighbor_mines) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx || dy) reveal_cell(state, x + dx, y + dy);
            }
        }
    }
}

// Check if the player has won
static bool check_win(const GameState* state) {
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        int x = i / GRID_SIZE, y = i % GRID_SIZE;
        if (!state->grid[x][y].mine && !state->grid[x][y].revealed) return false;
    }
    return true;
}

// Reveal a safe cell as a hint
static void use_hint(GameState* state) {
    if (state->hint_used || state->game_over || state->victory) return;
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        int x = i / GRID_SIZE, y = i % GRID_SIZE;
        if (!state->grid[x][y].revealed && !state->grid[x][y].mine && !state->grid[x][y].flagged) {
            reveal_cell(state, x, y);
            state->hint_used = true;
            return;
        }
    }
}

// Draw text with texture caching
static void draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color, TTF_Font* font, SDL_Texture** texture) {
    if (*texture) {
        SDL_DestroyTexture(*texture);
        *texture = NULL;
    }
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (surface) *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    int tw, th;
    TTF_SizeText(font, text, &tw, &th);
    SDL_Rect dst = {x, y, tw, th};
    SDL_RenderCopy(renderer, *texture, NULL, &dst);
}

// Draw a button with shadow and hover effect
static void draw_button(SDL_Renderer* renderer, int x, int y, int w, int h, const char* text, TTF_Font* font, SDL_Texture** texture, bool hovered) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
    SDL_Rect shadow = {x + SHADOW_OFFSET, y + SHADOW_OFFSET, w, h};
    SDL_RenderFillRect(renderer, &shadow);
    SDL_SetRenderDrawColor(renderer, hovered ? 220 : 192, 192, 192, 255);
    SDL_Rect btn = {x, y, w, h};
    SDL_RenderFillRect(renderer, &btn);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &btn);
    int tw, th;
    TTF_SizeText(font, text, &tw, &th);
    int text_x = x + (w - tw) / 2;
    int text_y = y + (h - th) / 2 - 2;
    draw_text(renderer, text_x, text_y, text, (SDL_Color){0,0,0,255}, font, texture);
}

// Draw a yellow "Reessayer" button with perfect fit
static void draw_reessayer_button(SDL_Renderer* renderer, int x, int y, int w, int h, TTF_Font* font, SDL_Texture** texture, bool hovered) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
    SDL_Rect shadow = {x + SHADOW_OFFSET, y + SHADOW_OFFSET, w, h};
    SDL_RenderFillRect(renderer, &shadow);
    SDL_SetRenderDrawColor(renderer, hovered ? 255 : 220, 220, 0, 255);
    SDL_Rect btn = {x, y, w, h};
    SDL_RenderFillRect(renderer, &btn);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &btn);
    int tw, th;
    TTF_SizeText(font, " REPEAT ", &tw, &th);
    int text_x = x + (w - tw) / 2; // Center horizontally
    int text_y = y + (h - th) / 2; // Center vertically with precise adjustment
    draw_text(renderer, text_x, text_y, " REPEAT ", (SDL_Color){0,0,0,255}, font, texture);
}

// Draw the game grid
static void draw_grid(SDL_Renderer* renderer, GameState* state) {
    static const SDL_Color colors[9] = { {0,0,0,255}, {0,0,255,255}, {0,128,0,255}, {255,0,0,255}, {0,0,128,255}, {128,0,0,255}, {0,128,128,255}, {0,0,0,255}, {128,128,128,255} };
    static SDL_Texture* num_textures[9] = {0}, *mine_texture = 0, *flag_texture = 0;

    SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
    SDL_Rect grid_rect = {state->offset_x, state->offset_y, GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE};
    SDL_RenderFillRect(renderer, &grid_rect);

    if (state->explosion_flash && state->flash_timer > 0) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
        SDL_RenderFillRect(renderer, &grid_rect);
        state->flash_timer--;
        if (state->flash_timer <= 0) state->explosion_flash = false;
    }

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            SDL_Rect cell = {state->offset_x + x * CELL_SIZE, state->offset_y + y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            SDL_SetRenderDrawColor(renderer, state->grid[x][y].revealed ? 255 : 128, state->grid[x][y].revealed ? 255 : 128, state->grid[x][y].revealed ? 255 : 128, 255);
            SDL_RenderFillRect(renderer, &cell);
            if (state->grid[x][y].revealed) {
                if (state->grid[x][y].mine) {
                    draw_text(renderer, cell.x + 10, cell.y + 5, "*", (SDL_Color){0,0,0,255}, state->font, &mine_texture);
                }
                else if (state->grid[x][y].neighbor_mines) {
                    char num[2] = {'0' + state->grid[x][y].neighbor_mines, 0};
                    draw_text(renderer, cell.x + 10, cell.y + 5, num, colors[state->grid[x][y].neighbor_mines], state->font, &num_textures[state->grid[x][y].neighbor_mines]);
                }
            } else if (state->grid[x][y].flagged) {
                draw_text(renderer, cell.x + 5, cell.y + 5, "F", (SDL_Color){255,0,0,255}, state->font, &flag_texture);
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &cell);
        }
    }

    if (state->game_over || state->victory) {
        static SDL_Texture* message_texture = 0;
        const char* message = state->victory ? "Bravo" : "Perdu";
        int msg_width = 120, msg_height = 60;
        int msg_x = state->offset_x + (GRID_SIZE * CELL_SIZE - msg_width) / 2;
        int msg_y = state->offset_y + (GRID_SIZE * CELL_SIZE - msg_height) / 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect msg_rect = {msg_x, msg_y, msg_width, msg_height};
        SDL_RenderFillRect(renderer, &msg_rect);
        draw_text(renderer, msg_x + 30, msg_y + 15, message, (SDL_Color){255,255,255,255}, state->font, &message_texture);
    }
}

// Draw the info bar with improved score and time boxes
static void draw_info_bar(SDL_Renderer* renderer, GameState* state, int mouse_x, int mouse_y) {
    static SDL_Texture* score_texture = 0, *time_texture = 0, *reessayer_texture = 0, *easy_texture = 0, *med_texture = 0, *hard_texture = 0, *hint_texture = 0, *mode_texture = 0;
    static int last_score = -1, last_time = -1; // Track last values for dynamic effect
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WINDOW_WIDTH, INFO_HEIGHT});

    // Format score with dynamic range (up to 5 digits)
    char score_text[16];
    snprintf(score_text, sizeof(score_text), "%05d", state->score);

    // Format time as MM:SS
    char time_text[16];
    int minutes = state->elapsed_time / 60;
    int seconds = state->elapsed_time % 60;
    snprintf(time_text, sizeof(time_text), "%02d:%02d", minutes, seconds);

    // Format mode text
    char mode_text[16];
    snprintf(mode_text, sizeof(mode_text), "Mode: %s", state->difficulty == EASY ? "Easy" : state->difficulty == MEDIUM ? "Middle" : "Hard");

    // Draw score box with dynamic highlight
    SDL_Rect score_box = {20, 20, 100, 40};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &score_box);
    if (state->score != last_score) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100); // Green highlight on score change
        SDL_RenderDrawRect(renderer, &score_box);
        last_score = state->score;
    }
    draw_text(renderer, 40, 25, score_text, (SDL_Color){255, 0, 0, 255}, state->font, &score_texture);

    // Draw time box with dynamic highlight
    SDL_Rect time_box = {WINDOW_WIDTH - 120, 20, 100, 40};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &time_box);
    if (state->elapsed_time != last_time && !state->game_over && !state->victory) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100); // Blue highlight on time change
        SDL_RenderDrawRect(renderer, &time_box);
        last_time = state->elapsed_time;
    }
    draw_text(renderer, WINDOW_WIDTH - 100, 25, time_text, (SDL_Color){255, 0, 0, 255}, state->font, &time_texture);

    bool reessayer_hovered = mouse_x >= WINDOW_WIDTH/2 - BUTTON_WIDTH/2 && mouse_x < WINDOW_WIDTH/2 + BUTTON_WIDTH/2 && mouse_y >= 35 && mouse_y < 35 + BUTTON_HEIGHT;
    bool easy_hovered = mouse_x >= 150 && mouse_x < 150 + BUTTON_WIDTH && mouse_y >= 65 && mouse_y < 65 + BUTTON_HEIGHT;
    bool med_hovered = mouse_x >= 250 && mouse_x < 250 + BUTTON_WIDTH && mouse_y >= 65 && mouse_y < 65 + BUTTON_HEIGHT;
    bool hard_hovered = mouse_x >= 350 && mouse_x < 350 + BUTTON_WIDTH && mouse_y >= 65 && mouse_y < 65 + BUTTON_HEIGHT;
    bool hint_hovered = mouse_x >= 450 && mouse_x < 450 + BUTTON_WIDTH && mouse_y >= 65 && mouse_y < 65 + BUTTON_HEIGHT;

    draw_reessayer_button(renderer, WINDOW_WIDTH/2 - BUTTON_WIDTH/2, 35, BUTTON_WIDTH, BUTTON_HEIGHT, state->font, &reessayer_texture, reessayer_hovered);
    draw_button(renderer, 150, 65, BUTTON_WIDTH, BUTTON_HEIGHT, "Easy", state->font, &easy_texture, easy_hovered);
    draw_button(renderer, 250, 65, BUTTON_WIDTH, BUTTON_HEIGHT, "Middle", state->font, &med_texture, med_hovered);
    draw_button(renderer, 350, 65, BUTTON_WIDTH, BUTTON_HEIGHT, "Hard", state->font, &hard_texture, hard_hovered);
    draw_button(renderer, 450, 65, BUTTON_WIDTH, BUTTON_HEIGHT, "Hint", state->font, &hint_texture, hint_hovered);
    draw_text(renderer, 20, 65, mode_text, (SDL_Color){255,255,255,255}, state->font, &mode_texture);
}

// Main game loop
int main(int argc, char* argv[]) {
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        fprintf(stderr, "Init error: %s\n", SDL_GetError());
        return 1;
    }

    GameState state = {0};
    state.font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 20);
    if (!state.font) {
        fprintf(stderr, "Font error: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window || !renderer) {
        fprintf(stderr, "Window/Renderer error: %s\n", SDL_GetError());
        TTF_CloseFont(state.font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    state.high_score = 0;
    init_game(&state, EASY);
    static SDL_Texture* textures[15] = {0};
    bool running = true;
    SDL_Event event;
    int mouse_x = 0, mouse_y = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEMOTION:
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.y < INFO_HEIGHT) {
                        if (event.button.x >= WINDOW_WIDTH/2 - BUTTON_WIDTH/2 && event.button.x < WINDOW_WIDTH/2 + BUTTON_WIDTH/2 && event.button.y >= 35 && event.button.y < 35 + BUTTON_HEIGHT) init_game(&state, state.difficulty);
                        else if (event.button.x >= 150 && event.button.x < 150 + BUTTON_WIDTH && event.button.y >= 65 && event.button.y < 65 + BUTTON_HEIGHT) init_game(&state, EASY);
                        else if (event.button.x >= 250 && event.button.x < 250 + BUTTON_WIDTH && event.button.y >= 65 && event.button.y < 65 + BUTTON_HEIGHT) init_game(&state, MEDIUM);
                        else if (event.button.x >= 350 && event.button.x < 350 + BUTTON_WIDTH && event.button.y >= 65 && event.button.y < 65 + BUTTON_HEIGHT) init_game(&state, HARD);
                        else if (event.button.x >= 450 && event.button.x < 450 + BUTTON_WIDTH && event.button.y >= 65 && event.button.y < 65 + BUTTON_HEIGHT) use_hint(&state);
                    } else if (event.button.button == SDL_BUTTON_LEFT && !state.game_over && !state.victory) {
                        int x = (event.button.x - state.offset_x) / CELL_SIZE;
                        int y = (event.button.y - state.offset_y) / CELL_SIZE;
                        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE && !state.grid[x][y].flagged) {
                            if (state.first_click) {
                                state.start_time = time(NULL); // Start time on first click
                                place_mines(&state, x, y);
                                state.first_click = false;
                            }
                            if (state.grid[x][y].mine) {
                                state.game_over = true;
                                state.elapsed_time = (int)(time(NULL) - state.start_time); // Stop time on loss
                                state.explosion_flash = true;
                                state.flash_timer = 30;
                                for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) if (state.grid[i / GRID_SIZE][i % GRID_SIZE].mine) state.grid[i / GRID_SIZE][i % GRID_SIZE].revealed = true;
                            } else {
                                reveal_cell(&state, x, y);
                                if (check_win(&state)) {
                                    state.victory = true;
                                    state.elapsed_time = (int)(time(NULL) - state.start_time); // Stop time on win
                                    if (state.score > state.high_score) state.high_score = state.score;
                                }
                            }
                        }
                    } else if (event.button.button == SDL_BUTTON_RIGHT && !state.game_over && !state.victory) {
                        int x = (event.button.x - state.offset_x) / CELL_SIZE;
                        int y = (event.button.y - state.offset_y) / CELL_SIZE;
                        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                            if (!state.grid[x][y].revealed) {
                                state.grid[x][y].flagged = !state.grid[x][y].flagged;
                                state.flagged_mines += state.grid[x][y].flagged ? 1 : -1;
                            }
                        }
                    }
                    break;
            }
        }

        if (!state.first_click && !state.game_over && !state.victory) {
            state.elapsed_time = (int)(time(NULL) - state.start_time); // Update time only during active play
        }
        SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255);
        SDL_RenderClear(renderer);
        draw_info_bar(renderer, &state, mouse_x, mouse_y);
        draw_grid(renderer, &state);
        SDL_RenderPresent(renderer);
    }

    for (int i = 0; i < 15; i++) SDL_DestroyTexture(textures[i]);
    TTF_CloseFont(state.font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
