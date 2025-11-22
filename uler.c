#include <curses.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SEGMENTS 1482
#define FRAME_TIME 220
#define COLOR_NAVYBLUE 10
#define COLOR_GREENGRASS 11

//==================== structs & vars ====================//
typedef struct
{
    int x;
    int y;

} vec2;

// score
int score = 0;
char score_str[16];

// game state
bool skip = false;
bool is_running = true;

// window
WINDOW *win;

// screen size
int screen_width = 59;
int screen_height = 28;

// snake
vec2 head = {0, 0};
vec2 dir = {1, 0};
vec2 segments[MAX_SEGMENTS + 1];

// fruit
vec2 fruit;

// pause
int is_paused = 0;

//========================================================//

//==================== function prototypes ====================//
bool collide(vec2 a, vec2 b);
bool collide_snake_body(vec2 point);
vec2 spawn_fruit();
void draw_border(int y, int x, int width, int height);
void draw_border2(int y, int x, int width, int height);
void print_art(int y, int x);
int center_x(const char *text, int w);
void quit_game();
void restart_game();
void set_console_char_size(short cols, short rows);
void init();
int input();
void game_over();
void update();
void draw();
void print_art();
int menu();
void draw_pause();
int GAME();
int credit();
//============================================================//

//============================== Game Logic Function ==============================//

// collision
bool collide(vec2 a, vec2 b)
{
    if (a.x == b.x && a.y == b.y)
    {
        return true;
    }
    else
        return false;
}

bool collide_snake_body(vec2 point)
{
    for (int i = 0; i < score; i++)
    {
        if (collide(point, segments[i]))
        {
            return true;
        }
    }
    return false;
}

// fruit spawn
vec2 spawn_fruit()
{
    // spawn a new fruit with 1 pixel padding from edges and not inside of the snake
    vec2 fruit = {1 + rand() % (screen_width - 2), 1 + rand() % (screen_height - 2)};
    while (collide(head, fruit) || collide_snake_body(fruit))
    {
        fruit.x = 1 + rand() % (screen_width - 2);
        fruit.y = 1 + rand() % (screen_height - 2);
    }
    return fruit;
}

// update function
void update()
{
    // update snake segments
    for (int i = score; i > 0; i--)
    {
        segments[i] = segments[i - 1];
    }
    segments[0] = head;

    // move snake
    head.x += dir.x;
    head.y += dir.y;

    // collide with body / wall
    if (collide_snake_body(head) || head.x < 0 || head.y < 0 || head.x >= screen_width || head.y >= screen_height)
    {
        is_running = false;
        game_over();
    }

    // eating fruit
    if (collide(head, fruit))
    {
        if (score + 1 <= MAX_SEGMENTS)
        {
            score += 1;
            sprintf(score_str, "SCORE: %d", score);
            if (score >= MAX_SEGMENTS)
            {
                is_running = false;
                draw();
                you_win();
                return;
            }
        }

        fruit = spawn_fruit();
    }

    Sleep(FRAME_TIME);
}

//============================== Game Logic Function ==============================//

//============================== Draw Function ==============================//

// border in game
void draw_border(int y, int x, int width, int height)
{
    // text score
    char score_str[32];
    sprintf(score_str, "[ SCORES: %4d ]", score);
    int score_len = strlen(score_str);

    // position of text score
    int top_width = width * 2 + 1;
    int score_x = (x + (top_width - score_len) / 2);

    // top row
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + top_width, ACS_URCORNER);

    mvaddch(y, x, ACS_ULCORNER);
    for (int i = 1; i < top_width; i++)
        mvaddch(y, x + i, ACS_HLINE);
    mvaddch(y, x + top_width, ACS_URCORNER);

    // timpa garis dengan string score (garis tidak dihitung lagi)
    mvprintw(y, score_x + 1, "%s", score_str);

    // vertical lines
    for (int i = 0; i < height + 1; i++)
    {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width * 2 + 1, ACS_VLINE);
    }
    // bottom row
    mvaddch(y + height + 1, x, ACS_LLCORNER);
    mvaddch(y + height + 1, x + width * 2 + 1, ACS_LRCORNER);
    for (int i = 1; i < width * 2 + 1; i++)
    {
        mvaddch(y + height + 1, x + i, ACS_HLINE);
    }
}

// border in game for all purpose
void draw_border2(int y, int x, int width, int height)
{
    int top_width = width * 2 + 1;
    // TOP ROW
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + top_width, ACS_URCORNER);

    for (int i = 1; i < top_width; i++)
    {
        mvaddch(y, x + i, ACS_HLINE);
    }

    // vertical lines
    for (int i = 0; i < height + 1; i++)
    {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width * 2 + 1, ACS_VLINE);
    }
    // bottom row
    mvaddch(y + height + 1, x, ACS_LLCORNER);
    mvaddch(y + height + 1, x + width * 2 + 1, ACS_LRCORNER);
    for (int i = 1; i < width * 2 + 1; i++)
    {
        mvaddch(y + height + 1, x + i, ACS_HLINE);
    }
}

