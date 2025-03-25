# Tetris Remastered

**Tetris Remastered** is a modern implementation of the classic Tetris game, designed with enhanced features and using the SDL2 library. The game retains the familiar gameplay mechanics of the original Tetris but introduces several new elements, such as hard drops, bomb blocks, and bonus point systems, to provide a fresh and engaging experience.

## Features
- Classic gameplay mechanics faithful to the original Tetris, such as piece rotation, soft drop, and piece movement.
- Customizable difficulty levels ranging from Level 0 to Level 29, with progressively increasing challenge.
- Scoring system based on the number of lines cleared and level progression.
- Hard drop functionality that allows pieces to fall instantly when the spacebar is pressed.
- Shadow movement showing where the piece will land, aiding in decision-making.
- Sweeper blocks that clear the line they land in.
- Bomb blocks that clear neighboring blocks when they land.
- Bonus points for clearing lines made up of the same color blocks, calculated by the formula: `10 * N * N`, where `N = level + 1`.
- Ability to pause and resume the game during gameplay.
- Option to restart or quit the game after a game over.

## Technology Used
- **Language**: C
- **Libraries and Frameworks**: 
  - **SDL2**: Used for rendering graphics, handling window management, and processing user inputs.
  - **SDL2_ttf**: Used to render TrueType fonts for displaying in-game text such as score, level, and instructions.

## Installation & Setup (Linux)

#### 1. Install SDL2 and SDL2_ttf
To compile and run the game, you will first need to install the necessary libraries, SDL2 and SDL2_ttf.

```sh
sudo apt update
sudo apt install libsdl2-dev libsdl2-ttf-dev build-essential
```

#### 2. Clone the repository  
```sh
git clone https://github.com/greenbinjack/tetris-remastered.git
cd tetris-remastered
```

#### 3. Compile and Run the Game
```sh
gcc -g main.c -o game -lSDL2 -lSDL2_ttf
./game
```
