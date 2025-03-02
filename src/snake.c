#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#define UNIT 10
#define WIN_H_UN 30
#define WIN_W_UN 30

#define WIN_H_PX UNIT * WIN_H_UN 
#define WIN_W_PX UNIT * WIN_W_UN

#define BACKGROUND_RGB 0,   0, 0, 1
#define SNAKE_RGB      0, 255, 0, 1
#define APPLE_RGB      255, 0, 0, 1

#define SNAKE_START_X 45
#define SNAKE_START_Y 30
#define SNAKE_START_X_D -1
#define SNAKE_START_Y_D 0
#define SNAKE_INIT_SIZE 3

#define FPS 10

typedef struct snake_node {
    int x, y, x_dir, y_dir;
    struct snake_node* next;
} snake_node;

typedef struct {
    snake_node* head;
    snake_node* tail;
    int size;
} snake;

typedef struct {
    int x, y;
    bool exists;
} apple;

typedef struct {
    snake* snake;
    apple* apple;
} game_state;

void handle_input(SDL_Keysym keysym, snake_node* head);
bool is_intersecting_snake(int x, int y, snake_node* tail);

game_state* init_state(snake* snake, apple* apple);
apple* init_apple();
snake* create_snake();
void grow_snake(snake* snake);
void destroy_snake(snake* snake);

void update_state(game_state* state);
void display(SDL_Renderer* renderer, game_state* state);

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W_PX, WIN_H_PX, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    snake* snake = create_snake();
    apple* apple = init_apple();
    game_state* state = init_state(snake, apple);

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        bool key_pressed = false;

        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (!key_pressed) {
                        key_pressed = true;
                        handle_input(event.key.keysym, snake->head);
                    }
            }
        }

        update_state(state);
        snake_node* head = snake->head;
        for (snake_node* node = snake->tail; node != head; node = node->next) {
            if (node->x == head->x && node->y == head->y) {
                quit = true;
            }
        }

        display(renderer, state);
        SDL_Delay(1000/FPS);
    }

    destroy_snake(snake);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

apple* init_apple() {
    apple* ap = malloc(sizeof(apple));

    if (ap == NULL) {
        fprintf(stderr, "Failed to allocate memory for apple initializing");
        exit(3);
    }

    ap->exists = false;

    return ap;
}

game_state* init_state(snake* snake, apple* apple) {
    game_state* state = malloc(sizeof(game_state));

    if (state == NULL) {
        fprintf(stderr, "Failed to allocate memory for apple initializing");
        exit(3);
    }

    state->snake = snake;
    state->apple = apple;

    return state;
}

void display(SDL_Renderer* renderer, game_state* state) {
    SDL_SetRenderDrawColor(renderer, BACKGROUND_RGB);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, APPLE_RGB);
    SDL_Rect point = { .x = state->apple->x, .y = state->apple->y, .w = UNIT, .h = UNIT };
    SDL_RenderDrawRect(renderer, &point);

    SDL_SetRenderDrawColor(renderer, SNAKE_RGB);
    for (snake_node* node = state->snake->tail; node != NULL; node = node->next) {
        SDL_Rect point = { .x = node->x, .y = node->y, .w = UNIT, .h = UNIT };
        SDL_RenderDrawRect(renderer, &point);
    }

    SDL_RenderPresent(renderer);
}


void update_state(game_state* state) {
    snake* snake = state->snake;

    // move snake
    snake_node* tail = snake->tail;
    for (snake_node* node = tail; node != NULL; node =  node->next) {
        snake_node* next = node->next;

        if (next != NULL) {
            node->x = node->next->x;
            node->y = node->next->y;
            node->x_dir = node->next->x_dir;
            node->y_dir = node->next->y_dir;
        }
    }

    // move head
    snake_node* head = snake->head;
    if (head->x_dir == 1) {
        head->x += UNIT;
    } else if (head->x_dir == -1) {
        head->x -= UNIT;
    }
    if (head->x > WIN_W_PX - 10) head->x = 0;
    if (head->x < 0) head->x = WIN_W_PX - 10;

    if (head->y_dir == 1) {
        head->y += UNIT;
    } else if (head->y_dir == -1) {
        head->y -= UNIT;
    }
    if (head->y > WIN_H_PX - 10) head->y = 0;
    if (head->y < 0) head->y = WIN_H_PX - 10;
    
    // process apple
    apple* apple = state->apple;
    if (apple->exists) {
        if ((snake->head->x == apple->x) && (snake->head->y == apple->y)) {
            grow_snake(snake);
            apple->exists = false;
        }
    } else {
        srand(time(NULL));

        do {
            apple->x = rand() % WIN_W_PX;
            apple->y = rand() % WIN_H_PX;
        } while (is_intersecting_snake(apple->x, apple->y, snake->tail));
        apple->exists = true;
    }
}