// draw function
void draw()
{
    erase();

    // draw fruit
    attron(COLOR_PAIR(1));
    mvaddch(fruit.y + 1, fruit.x * 2 + 1, '@');
    attroff(COLOR_PAIR(1));

    // draw snake
    attron(COLOR_PAIR(2));
    for (int i = 0; i < score; i++)
    {
        mvaddch(segments[i].y + 1, segments[i].x * 2 + 1, ACS_DIAMOND);
    }
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(2));
    mvaddch(head.y + 1, head.x * 2 + 1, 'O');
    attroff(COLOR_PAIR(2));

    // draw score etc'
    attron(COLOR_PAIR(3));
    draw_border(0, 0, screen_width, screen_height);
    attroff(COLOR_PAIR(3));
}

void draw_pause()
{
    // gunakan draw_border di atas layar
    attron(COLOR_PAIR(3));
    draw_border2(screen_height / 2 - 2, screen_width - 17, 17, 3);
    mvprintw(screen_height / 2 - 2, screen_width - 8, "[  GAME  PAUSED  ]");
    attroff(COLOR_PAIR(3));

    // tulis teks pause di bar atas
    mvprintw(screen_height / 2, screen_width - 14, "Press [BACKSPACE] to Continue!");
    refresh();
}

// ascii art
void print_art(int y, int x)
{
    const char *art[] = {
        "  ______   __    __   ______   __    __  ________  _______   ________   ______   __    __ ",
        " /      \\ /  \\  /  | /      \\ /  |  /  |/        |/       \\ /        | /      \\ /  |  /  |",
        "/$$$$$$  |$$  \\ $$ |/$$$$$$  |$$ | /$$/ $$$$$$$$/ $$$$$$$  |$$$$$$$$/ /$$$$$$  |$$ | /$$/ ",
        "$$ \\__$$/ $$$  \\$$ |$$ |__$$ |$$ |/$$/  $$ |__    $$ |__$$ |$$ |__    $$ |__$$ |$$ |/$$/  ",
        "$$      \\ $$$$  $$ |$$    $$ |$$  $$<   $$    |   $$    $$/ $$    |   $$    $$ |$$  $$<   ",
        " $$$$$$  |$$ $$ $$ |$$$$$$$$ |$$$$$  \\  $$$$$/    $$$$$$$/  $$$$$/    $$$$$$$$ |$$$$$  \\  ",
        "/  \\__$$ |$$ |$$$$ |$$ |  $$ |$$ |$$  \\ $$ |_____ $$ |      $$ |_____ $$ |  $$ |$$ |$$  \\ ",
        "$$    $$/ $$ | $$$ |$$ |  $$ |$$ | $$  |$$       |$$ |      $$       |$$ |  $$ |$$ | $$  |",
        " $$$$$$/  $$/   $$/ $$/   $$/ $$/   $$/ $$$$$$$$/ $$/       $$$$$$$$/ $$/   $$/ $$/   $$/ "};

    int lines = sizeof(art) / sizeof(art[0]);

    for (int i = 0; i < lines; i++)
    {
        mvprintw(y + i, x, "%s", art[i]);
    }
}

//============================== Draw Function ==============================//

//============================== Fature For Quit, Paused, Restart, Game over, win Function ==============================//

// quit game function
void quit_game()
{
    // exit cleanly from application
    endwin();
    // clear screen, place cursor on top, and un_hide cursor
    printf("\e[1;1H\e[2J");
    printf("\e[?25h");
    exit(0);
}

// restart function
void restart_game()
{
    // reset score
    score = 0;
    sprintf(score_str, "[SCORE: %d]", score);

    // reset head position
    head.x = 0;
    head.y = 0;

    // reset snake direction
    dir.x = 1;
    dir.y = 0;

    // reset flag
    skip = false;
    is_running = true;

    // clear all old segment positions
    for (int i = 0; i <= MAX_SEGMENTS; i++)
    {
        segments[i].x = 0;
        segments[i].y = 0;
    }

    // reset pause
    is_paused = 0;

    // spawn new fruit
    fruit = spawn_fruit();

    segments[0] = head;
}

