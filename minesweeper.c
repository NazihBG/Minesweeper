#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define TAILLE_BLOC 32
#define NB_BLOCS_LARGEUR 10
#define NB_BLOCS_HAUTEUR 10
#define NB_MINES 10

typedef struct {
    bool estMine;
    bool estDecouvert;
    bool estDrapeau;
    int minesAutour;
} Case;

Case grille[NB_BLOCS_LARGEUR][NB_BLOCS_HAUTEUR];
bool jeuTermine = false;
bool victoire = false;
int score = 0;
Uint32 tempsDebut = 0;
Uint32 tempsFin = 0;

void initialiserGrille() {
    srand(time(NULL));
    for (int x = 0; x < NB_BLOCS_LARGEUR; x++) {
        for (int y = 0; y < NB_BLOCS_HAUTEUR; y++) {
            grille[x][y].estMine = false;
            grille[x][y].estDecouvert = false;
            grille[x][y].estDrapeau = false;
            grille[x][y].minesAutour = 0;
        }
    }

    int minesPlacees = 0;
    while (minesPlacees < NB_MINES) {
        int x = rand() % NB_BLOCS_LARGEUR;
        int y = rand() % NB_BLOCS_HAUTEUR;
        if (!grille[x][y].estMine) {
            grille[x][y].estMine = true;
            minesPlacees++;
        }
    }

    for (int x = 0; x < NB_BLOCS_LARGEUR; x++) {
        for (int y = 0; y < NB_BLOCS_HAUTEUR; y++) {
            if (!grille[x][y].estMine) {
                int compteur = 0;
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < NB_BLOCS_LARGEUR && ny >= 0 && ny < NB_BLOCS_HAUTEUR && grille[nx][ny].estMine) {
                            compteur++;
                        }
                    }
                }
                grille[x][y].minesAutour = compteur;
            }
        }
    }
}

void decouvrirCase(int x, int y) {
    if (x < 0 || x >= NB_BLOCS_LARGEUR || y < 0 || y >= NB_BLOCS_HAUTEUR) return;
    if (grille[x][y].estDecouvert || grille[x][y].estDrapeau) return;

    grille[x][y].estDecouvert = true;

    if (grille[x][y].estMine) {
        jeuTermine = true;
        tempsFin = SDL_GetTicks();
        return;
    }

    score += 10; // Incrémenter score si ce n’est pas une mine

    if (grille[x][y].minesAutour == 0) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx != 0 || dy != 0) {
                    decouvrirCase(x + dx, y + dy);
                }
            }
        }
    }
}

void verifierVictoire() {
    for (int x = 0; x < NB_BLOCS_LARGEUR; x++) {
        for (int y = 0; y < NB_BLOCS_HAUTEUR; y++) {
            if (!grille[x][y].estMine && !grille[x][y].estDecouvert) {
                return;
            }
        }
    }
    victoire = true;
    jeuTermine = true;
    tempsFin = SDL_GetTicks();
}

void afficherGrille(SDL_Renderer* renderer) {
    for (int x = 0; x < NB_BLOCS_LARGEUR; x++) {
        for (int y = 0; y < NB_BLOCS_HAUTEUR; y++) {
            SDL_Rect rect = { x * TAILLE_BLOC, y * TAILLE_BLOC, TAILLE_BLOC, TAILLE_BLOC };
            if (grille[x][y].estDecouvert) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
}

void afficherScoreEtTemps(SDL_Renderer* renderer) {
    char texte[64];
    Uint32 tempsActuel = SDL_GetTicks();
    Uint32 temps = jeuTermine ? (tempsFin - tempsDebut) : (tempsActuel - tempsDebut);

    snprintf(texte, sizeof(texte), "Score: %d  Temps: %d s", score, temps / 1000);
    printf("%s\n", texte); // Affichage console, à remplacer par affichage texte SDL_TTF si désiré
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Demineur", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, NB_BLOCS_LARGEUR * TAILLE_BLOC, NB_BLOCS_HAUTEUR * TAILLE_BLOC + 40, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initialiserGrille();
    tempsDebut = SDL_GetTicks();

    SDL_Event event;
    bool quitter = false;

    while (!quitter) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quitter = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && !jeuTermine) {
                int x = event.button.x / TAILLE_BLOC;
                int y = event.button.y / TAILLE_BLOC;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    decouvrirCase(x, y);
                    verifierVictoire();
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        afficherGrille(renderer);
        afficherScoreEtTemps(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
