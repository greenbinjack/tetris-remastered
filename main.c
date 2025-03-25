#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>
#include <SDL_ttf.h>

typedef uint8_t u8;

#define WIDTH 10
#define HEIGHT 22
#define VISIBLE_HEIGHT 20
#define GRID_SIZE 30

#define ARRAY_SZ(x) (sizeof (x) / sizeof ((x)[0]))

int dx[9] = { 0, 0, 0, -1, -1, -1, +1, +1, +1 };
int dy[9] = { -1, 0, +1, -1, 0, +1, -1, 0, +1 };

const u8 FRAMES_PER_DROP[] = { 48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 };

const double TARGET_SECONDS_PER_FRAME = 1.f / 60.f;

struct Color
{
  u8 r, g, b, a;
};

struct Color BASE_COLORS[]
    = { 
        { 0x28, 0x28, 0x28, 0xFF }, { 0x2D, 0x99, 0x99, 0xFF }, { 0x99, 0x99, 0x2D, 0xFF }, { 0x99, 0x2D, 0x99, 0xFF }, { 0x2D, 0x99, 0x51, 0xFF }, { 0x99, 0x2D, 0x2D, 0xFF }, { 0x2D, 0x63, 0x99, 0xFF }, { 0x99, 0x63, 0x2D, 0xFF }, { 0x83, 0x21, 0x61, 0xFF }, { 0xB4, 0xA8, 0xA8, 0xFF } };
struct Color LIGHT_COLORS[]
    = { { 0x28, 0x28, 0x28, 0xFF }, { 0x44, 0xE5, 0xE5, 0xFF }, { 0xE5, 0xE5, 0x44, 0xFF }, { 0xE5, 0x44, 0xE5, 0xFF }, { 0x44, 0xE5, 0x7A, 0xFF }, { 0xE5, 0x44, 0x44, 0xFF }, { 0x44, 0x95, 0xE5, 0xFF }, { 0xE5, 0x95, 0x44, 0xFF }, { 0xDA, 0x41, 0x67, 0xFF }, { 0xD0, 0xC6, 0xC6, 0xFF } };
struct Color DARK_COLORS[]
    = { { 0x28, 0x28, 0x28, 0xFF }, { 0x1E, 0x66, 0x66, 0xFF }, { 0x66, 0x66, 0x1E, 0xFF }, { 0x66, 0x1E, 0x66, 0xFF }, { 0x1E, 0x66, 0x36, 0xFF }, { 0x66, 0x1E, 0x1E, 0xFF }, { 0x1E, 0x42, 0x66, 0xFF }, { 0x66, 0x42, 0x1E, 0xFF }, { 0x3D, 0x26, 0x45, 0xFF }, { 0x9F, 0x93, 0x93, 0xFF } };

struct Tetrino
{
  u8 *data;
  int side;
};

u8 TETRINO_1[] = { 
                  0, 0, 0, 0, 
                  1, 1, 1, 1, 
                  0, 0, 0, 0, 
                  0, 0, 0, 0 
                };

u8 TETRINO_2[] = { 
                  2, 2, 
                  2, 2 
                };

u8 TETRINO_3[] = { 
                  0, 3, 0, 
                  3, 3, 3, 
                  0, 0, 0 
                };

u8 TETRINO_4[] = { 
                  0, 4, 4, 
                  4, 4, 0, 
                  0, 0, 0 
                };

u8 TETRINO_5[] = { 
                  5, 5, 0, 
                  0, 5, 5, 
                  0, 0, 0 
                };

u8 TETRINO_6[] = { 
                  6, 0, 0, 
                  6, 6, 6, 
                  0, 0, 0 
                };

u8 TETRINO_7[] = {
                  0, 0, 7, 
                  7, 7, 7, 
                  0, 0, 0 
                };

u8 TETRINO_SWEEPER[] = { 8 };

u8 TETRINO_BOMB[] = { 9 };

struct Tetrino TETRINOS[] = { { TETRINO_1, 4 }, { TETRINO_2, 2 }, { TETRINO_3, 3 }, { TETRINO_4, 3 }, { TETRINO_5, 3 }, { TETRINO_6, 3 }, { TETRINO_7, 3 }, { TETRINO_SWEEPER, 1 }, { TETRINO_BOMB, 1 } };