// game over function
void game_over()
{
    while (is_running == false)
    {
        int event = input();

        mvaddstr(screen_height / 2, screen_width - 4, "Game  Over");
        mvaddstr(screen_height / 2 + 1, screen_width - 15, "[SPACE] to restart,[ESC] to quit");
        attron(COLOR_PAIR(3));
        draw_border(screen_height / 2 - 1, screen_width - 17, 17, 2);
        attroff(COLOR_PAIR(3));

        if (event == 1)
        {
            return;
        }

        Sleep(FRAME_TIME);
    }
}

// win function
void you_win()
{
    while (is_running == false)
    {
        int event = input();

        mvaddstr(screen_height / 2, screen_width - 4, "YOU  WIN");
        mvaddstr(screen_height / 2 + 1, screen_width - 16, "[SPACE] to restart, [ESC] to quit");
        attron(COLOR_PAIR(3));
        draw_border(screen_height / 2 - 1, screen_width - 17, 17, 2);
        attroff(COLOR_PAIR(3));

        if (event == 1)
        {
            return;
        }

        Sleep(FRAME_TIME);
    }
}

//============================== Fature For Quit, Paused, Restart, Game over, win Function ==============================//

//============================== Setup Terminal Function ==============================//

// lock console
void set_console_char_size(short cols, short rows)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;

    // Step 1: Besarkan buffer dulu (aman)
    COORD bufferSize = {cols, rows};
    SetConsoleScreenBufferSize(hOut, bufferSize);

    // Step 2: Set window size sesuai buffer
    SMALL_RECT windowSize = {0, 0, cols - 1, rows - 1};
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);

    // Step 3: Kunci resize & maximize
    HWND hwnd = GetConsoleWindow();
    if (!hwnd)
        return;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_MAXIMIZEBOX; // hapus tombol maximize
    style &= ~WS_SIZEBOX;     // matikan resize via drag
    SetWindowLong(hwnd, GWL_STYLE, style);

    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOZORDER | SWP_FRAMECHANGED);
}

// initialization function
void init()
{
    srand(time(NULL));

    // initialize window
    win = initscr();

    // player input
    keypad(win, TRUE);
    nodelay(win, TRUE);
    noecho();
    curs_set(0);

    // initializze colors
    if (has_colors() == false)
    {
        endwin();
        fprintf(stderr, "Your terminal does not support color\n");
        exit(1);
    }
    start_color();

    init_color(COLOR_GREENGRASS, 78, 639, 114);
    init_color(COLOR_NAVYBLUE, 27, 12, 369);

    can_change_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(10, COLOR_NAVYBLUE, -1);
    init_pair(11, COLOR_GREENGRASS, -1);

    fruit.x = 1 + rand() % (screen_width - 2);
    fruit.y = 1 + rand() % (screen_height - 2);

    // update score string
    sprintf(score_str, "[   SCORE: %d   ]", score);
}

//============================== Setup Terminal Function ==============================//

//============================== User Input Function ==============================//

// input function
int input()
{
    int pressed = wgetch(win);

    int dx = dir.x;
    int dy = dir.y;

    int nx = head.x;
    int ny = head.y;

    if (pressed == KEY_LEFT)
    {

        nx = head.x - 1;

        if (dx == 1 || nx < 0)
        {
            skip = true;
            return 0;
        }

        dir.x = -1;
        dir.y = 0;
    }
    else if (pressed == KEY_RIGHT)
    {
        nx = head.x + 1;

        if (dx == -1 || nx >= screen_width)
        {
            skip = true;
            return 0;
        }

        dir.x = 1;
        dir.y = 0;
    }
    else if (pressed == KEY_UP)
    {
        ny = head.y - 1;

        if (dy == 1 || ny < 0)
        {
            skip = true;
            return 0;
        }

        dir.x = 0;
        dir.y = -1;
    }
    else if (pressed == KEY_DOWN)
    {
        ny = head.y + 1;

        if (dy == -1 || ny >= screen_height)
        {
            skip = true;
            return 0;
        }

        dir.x = 0;
        dir.y = 1;
    }
    else if (pressed == ' ')
    {
        if (!is_running)
            restart_game();
    }
    else if (pressed == 27)
    {
        if (is_running)
        {
            return 0;
        }
        else
        {
            is_running = false;
            return 1;
        }
    }
    else if (pressed == 'p' || pressed == 'P')
    {
        is_paused = 1;
    }
    else if (pressed == 8)
    {
        is_paused = 0;
    }

    return 0;
}

//============================== User Input Function ==============================//

//============================== User Interface Function ==============================//

// centered text
int center_x(const char *text, int w)
{
    return (w) - (strlen(text) / 2);
}

