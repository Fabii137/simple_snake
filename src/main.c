#include "raylib.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 900

#define GRID_ROWS 25
#define GRID_COLS 25

#define CELL_WIDTH SCREEN_WIDTH / GRID_COLS
#define CELL_HEIGHT SCREEN_HEIGHT / GRID_ROWS

typedef struct {
  int x, y;
} Vector2i;

bool vector2i_equals(Vector2i v1, Vector2i v2) {
  return v1.x == v2.x && v1.y == v2.y;
}

Vector2i vector2i_add(Vector2i v1, Vector2i v2) {
  return (Vector2i){v1.x + v2.x, v1.y + v2.y};
}

typedef struct node {
  Vector2i value;
  struct node *next;
  struct node *prev;
} Node;

void node_free(Node *node) {
  free(node);
  node = NULL;
}

typedef struct {
  Node *head;
  Node *tail;
  size_t size;
} LinkedList;

void linked_list_init(LinkedList *list) {
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

void linked_list_free(LinkedList *list) {
  if (list->size == 0) {
    return;
  }

  Node *current = list->head;

  while (current != NULL) {
    Node *next = current->next;
    node_free(current);
    current = next;
  }
}

void linked_list_push_front(LinkedList *list, Vector2i value) {
  Node *new_head = (Node *)malloc(sizeof(Node));
  if (new_head == NULL) {
    printf("Memory allocation failed!\n");
    exit(EXIT_FAILURE);
  };

  new_head->value = value;
  new_head->prev = NULL;
  Node *prev_head = list->head;
  list->head = new_head;
  list->size++;
  if (list->size == 1) {
    list->tail = new_head;
    new_head->next = NULL;
    return;
  }
  new_head->next = prev_head;
  prev_head->prev = new_head;
}

void linked_list_pop_back(LinkedList *list) {
  if (list->size == 0) {
    return;
  }

  Node *prev_tail = list->tail;
  Node *new_tail = prev_tail->prev;
  node_free(prev_tail);
  list->size--;

  if (list->size == 0) {
    list->head = NULL;
    list->tail = NULL;
    return;
  }
  new_tail->next = NULL;
  list->tail = new_tail;
}

bool linked_list_contains(LinkedList *list, Vector2i value) {
  if (list->size == 0) {
    return false;
  }
  Node *current = list->head;
  while (current != NULL) {
    if (vector2i_equals(current->value, value)) {
      return true;
    }
    current = current->next;
  }
  return false;
}

Vector2i linked_list_get_head_val(LinkedList *list) {
  return list->head->value;
}

Vector2i linked_list_get_tail_val(LinkedList *list) {
  return list->tail->value;
}

enum Direction { UP, LEFT, DOWN, RIGHT, NONE };

Vector2i parse_direction(enum Direction direction) {
  switch (direction) {
  case UP:
    return (Vector2i){0, -1};
  case LEFT:
    return (Vector2i){-1, 0};
  case DOWN:
    return (Vector2i){0, 1};
  case RIGHT:
    return (Vector2i){1, 0};
  case NONE:
    return (Vector2i){0, 0};
  }
}

enum CellState { EMPTY, SNAKE_BODY, SNAKE_HEAD, FOOD };

typedef struct {
  Vector2i pos;
  enum CellState state;
} Cell;

typedef struct {
  LinkedList positions;
} Snake;

typedef struct {
  Cell grid[GRID_COLS][GRID_ROWS];
  Snake snake;
  Vector2i food;
  int score;
} Game;

void cell_set_state(Game *game, Vector2i pos, enum CellState state) {
  Cell *cell = &game->grid[pos.x][pos.y];
  cell->state = state;
}

enum CellState cell_get_state(Game *game, Vector2i pos) {
  return game->grid[pos.x][pos.y].state;
}

Color cell_get_color(Cell cell) {
  switch (cell.state) {
  case EMPTY:
    return BLACK;
  case SNAKE_BODY:
    return GREEN;
  case SNAKE_HEAD:
    return DARKGREEN;
  case FOOD:
    return RED;
  }
}

void cell_draw(Cell cell) {
  int pos_x = cell.pos.x * CELL_WIDTH;
  int pos_y = cell.pos.y * CELL_HEIGHT;
  DrawRectangle(pos_x, pos_y, CELL_WIDTH, CELL_HEIGHT, cell_get_color(cell));
  DrawRectangleLines(pos_x, pos_y, CELL_WIDTH, CELL_HEIGHT, DARKGRAY);
}

void snake_init(Game *game) {
  Snake *snake = &game->snake;
  LinkedList positions;
  linked_list_init(&positions);
  snake->positions = positions;

  Vector2i center = {GRID_COLS / 2, GRID_ROWS / 2};
  linked_list_push_front(&snake->positions, center);
  cell_set_state(game, center, SNAKE_HEAD);
}

void snake_add_head(Game *game, Vector2i new_head_pos) {
  Snake *snake = &game->snake;

  cell_set_state(game, linked_list_get_head_val(&snake->positions),
                 SNAKE_BODY); // change old head to body

  linked_list_push_front(&snake->positions, new_head_pos);
  cell_set_state(game, new_head_pos, SNAKE_HEAD);
}

void snake_move(Game *game, Vector2i new_head_pos) {
  Snake *snake = &game->snake;

  if (snake->positions.size > 1) {
    cell_set_state(game, linked_list_get_head_val(&snake->positions),
                   SNAKE_BODY); // change old head to body
  }

  Vector2i tail = linked_list_get_tail_val(&snake->positions);
  linked_list_pop_back(&snake->positions);
  cell_set_state(game, tail, EMPTY);

  linked_list_push_front(&snake->positions, new_head_pos);
  cell_set_state(game, new_head_pos, SNAKE_HEAD);
}

void place_food(Game *game) {
  Vector2i pos;
  do {
    pos.x = rand() % GRID_COLS;
    pos.y = rand() % GRID_ROWS;
  } while (linked_list_contains(&game->snake.positions, pos));
  game->food = pos;
  cell_set_state(game, pos, FOOD);
}

void grid_init(Cell grid[GRID_COLS][GRID_ROWS]) {
  for (int x = 0; x < GRID_COLS; x++) {
    for (int y = 0; y < GRID_ROWS; y++) {
      grid[x][y] = (Cell){(Vector2i){x, y}, EMPTY};
    }
  }
}

void grid_draw(Cell grid[GRID_COLS][GRID_ROWS]) {
  for (int x = 0; x < GRID_COLS; x++) {
    for (int y = 0; y < GRID_ROWS; y++) {
      cell_draw(grid[x][y]);
    }
  }
}

void game_init(Game *game) {
  srand(time(NULL));
  grid_init(game->grid);
  snake_init(game);
  place_food(game);
  game->score = 0;
}

void game_destroy(Game *game) {
  linked_list_free(&game->snake.positions);
  game = NULL;
}

bool is_colliding(Game *game, Vector2i new_head_pos) {
  // check bounds
  if (new_head_pos.x < 0 || new_head_pos.y < 0 || new_head_pos.x >= GRID_COLS ||
      new_head_pos.y >= GRID_ROWS)
    return true;

  Snake *snake = &game->snake;
  enum CellState state = cell_get_state(game, new_head_pos);
  bool growing = state == FOOD;

  Vector2i tail = linked_list_get_tail_val(&snake->positions);
  // allow going into prev tail if not growing
  if (!growing && vector2i_equals(new_head_pos, tail)) {
    return false;
  }

  // check self
  if (linked_list_contains(&snake->positions, new_head_pos))
    return true;

  return false;
}

bool game_update(Game *game, enum Direction current_direction) {
  Snake *snake = &game->snake;
  Vector2i prev_head_pos = linked_list_get_head_val(&snake->positions);
  Vector2i new_head_pos =
      vector2i_add(prev_head_pos, parse_direction(current_direction));

  if (is_colliding(game, new_head_pos)) {
    printf("You lost! Score: %d\n", game->score);
    return false;
  }

  enum CellState new_head_state = cell_get_state(game, new_head_pos);
  switch (new_head_state) {
  case SNAKE_BODY:
  case EMPTY:
    snake_move(game, new_head_pos);
    break;
  case FOOD:
    snake_add_head(game, new_head_pos);
    place_food(game);
    game->score++;
    break;
  default:
    break;
  }

  return true;
}

void handle_input(enum Direction current_direction,
                  enum Direction *queued_direction) {
  if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && current_direction != DOWN)
    *queued_direction = UP;
  else if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) &&
           current_direction != RIGHT)
    *queued_direction = LEFT;
  else if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && current_direction != UP)
    *queued_direction = DOWN;
  else if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) &&
           current_direction != LEFT)
    *queued_direction = RIGHT;
}

int main() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");
  SetTargetFPS(240);

  Game game;
  game_init(&game);
  float snake_timer = 0.f;
  float snake_speed = 10.f; // moves per second
  enum Direction current_direction = LEFT;
  enum Direction queued_direction = NONE;

  while (!WindowShouldClose()) {
    handle_input(current_direction, &queued_direction);

    float dt = GetFrameTime();
    snake_timer += dt;
    if (snake_timer >= 1.f / snake_speed) {
      if (queued_direction != NONE) {
        current_direction = queued_direction;
        queued_direction = NONE;
      }

      if (!game_update(&game, current_direction)) {
        break;
      }
      snake_timer = 0.f;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    grid_draw(game.grid);
    EndDrawing();
  }
  game_destroy(&game);
  CloseWindow();
}