enum Game_Phase
{
  GAME_PHASE_START,
  GAME_PHASE_PLAY,
  GAME_PHASE_LINE,
  GAME_PHASE_GAMEOVER
};

struct Piece_State
{
  u8 tetrino_index;
  int offset_row, offset_col;
  int rotation;
};

struct Game_State
{
  u8 board[WIDTH * HEIGHT];
  u8 lines[HEIGHT];
  int pending_line_count;
  int bonus_line_count;
  int bomb_row, bomb_col;

  bool isbomb;

  struct Piece_State piece;
  struct Piece_State next_piece;

  enum Game_Phase phase;

  int start_level;
  int level;
  int line_count;
  int points;

  double next_drop_time;
  double highlight_end_time;
  double time;
};

struct Input_State
{
  u8 left, right, up, down, a;
  int dleft, dright, dup, ddown, da;
};

enum Text_Align
{
  TEXT_ALIGN_LEFT,
  TEXT_ALIGN_CENTER,
  TEXT_ALIGN_RIGHT
};

int
random_int (int mn, int mx)
{
  int range = mx - mn;
  return mn + rand () % range;
}

int
min (int x, int y)
{
  return x < y ? x : y;
}
int
max (int x, int y)
{
  return x > y ? x : y;
}

/**
 * GAME PHYSICS STARTS HERE
 */

u8
matrix_get (u8 *values, int row, int col)
{
  return values[row * WIDTH + col];
}

void
matrix_set (u8 *values, int row, int col, u8 value)
{
  values[row * WIDTH + col] = value;
}

bool
valid_cell (int x, int y)
{
  return x >= 0 && x < HEIGHT && y >= 0 && y < WIDTH;
}

u8
tetrino_get (struct Tetrino *tetrino, int row, int col, int rotation)
{
  int side = tetrino->side;
  if (rotation == 0)
    return tetrino->data[row * side + col];
  else if (rotation == 1)
    return tetrino->data[(side - col - 1) * side + row];
  else if (rotation == 2)
    return tetrino->data[(side - row - 1) * side + (side - col - 1)];
  else if (rotation == 3)
    return tetrino->data[col * side + (side - row - 1)];
  return 0;
}

u8
check_row_filled (struct Game_State *game, int row)
{
  int arr[10] = { 0 };
  for (int col = 0; col < WIDTH; ++col)
    {
      arr[matrix_get (game->board, row, col)]++;
    }
  for (int color = 1; color < 8; color++)
    {
      if (arr[color] == WIDTH - arr[8])
        return 2;
    }
  if (arr[8])
    return 1;
  else if (arr[0])
    return 0;
  else
    return 1;
}

u8
check_row_empty (u8 *values, int row)
{
  for (int col = 0; col < WIDTH; ++col)
    {
      if (matrix_get (values, row, col))
        {
          return 0;
        }
    }
  return 1;
}

int
find_lines (struct Game_State *game, u8 *lines_out)
{
  int count = 0;
  for (int row = 0; row < HEIGHT; ++row)
    {
      u8 filled = check_row_filled (game, row);
      lines_out[row] = filled;
      count += (filled > 0);
    }
  return count;
}

bool
find_bomb (struct Game_State *game)
{
  for (int row = 0; row < HEIGHT; ++row)
    {
      for (int col = 0; col < WIDTH; col++)
        {
          if (matrix_get (game->board, row, col) == 9)
            {
              game->bomb_row = row;
              game->bomb_col = col;
              return true;
            }
        }
    }
  return false;
}

void
clear_lines (u8 *values, u8 *lines)
{
  int src_row = HEIGHT - 1;
  for (int dst_row = HEIGHT - 1; dst_row >= 0; --dst_row)
    {
      while (src_row >= 0 && lines[src_row])
        {
          --src_row;
        }
      if (src_row < 0)
        {
          memset (values + dst_row * WIDTH, 0, WIDTH);
        }
      else
        {
          if (src_row != dst_row)
            {
              memcpy (values + dst_row * WIDTH, values + src_row * WIDTH, WIDTH);
            }
          --src_row;
        }
    }
}

int
find_bonus_lines (u8 *lines)
{
  int cnt = 0;
  for (int row = 0; row < HEIGHT; row++)
    {
      if (lines[row] == 2)
        cnt++;
    }
  return cnt;
}