snake* create_snake() {
    snake* sn = malloc(sizeof(snake));
    if (sn == NULL) {
        fprintf(stderr, "Failed to allocate memory for snake");
        exit(2);
    }

    //create head
    snake_node* head = malloc(sizeof(snake_node));
    if (head == NULL) {
        fprintf(stderr, "Failed to allocate memory for snake head");
        exit(2);
    }

    head->next = NULL;
    head->x = SNAKE_START_X;
    head->y = SNAKE_START_Y;
    head->x_dir = SNAKE_START_X_D;
    head->y_dir = SNAKE_START_Y_D;

    sn->head = head;
    sn->size = SNAKE_INIT_SIZE;

    //create tail
    snake_node* tail = malloc(sizeof(snake_node));
    if (tail == NULL) {
        fprintf(stderr, "Failed to allocate memory for snake tail");
        exit(2);
    }

    tail->next = head;
    tail->x_dir = head->x_dir;
    tail->y_dir = head->y_dir;

    tail->x = SNAKE_START_X + 1;
    tail->y = SNAKE_START_Y;

    if (head->y_dir == 0) {
        if (head->x_dir == 1) {
            tail->x = (head->x) - 1;
        } else {
            tail->x = (head->x) + 1;
        }
        tail->y = head->y;
    } else if (head->x_dir == 0) {
        if (head->y_dir == 1) {
            tail->y = (head->y) - 1;
        } else {
            tail->y = (head->y) + 1;
        }
        tail->x = head->x;
    }

    sn->tail = tail;

    //create body
    for (int i = 2; i < SNAKE_INIT_SIZE; i++) {
        grow_snake(sn);
    }

    return sn;
}

void grow_snake(snake* snake) {
    snake_node* tail = snake->tail;

    snake_node* new_node = malloc(sizeof(snake_node));
    if (new_node == NULL) {
        fprintf(stderr, "Failed to allocate memory for snake node");
        exit(2);
    }

    new_node->next = (struct snake_node*) tail;

    new_node->x_dir = tail->x_dir;
    new_node->y_dir = tail->y_dir;

    if (tail->y_dir == 0) {
        if (tail->x_dir == 1) {
            new_node->x = (tail->x) - 1;
        } else {
            new_node->x = (tail->x) + 1;
        }
        new_node->y = tail->y;
    }

    if (tail->x_dir == 0) {
        if (tail->y_dir == 1) {
            new_node->y = (tail->y) - 1;
        } else {
            new_node->y = (tail->y) + 1;
        }
        new_node->x = tail->x;
    }

    snake->tail = new_node;
    snake->size++;
}

bool is_intersecting_snake(int x, int y, snake_node* tail) {
    for (snake_node* node = tail; node != NULL; node = node->next) {
        if (node->x == x && node->y == y) {
            return true;
        }
    }

    return false;
}

void handle_input(SDL_Keysym keysym, snake_node* head) {
    SDL_Scancode scancode = keysym.scancode;

    if (keysym.sym == 'q') exit(0);

    switch (scancode) {
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:
            if (head->y_dir == 0) {
                head->y_dir = -1;
                head->x_dir = 0;
            }
            break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:
            if (head->x_dir == 0) {
                head->x_dir = -1;
                head->y_dir = 0;
            }
            break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:
            if (head->y_dir == 0) {
                head->y_dir = 1;
                head->x_dir = 0;
            }
            break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT:
            if (head->x_dir == 0) {
                head->x_dir = 1;
                head->y_dir = 0;
            }
            break;
        default:
            break;
    }
}

// TODO
void destroy_snake(snake* snake) {
    /* free(snake->head); */
    /* free(snake->tail); */

    /* // TODO */
    /* snake_node* node =  snake->head->prev; */
    /* for (; node != NULL; node =  node->prev) { */
    /*     free(node); */
    /* } */

    /* free(snake); */
}
