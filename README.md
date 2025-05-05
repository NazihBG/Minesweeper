# 🎮 Minesweeper Game

## 📝 Overview

This project is a modern implementation of the classic Minesweeper game, a single-player puzzle video game that has captivated players since its rise to fame with Microsoft in the 1990s. The goal is to clear a grid containing hidden "mines" without triggering them, using numerical hints to locate the mines safely.  
This version features a sleek interface, multiple difficulty levels, and interactive elements, making it both entertaining and a great learning tool for C and SDL2 game development.  
The game is built entirely in C, utilizing the SDL2 (Simple DirectMedia Layer) library for graphics and input handling, and SDL_ttf for text rendering. It includes a dynamic timer, score system, animated buttons, and gradient backgrounds for a contemporary look. The project is self-contained within a single file, `jeuDemineur.c`, requiring no external images or assets.

## 🎯 Game Principle

Minesweeper challenges players to reveal all non-mine cells on a grid while avoiding mines. Here's the core mechanic:

### Grid Setup:
A grid of cells (sized according to difficulty) hides a set number of mines.

### Gameplay:
- Left-click to reveal a cell. A mine ends the game in a loss, while a safe cell displays a number (adjacent mine count) or remains blank (no adjacent mines).
- Right-click to toggle a flag on a cell to mark suspected mines.
- Revealing a "0" cell triggers a recursive reveal of adjacent cells.

### Winning Condition:
Clear all non-mine cells.

### Losing Condition:
Hit a mine.

### Scoring:
Earn 10 points for each non-mine cell revealed.

### Timer:
Starts on the first click and halts when the game ends.

## ✨ Features

- **Multiple Difficulty Levels** 🌟:
  - Easy: 10x10 grid, 15 mines
  - Medium: 11x11 grid, 25 mines
  - Hard: 12x12 grid, 40 mines

- **Score Tracking** 📈: Increments by 10 points per non-mine cell revealed.

- **Dynamic Timer** ⏰: Starts on first click, stops on win/loss.

- **Interactive UI** 🎨:
  - Animated buttons with hover effects
  - Gradient backgrounds
  - Rounded rectangles with shadows

- **Game State Indicators** 🔔: Displays "Félicitations" on win and "Perdu" on loss with animation.

- **Flagging System** 🚩: Right-click to toggle a text-based flag ("F").

- **Recursive Reveal** 🔄: Uncovers adjacent "0" cells automatically.

- **No External Assets** 🖼️: Entirely rendered with SDL2 (no image files used).

## 🗂️ Global Structure

The project is encapsulated in a single C file, `jeuDemineur.c`, organized to separate game logic, rendering, and event handling.

### 🔑 Key Components

#### Constants and Macros 📏:
- Window/grid sizes, cell size, mine count per difficulty
- UI constants (button size, animation speed, corner radius)

#### Data Structures 🛠️:
- `enum Difficulty` for EASY, MEDIUM, HARD
- `struct Cell` for cell state (revealed, mine, flagged, neighbor mines)
- `struct GameState` for managing grid, timer, score, difficulty, fonts

#### Core Functions ⚙️:
- `set_difficulty`, `init_grid`, `place_mines`, `reveal_cell`, `check_win`
- **Rendering:** `draw_rounded_rect`, `draw_text_centered`, `draw_button`, `draw_gradient_background`, `draw_grid`, `draw_info_bar`

#### Main Loop 🔄:
- SDL2 and TTF init, window/renderer setup
- Event processing and state updates
- Frame-based display updates

## 💻 Technical Stack

### Language: C

### Libraries 📚:
- SDL2 (graphics, input)
- SDL_ttf (text rendering)

### Dependencies 🔗:
- Arial font (default: `C:/Windows/Fonts/arial.ttf`)

### Environment 🛠️:
- C99-compatible compiler (e.g., gcc)
- SDL2/SDL_ttf linked during compilation

## 📦 Installation and Setup

### 🔧 Prerequisites

Install SDL2 and SDL_ttf:

- **Windows:** Download and link development libraries
- **Linux:** `sudo apt-get install libsdl2-dev libsdl2-ttf-dev`
- **macOS:** `brew install sdl2 sdl2_ttf`

Ensure a C compiler (e.g., gcc) is installed and the Arial font is available at `C:/Windows/Fonts/arial.ttf` (or adjust the path in the code).

### ⚙️ Compilation

Compile the code using the following command (adjust for your environment):

```bash
gcc jeuDemineur.c -o minesweeper -lSDL2 -lSDL2_ttf

```
### 🏃 Running the Game

Après avoir compilé le projet avec `make`, vous pouvez lancer le jeu en exécutant le binaire compilé :

```bash
./minesweeper
```

## 🎮 How to Play

**Launch the Game:** 🚀 Run the executable; it starts in Easy mode by default.

**Choose Difficulty:** 🌟 Click "Facile" (Easy), "Moyen" (Medium), or "Difficile" (Hard) to select a difficulty level:

- **Easy:** 10x10 grid, 15 mines
- **Medium:** 11x11 grid, 25 mines
- **Hard:** 12x12 grid, 40 mines

**Play the Game:** ⏯️

- Left-click on a cell to reveal it. If it’s a mine, you lose; otherwise, it shows a number or blank.
- Right-click to flag/unflag a cell as a suspected mine (marked with "F").
- The timer ⏰ starts on your first click and stops when you win or lose.
- Earn 📈 10 points for each non-mine cell revealed.

**Restart:** 🔄 Click the "Restart" button to begin a new game at the current difficulty.

**Win or Lose:** 🏁 The game ends when you either reveal a mine ("Perdu") or clear all non-mine cells ("Félicitations"). A message with a scaling animation appears.



## 🚀 Future Improvements
Here are some ideas to enhance the game in future updates:

- **Leaderboard:** 📊 Implement a system to save and display high scores for each difficulty level.
- **Custom Grid Sizes:** 🎨 Allow players to specify custom grid dimensions and mine counts.
- **Sound Effects:** 🔊 Add audio feedback for actions like revealing cells, flagging, and winning/losing.
- **Flexible Font Paths:** 🌐 Use relative paths for the Arial font to improve cross-platform compatibility.
- **Enhanced Animations:** 🎞️ Introduce more visual effects, such as explosions for mines or a celebratory animation for wins.
- **Themes:** 🎨 Add support for different color themes or skins for the grid and UI.

## 🤝 Contributing
Contributions are welcome! 📬 If you'd like to improve this project, you can always reach me:



If you have any questions or suggestions, feel free to reach out to me:

**Email:** ✉️ [nazih.bouguerba@etudiant-enit.utm.tn](mailto:nazih.bouguerba@etudiant-enit.utm.tn) 📩


## ✨ Acknowledgments
- Inspired by the classic Minesweeper game popularized by Microsoft. 🖥️
- Built using the amazing SDL2 and SDL_ttf libraries, thanks to the open-source community. ❤️
- Gradient backgrounds and animation effects inspired by modern UI design trends. 🎨