void
clear_surroundings (u8 *values, int row, int col)
{
  for (int i = 0; i < 9; i++)
    {
      int new_x = row + dx[i], new_y = col + dy[i];
      if (valid_cell (new_x, new_y))
        matrix_set (values, new_x, new_y, 0);
    }
}

bool
check_piece_valid (struct Piece_State *piece, u8 *board)
{
  struct Tetrino *tetrino = TETRINOS + piece->tetrino_index;
  for (int row = 0; row < tetrino->side; ++row)
    {
      for (int col = 0; col < tetrino->side; ++col)
        {
          u8 value = tetrino_get (tetrino, row, col, piece->rotation);
          if (value > 0)
            {
              int board_row = piece->offset_row + row;
              int board_col = piece->offset_col + col;
              if (!valid_cell (board_row, board_col) || matrix_get (board, board_row, board_col))
                {
                  return false;
                }
            }
        }
    }
  return true;
}

void
merge_piece (struct Game_State *game)
{
  struct Tetrino *tetrino = TETRINOS + game->piece.tetrino_index;
  for (int row = 0; row < tetrino->side; ++row)
    {
      for (int col = 0; col < tetrino->side; ++col)
        {
          u8 value = tetrino_get (tetrino, row, col, game->piece.rotation);
          if (value)
            {
              int board_row = game->piece.offset_row + row;
              int board_col = game->piece.offset_col + col;
              matrix_set (game->board, board_row, board_col, value);
            }
        }
    }
}

double
get_time_to_next_drop (int level)
{
  level = min (level, ARRAY_SZ (FRAMES_PER_DROP) - 1);
  return FRAMES_PER_DROP[level] * TARGET_SECONDS_PER_FRAME;
}

void
spawn_piece (struct Game_State *game)
{
  memset (&(game->piece), 0, sizeof (game->piece));
  game->piece.tetrino_index = game->next_piece.tetrino_index;

  memset (&(game->next_piece), 0, sizeof (game->next_piece));
  game->next_piece.tetrino_index = (u8)random_int (0, ARRAY_SZ (TETRINOS));

  game->piece.offset_col = WIDTH / 3;
  game->next_drop_time = game->time + get_time_to_next_drop (game->level);
}

bool
soft_drop (struct Game_State *game)
{
  ++game->piece.offset_row;
  if (!check_piece_valid (&game->piece, game->board))
    {
      --game->piece.offset_row;
      merge_piece (game);
      spawn_piece (game);
      return false;
    }

  game->next_drop_time = game->time + get_time_to_next_drop (game->level);
  return true;
}

int
compute_points (int level, int line_count)
{
  if (line_count == 1)
    return 40 * (level + 1);
  else if (line_count == 2)
    return 100 * (level + 1);
  else if (line_count == 3)
    return 300 * (level + 1);
  else if (line_count == 4)
    return 1200 * (level + 1);
  return 0;
}

int
compute_bonus_points (int level, int line_count)
{
  return 10 * line_count * line_count * (level + 1);
}

int
get_lines_for_next_level (int start_level, int level)
{
  int first_level_up_limit = min ((start_level * 10 + 10), max (100, (start_level * 10 - 50)));
  if (level == start_level)
    return first_level_up_limit;
  int diff = level - start_level;
  return first_level_up_limit + diff * 10;
}

void
update_game_start (struct Game_State *game, struct Input_State *input)
{
  memset (game->board, 0, WIDTH * HEIGHT);
  if (input->dup > 0 && game->start_level < 29)
    {
      ++game->start_level;
    }
  if (input->ddown > 0 && game->start_level > 0)
    {
      --game->start_level;
    }
  if (input->da > 0)
    {
      game->level = game->start_level;
      game->line_count = 0;
      game->points = 0;
      game->next_piece.tetrino_index = (u8)random_int (0, ARRAY_SZ (TETRINOS));
      spawn_piece (game);
      game->phase = GAME_PHASE_PLAY;
    }
}

void
update_game_gameover (struct Game_State *game, struct Input_State *input)
{
  if (input->da > 0)
    {
      game->phase = GAME_PHASE_START;
    }
}