// menu function
int menu()
{
    int menu_width = 20;
    int menu_height = 5;
    int start_y = screen_height / 2 + 5;
    int start_x = screen_width / 2 + 10;

    const char *choices[3] = {"START GAME", "CREDIT", "QUIT"};
    int num_choice = 3;
    int highlight = 0;
    int input_menu;
    int extra_space = 0;

    timeout(50);

    while (1)
    {
        erase();

        attron(COLOR_PAIR(3));
        draw_border2(0, 0, screen_width, screen_height);
        draw_border2(start_y, start_x, menu_width, menu_height);
        attroff(COLOR_PAIR(3));

        mvprintw(screen_height, 1, "By : COW");

        // print menu choicesP
        for (int i = 0; i < num_choice; i++)
        {
            // center
            int text_len = strlen(choices[i]);
            int centered_x = (start_x + (menu_width - text_len) / 2) + 9;

            if (i == highlight)
            {
                attron(A_REVERSE);
                extra_space = 4;
                text_len += 4;
            }
            if (i == highlight)
            {
                mvprintw(start_y + 2 + i, centered_x, "> %s <", choices[i]);
                attroff(A_REVERSE);
            }
            else
            {
                mvprintw(start_y + 2 + i, centered_x, "  %s  ", choices[i]);
            }
        }

        print_art(screen_height / 2 - 9, screen_width / 2 - 14);

        refresh();
        input_menu = getch();

        switch (input_menu)
        {
        case KEY_UP:
            highlight--;
            if (highlight < 0)
                highlight = 2;
            break;
        case KEY_DOWN:
            highlight++;
            if (highlight > 2)
                highlight = 0;
            break;
        case 10: // Enter
            if (highlight == 0)
            {
                return 1; // start game
            }
            if (highlight == 1)
            {
                return 2; // credit
            }
            if (highlight == 2)
            {
                return 0; // quit
            }
            break;
        default:
            break;
        }
    }
}

// game function
int GAME()
{
    restart_game();

    while (is_running)
    {
        int event = input();

        if (is_paused) // jika paused, hanya tampil pesan pause
        {
            draw_pause();
            Sleep(FRAME_TIME);
            continue;
        }

        if (skip == true)
        {
            skip = false;
            continue;
        }

        update();
        draw();

        if (!is_running)
        {
            return 1;
        }
    }
    return 1;
}

// credit function
int credit()
{
    erase();

    int mid_x_border = (screen_width * 2 + 1) / 2;
    int mid_x = (screen_width * 2 + 2) / 2;
    int mid_y = screen_height;
    int mid_y_border = screen_height - 17;

    const char *nama[5] = {
        "RAFIF RAJENDRA",
        "ABIYU ALDY",
        "DEMAS MAHEZA",
        "GIVEN ELYADA",
        "SEBASTIAN BACH"};
    int num_nama = 5;

    timeout(50);

    while (1)
    {
        erase();
        attron(COLOR_PAIR(3));
        draw_border2(0, 0, screen_width, screen_height);
        attroff(COLOR_PAIR(3));

        // print_art(screen_height / 2 - 13, screen_width / 2 - 14);
        draw_border2(mid_y_border, mid_x_border - 24, 24, 10);
        mvprintw(mid_y_border + 1, center_x("CREDIT", mid_x), "CREDIT");

        for (int i = 0; i < num_nama; i++)
        {
            mvprintw(mid_y_border + 3 + i, center_x(nama[i], mid_x), "%s", nama[i]);
        }

        mvprintw(mid_y_border + 9, center_x("THANKS FOR PLAYING", mid_x), "THANKS FOR PLAYING");
        mvprintw(mid_y - 1, 3, "[ESC] to return");

        refresh();

        int pressed = getch();
        if (pressed == 27)
        {
            return 1;
        }
    }
}

//============================== User Interface Function ==============================//

//============================== main Function ==============================//

//  main function
int main(int argc, char const *argv[])
{
    // process user args
    if (argc == 1)
    {
    }
    else if (argc == 3)
    {
        if (!strcmp(argv[1], "-d"))
        {
            if (sscanf(argv[2], "%dx%d", &screen_width, &screen_height) != 2)
            {
                printf("Usage: snake [options]\nOptions:\n -d [width]x[height]"
                       "\tdefine dimensions of the screen\n\nDefault dimensions are 59x25\n");
                exit(1);
            }
        }
    }
    else
    {
        printf("Usage: snake [options]\nOptions:\n -d [width]x[height]"
               "\tdefine dimensions of the screen\n\nDefault dimensions are 59x25\n");
        exit(1);
    }

    set_console_char_size(118, 56);
    init();

    while (1)
    {
        int action = menu();

        if (action == 0)
        {
            break; // quit
        }

        if (action == 1)
        {
            GAME();
        }

        if (action == 2)
        {
            credit();
        }
    }

    quit_game();
    return 0;
}

//============================== User main Function ==============================//