void
update_game_line (struct Game_State *game)
{
  if (game->time >= game->highlight_end_time)
    {
      if (game->isbomb)
        {
          clear_surroundings (game->board, game->bomb_row, game->bomb_col);
        }
      else
        {
          clear_lines (game->board, game->lines);
          game->line_count += game->pending_line_count;
          game->points += compute_points (game->level, game->pending_line_count);
          game->points += compute_bonus_points (game->level, game->bonus_line_count);
        }

      int lines_for_next_level = get_lines_for_next_level (game->start_level, game->level);
      if (game->line_count >= lines_for_next_level)
        {
          ++game->level;
        }

      game->phase = GAME_PHASE_PLAY;
    }
}

void
update_game_play (struct Game_State *game, struct Input_State *input)
{
  struct Piece_State piece = game->piece;
  if (input->dleft > 0)
    {
      --piece.offset_col;
    }
  if (input->dright > 0)
    {
      ++piece.offset_col;
    }
  if (input->dup > 0)
    {
      piece.rotation = (piece.rotation + 1) % 4;
    }
  if (check_piece_valid (&piece, game->board))
    {
      game->piece = piece;
    }
  if (input->ddown > 0)
    {
      soft_drop (game);
    }
  if (input->da > 0)
    {
      while (soft_drop (game))
        ;
    }
  while (game->time >= game->next_drop_time)
    {
      soft_drop (game);
    }

  game->pending_line_count = find_lines (game, game->lines);
  game->isbomb = find_bomb (game);
  game->bonus_line_count = find_bonus_lines (game->lines);

  if (game->pending_line_count > 0 || game->isbomb == true)
    {
      game->phase = GAME_PHASE_LINE;
      game->highlight_end_time = game->time + 0.5f;
    }

  int game_over_row = 1;
  if (!check_row_empty (game->board, game_over_row))
    {
      game->phase = GAME_PHASE_GAMEOVER;
    }
}

void
update_game (struct Game_State *game, struct Input_State *input)
{
  if (game->phase == GAME_PHASE_START)
    update_game_start (game, input);
  else if (game->phase == GAME_PHASE_PLAY)
    update_game_play (game, input);
  else if (game->phase == GAME_PHASE_LINE)
    update_game_line (game);
  else if (game->phase == GAME_PHASE_GAMEOVER)
    update_game_gameover (game, input);
}

/**
 * SDL FUNCTIONS STARTS HERE
 */

void
fill_rect (SDL_Renderer *renderer, int x, int y, int width, int height, struct Color color)
{
  SDL_Rect rect = { 0 };
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;
  SDL_SetRenderDrawColor (renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect (renderer, &rect);
}

void
draw_rect (SDL_Renderer *renderer, int x, int y, int width, int height, struct Color color)
{
  SDL_Rect rect = { 0 };
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;
  SDL_SetRenderDrawColor (renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawRect (renderer, &rect);
}

void
draw_string (SDL_Renderer *renderer, TTF_Font *font, char *text, int x, int y, enum Text_Align alignment, struct Color color)
{
  SDL_Color sdl_color = { color.r, color.g, color.b, color.a };
  SDL_Surface *surface = TTF_RenderText_Solid (font, text, sdl_color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface (renderer, surface);

  SDL_Rect rect;
  rect.y = y;
  rect.w = surface->w;
  rect.h = surface->h;
  if (alignment == TEXT_ALIGN_LEFT)
    rect.x = x;
  else if (alignment == TEXT_ALIGN_CENTER)
    rect.x = x - surface->w / 2;
  else if (alignment == TEXT_ALIGN_RIGHT)
    rect.x = x - surface->w;

  SDL_RenderCopy (renderer, texture, 0, &rect);
  SDL_FreeSurface (surface);
  SDL_DestroyTexture (texture);
}

void
draw_cell (SDL_Renderer *renderer, int row, int col, u8 value, int offset_x, int offset_y, bool outline)
{
  struct Color base_color = BASE_COLORS[value];
  struct Color light_color = LIGHT_COLORS[value];
  struct Color dark_color = DARK_COLORS[value];

  int edge = GRID_SIZE / 8;

  int x = col * GRID_SIZE + offset_x;
  int y = row * GRID_SIZE + offset_y;

  if (outline)
    {
      draw_rect (renderer, x, y, GRID_SIZE, GRID_SIZE, base_color);
      return;
    }

  fill_rect (renderer, x, y, GRID_SIZE, GRID_SIZE, dark_color);
  fill_rect (renderer, x + edge, y, GRID_SIZE - edge, GRID_SIZE - edge, light_color);
  fill_rect (renderer, x + edge, y + edge, GRID_SIZE - edge * 2, GRID_SIZE - edge * 2, base_color);
}

void
draw_piece (SDL_Renderer *renderer, struct Piece_State *piece, int offset_x, int offset_y, bool outline)
{
  struct Tetrino *tetrino = TETRINOS + piece->tetrino_index;
  for (int row = 0; row < tetrino->side; ++row)
    {
      for (int col = 0; col < tetrino->side; ++col)
        {
          u8 value = tetrino_get (tetrino, row, col, piece->rotation);
          if (value)
            {
              draw_cell (renderer, row + piece->offset_row, col + piece->offset_col, value, offset_x, offset_y, outline);
            }
        }
    }
}

void
draw_board (SDL_Renderer *renderer, u8 *board, int width, int height, int offset_x, int offset_y)
{
  fill_rect (renderer, offset_x, offset_y, width * GRID_SIZE, height * GRID_SIZE, BASE_COLORS[0]);
  for (int row = 0; row < height; ++row)
    {
      for (int col = 0; col < width; ++col)
        {
          u8 value = matrix_get (board, row, col);
          if (value)
            {
              draw_cell (renderer, row, col, value, offset_x, offset_y, false);
            }
        }
    }
}

void
render_game (struct Game_State *game, SDL_Renderer *renderer, TTF_Font *font)
{
  char buffer[308];

  struct Color highlight_color = { 0xFF, 0xFF, 0xFF, 0xFF };

  int margin_y = 60;

  draw_board (renderer, game->board, WIDTH, HEIGHT, 0, margin_y);

  snprintf (buffer, sizeof (buffer), "NEXT PIECE:");
  draw_string (renderer, font, buffer, 400, 320, TEXT_ALIGN_CENTER, highlight_color);

  /**
   * GAME MANUAL
   */
  draw_string (renderer, font, "GAME MANUAL:", 310, 30, TEXT_ALIGN_LEFT, highlight_color);
  draw_string (renderer, font, "#ARROW KEYS TO MOVE PIECE", 310, 80, TEXT_ALIGN_LEFT, highlight_color);
  draw_string (renderer, font, "#UP ARROW FOR ROTATION", 310, 120, TEXT_ALIGN_LEFT, highlight_color);
  draw_string (renderer, font, "#SPACEBAR FOR HARD DROP", 310, 160, TEXT_ALIGN_LEFT, highlight_color);
  draw_string (renderer, font, "#ESCAPE TO QUIT GAME", 310, 200, TEXT_ALIGN_LEFT, highlight_color);
  draw_string (renderer, font, "#MAX LEVEL: 29", 310, 240, TEXT_ALIGN_LEFT, highlight_color);

  if (game->phase == GAME_PHASE_PLAY)
    {
      draw_piece (renderer, &game->piece, 0, margin_y, false);
      draw_piece (renderer, &game->next_piece, 350, 370, false);
      struct Piece_State piece = game->piece;
      while (check_piece_valid (&piece, game->board))
        {
          piece.offset_row++;
        }
      --piece.offset_row;

      draw_piece (renderer, &piece, 0, margin_y, true);
    }

  if (game->phase == GAME_PHASE_LINE)
    {
      if (game->isbomb == true)
        {
          for (int i = 0; i < 9; i++)
            {
              int new_x = game->bomb_row + dx[i], new_y = game->bomb_col + dy[i];
              if (valid_cell (new_x, new_y))
                {
                  int x = new_y * GRID_SIZE;
                  int y = new_x * GRID_SIZE + margin_y;
                  fill_rect (renderer, x, y, GRID_SIZE, GRID_SIZE, highlight_color);
                }
            }
        }
      else
        {
          for (int row = 0; row < HEIGHT; ++row)
            {
              if (game->lines[row])
                {
                  int x = 0;
                  int y = row * GRID_SIZE + margin_y;

                  fill_rect (renderer, x, y, WIDTH * GRID_SIZE, GRID_SIZE, highlight_color);
                }
            }
        }
    }
  else if (game->phase == GAME_PHASE_GAMEOVER)
    {
      int x = WIDTH * GRID_SIZE / 2;
      int y = (HEIGHT * GRID_SIZE + margin_y) / 2;
      draw_string (renderer, font, "GAME OVER", x, y, TEXT_ALIGN_CENTER, highlight_color);
      draw_string (renderer, font, "PRESS SPACEBAR TO", x, y + 30, TEXT_ALIGN_CENTER, highlight_color);
      draw_string (renderer, font, "START NEW GAME", x, y + 60, TEXT_ALIGN_CENTER, highlight_color);
    }
  else if (game->phase == GAME_PHASE_START)
    {
      int x = WIDTH * GRID_SIZE / 2;
      int y = (HEIGHT * GRID_SIZE + margin_y) / 2 - 30;

      snprintf (buffer, sizeof (buffer), "STARTING LEVEL: %d", game->start_level);
      draw_string (renderer, font, buffer, x, y, TEXT_ALIGN_CENTER, highlight_color);
      draw_string (renderer, font, "USE ARROW KEYS", x, y + 30, TEXT_ALIGN_CENTER, highlight_color);
      draw_string (renderer, font, "TO CHANGE LEVEL", x, y + 60, TEXT_ALIGN_CENTER, highlight_color);
      draw_string (renderer, font, "PRESS SPACEBAR", x, 500, TEXT_ALIGN_CENTER, highlight_color);
      draw_string (renderer, font, "TO START", x, 530, TEXT_ALIGN_CENTER, highlight_color);
    }

  struct Color black_color = { 0x00, 0x00, 0x00, 0x00 };

  fill_rect (renderer, 0, margin_y, WIDTH * GRID_SIZE, (HEIGHT - VISIBLE_HEIGHT) * GRID_SIZE, black_color);

  snprintf (buffer, sizeof (buffer), "LEVEL: %d", game->level);
  draw_string (renderer, font, buffer, 5, 10, TEXT_ALIGN_LEFT, highlight_color);

  snprintf (buffer, sizeof (buffer), "LINES: %d", game->line_count);
  draw_string (renderer, font, buffer, 5, 40, TEXT_ALIGN_LEFT, highlight_color);

  snprintf (buffer, sizeof (buffer), "POINTS: %d", game->points);
  draw_string (renderer, font, buffer, 5, 70, TEXT_ALIGN_LEFT, highlight_color);
}

int
main (int argc, char *argv[])
{
  if (SDL_Init (SDL_INIT_VIDEO) < 0)
    {
      return 1;
    }
  if (TTF_Init () < 0)
    {
      return 2;
    }
  SDL_Window *window = SDL_CreateWindow ("TETRIS REMASTERED", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 300 + 400, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  const char *font_name = "custom_font.ttf";
  TTF_Font *font = TTF_OpenFont (font_name, 22);

  struct Game_State game;
  struct Input_State input;
  memset (&(game), 0, sizeof (game));
  memset (&(input), 0, sizeof (input));

  spawn_piece (&game);

  bool quit = false;
  while (!quit)
    {
      game.time = SDL_GetTicks () / 1000.0f;

      SDL_Event e;
      while (SDL_PollEvent (&e) != 0)
        {
          if (e.type == SDL_QUIT)
            {
              quit = true;
            }
        }

      int key_count;
      const u8 *key_states = SDL_GetKeyboardState (&key_count);

      if (key_states[SDL_SCANCODE_ESCAPE])
        {
          quit = true;
        }

      struct Input_State prev_input = input;

      input.left = key_states[SDL_SCANCODE_LEFT];
      input.right = key_states[SDL_SCANCODE_RIGHT];
      input.up = key_states[SDL_SCANCODE_UP];
      input.down = key_states[SDL_SCANCODE_DOWN];
      input.a = key_states[SDL_SCANCODE_SPACE];

      input.dleft = (int)input.left - (int)prev_input.left;
      input.dright = (int)input.right - (int)prev_input.right;
      input.dup = (int)input.up - (int)prev_input.up;
      input.ddown = (int)input.down - (int)prev_input.down;
      input.da = (int)input.a - (int)prev_input.a;

      SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
      SDL_RenderClear (renderer);

      update_game (&game, &input);
      render_game (&game, renderer, font);

      SDL_RenderPresent (renderer);
    }

  TTF_CloseFont (font);
  SDL_DestroyRenderer (renderer);
  SDL_Quit ();

  return 0;
}
