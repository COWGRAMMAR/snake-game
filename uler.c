#include <curses.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define MAX_SEGMENTS 1482
#define FRAME_TIME 220
#define COLOR_NAVYBLUE 10
#define COLOR_GREENGRASS 11
#define MAX_DATA 100

//==================== structs & vars ====================//

// struct variable
typedef struct
{
    int x;
    int y;

} vec2;

// struct leaderboard
typedef struct {
    char nama[50];
    int score;
} leaderboard;

leaderboard board[MAX_DATA];

// banyak data leaderboard
int count;

// score
int score = 20;
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
#define MAX_FRUITS 20
vec2 fruits[MAX_FRUITS];
int fruit_count = 1;

// pause
int is_paused = 0;

//========================================================//

//==================== function prototypes ====================//
bool collide(vec2 a, vec2 b);
bool collide_snake_body(vec2 point);
bool check_fruit_collision(int current_index);
void spawn_fruits();
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
void you_win();
void update();
void draw();
int menu();
void pause();
int GAME();
int credit();
int info();
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

bool check_fruit_collision(int current_index)
{
    // check fruit collision at current index
    for (int i = 0; i < fruit_count; i++)
    {
        if (i != current_index && collide(fruits[current_index], fruits[i]))
        {
            return true;
        }
    }
    return false;
}

// fruit spawn
void spawn_fruits()
{
    fruit_count = 1;
    do
    {
        fruits[0].x = 1 + rand() % (screen_width - 2);
        fruits[0].y = 1 + rand() % (screen_height - 2);
    } while (collide(head, fruits[0]) || collide_snake_body(fruits[0]));
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

    // update the number of fruit based on score
    int new_fruit_count = 1 + (score / 10); // every 10 score, add 1 fruit
    if (new_fruit_count > MAX_FRUITS)
    {
        new_fruit_count = MAX_FRUITS;
    }

    // only add 1 new fruit if new count added
    if (new_fruit_count > fruit_count)
    {
        int fruits_to_add = new_fruit_count - fruit_count;

        for (int i = 0; i < fruits_to_add; i++)
        {
            // spawn new fruit at valid position
            vec2 new_fruit;
            do
            {
                new_fruit.x = 1 + rand() % (screen_width - 2);
                new_fruit.y = 1 + rand() % (screen_height - 2);
            } while (collide(head, new_fruit) || collide_snake_body(new_fruit));

            fruits[fruit_count] = new_fruit;
            fruit_count++;
        }
    }

    // Check eating fruits
    for (int i = 0; i < fruit_count; i++)
    {
        if (collide(head, fruits[i]))
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

                // respawn the eaten fruit
                do
                {
                    fruits[i].x = 1 + rand() % (screen_width - 2);
                    fruits[i].y = 1 + rand() % (screen_height - 2);
                } while (collide(head, fruits[i]) ||
                         collide_snake_body(fruits[i]));
            }
            break; // only eat 1 fruit/frame
        }
    }

    Sleep(FRAME_TIME);
}

// load leaderboard dan sorting
void loadLeaderboard() {
    count = 0; // reset counter

    FILE *fptr = fopen("data.txt", "r");
    if (!fptr) return;

    while (fscanf(fptr, " %49[^;];%d", board[count].nama, &board[count].score) == 2) {
        count++;
    }

    // sorting skor tertinggi ke bawah
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (board[j].score < board[j + 1].score) {
                leaderboard temp = board[j];
                board[j] = board[j + 1];
                board[j + 1] = temp;
            }
        }
    }

    fclose(fptr);
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
    for (int i = 1; i < top_width; i++)
    {
        mvaddch(y, x + i, ACS_HLINE);
    }

    // print score in the middle of top row
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
    for (int i = 0; i < fruit_count; i++)
    {
        mvaddch(fruits[i].y + 1, fruits[i].x * 2 + 1, '@');
    }
    attroff(COLOR_PAIR(1));

    // draw snake head
    attron(COLOR_PAIR(2));
    mvaddch(head.y + 1, head.x * 2 + 1, 'O');
    attroff(COLOR_PAIR(2));

    // draw snake body
    attron(COLOR_PAIR(2));
    for (int i = 0; i < score; i++)
    {
        mvaddch(segments[i].y + 1, segments[i].x * 2 + 1, ACS_DIAMOND);
    }
    attroff(COLOR_PAIR(2));

    // draw score etc'
    attron(COLOR_PAIR(3));
    draw_border(0, 0, screen_width, screen_height);
    attroff(COLOR_PAIR(3));
}

// draw pause function
void pause()
{
    // draw pause menu box
    attron(COLOR_PAIR(3));
    draw_border2(screen_height / 2 - 2, screen_width - 17, 17, 3);
    mvprintw(screen_height / 2 - 2, screen_width - 9, "[  GAME  PAUSED  ]");
    attroff(COLOR_PAIR(3));

    // draw pause menu content
    mvprintw(screen_height / 2, screen_width - 14, "Press [BACKSPACE] to Continue!");
    refresh();

    Sleep(FRAME_TIME);
}

// ascii art function
void print_art(int y, int x)
{
    // asci art is stored in an array
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

    // print ascii art
    for (int i = 0; i < lines; i++)
    {
        mvprintw(y + i, x, "%s", art[i]);
    }
}

void printLeaderboard(int y, int x) {
    nodelay(win, FALSE);
    clear();
    refresh();

    WINDOW *printBoard = newwin(26, 50, y, x); 
    // Tinggi 25 baris, lebar 50 kolom, posisi y=1, x=1 (biar gak mepet)

    timeout(50);

    while(1) 
    {
        char title[] = "[ HALL OF FAME ]";
        int center = (50 - strlen(title)) / 2;

        // gambar table, title dan outer line
        attron(COLOR_PAIR(3));
        draw_border2(0, 0, screen_width, screen_height);     
        attroff(COLOR_PAIR(3));

        wattron(printBoard, COLOR_PAIR(3));
        box(printBoard, 0, 0);
        mvwhline(printBoard, 2, 1, ACS_HLINE, 48);
        mvwprintw(printBoard, 0, center, "%s", title);
        wattroff(printBoard, COLOR_PAIR(3));

        mvwprintw(printBoard, 24, 1, "Press [ESC] to quit");

        // Header kolom
        mvwprintw(printBoard, 1, 2, "RANK   NAME                              SCORE");

        // Tampilkan max 20 data
        for (int i = 0; i < count && i < 20; i++) {
            mvwprintw(printBoard, i + 3, 2, "%2d.    %-20s              %5d",i + 1, board[i].nama, board[i].score);
        }

        //mvwprintw(printBoard, 24, 2, "Tekan apa saja untuk kembali...");
        wrefresh(printBoard);

        int pressed = getch();
        if (pressed == 27) {
            delwin(printBoard);
            nodelay(win, TRUE);
            break;
        }

    }
    
}

void writeLeaderboard(int y, int x) {
    nodelay(win, FALSE);

    clear();
    refresh();

    char name[50];
    int rewriteIndex = -1;

    attron(COLOR_PAIR(3));
    draw_border2(0, 0, screen_width, screen_height);     
    attroff(COLOR_PAIR(3));

    // Window untuk input
    WINDOW *inputBox = newwin(3, 40, y, x); // tinggi 7, lebar 40, posisi sesuai argumen
    box(inputBox, 0, 0);

    mvwprintw(inputBox, 1, 2, "Name : ");
    wrefresh(inputBox);

    echo();
    mvwgetnstr(inputBox, 1, 9, name, sizeof(name) - 1);
    noecho();

    delwin(inputBox);
    clear();
    refresh();

    // cek apakah nama sudah ada
    for (int i = 0; i < count; i++) {
        if (strcmp(board[i].nama, name) == 0) {
            rewriteIndex = i;
            break;
        }
    }

    // jika rewrite → kosongkan dulu file
    if (rewriteIndex != -1) {
        FILE *clearf = fopen("data.txt", "w");
        fclose(clearf);
    }

    FILE *fptr = fopen("data.txt", "a");

    // tulis data
    if (rewriteIndex == -1) {
        fprintf(fptr, "%s;%d\n", name, score);
    } else {
        for (int i = 0; i < count; i++) {
            if (i == rewriteIndex)
                fprintf(fptr, "%s;%d\n", name, score);
            else
                fprintf(fptr, "%s;%d\n", board[i].nama, board[i].score);
        }
    }

    fclose(fptr);

    nodelay(win, TRUE);
}

//============================== Draw Function ==============================//

//============================== Fature For Quit, Paused, Restart, Game over, win, settings Function ==============================//

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

    // clear all old snake segment positions
    for (int i = 0; i <= MAX_SEGMENTS; i++)
    {
        segments[i].x = 0;
        segments[i].y = 0;
    }

    // reset snake direction
    dir.x = 1;
    dir.y = 0;

    // reset flag
    skip = false;
    is_running = true;

    // reset pause
    is_paused = 0;

    // spawn new fruit
    spawn_fruits();

    segments[0] = head;
}

// game over function
void game_over()
{
    while (is_running == false)
    {
        int event = input();

        // draw game over menu box
        attron(COLOR_PAIR(3));
        draw_border(screen_height / 2 - 1, screen_width - 24, 24, 2);
        attroff(COLOR_PAIR(3));

        // draw game over menu content
        mvaddstr(screen_height / 2, screen_width - 4, "Game  Over");
        mvaddstr(screen_height / 2 + 1, screen_width - 22, "[SPACE] to restart, [ESC] to quit, [S] to save");

        // return to menu
        if (event == 1)
        {
            return;
        }

        if (event == 10)
        {
            loadLeaderboard();
            writeLeaderboard(screen_height/2, screen_width - 20);
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

        // draw you win menu box
        attron(COLOR_PAIR(3));
        draw_border(screen_height / 2 - 1, screen_width - 17, 17, 2);
        attroff(COLOR_PAIR(3));

        // draw you win menu content
        mvaddstr(screen_height / 2, screen_width - 3, "YOU  WIN");
        mvaddstr(screen_height / 2 + 1, screen_width - 16, "[SPACE] to restart, [ESC] to quit");

        // return to menu
        if (event == 1)
        {
            return;
        }

        Sleep(FRAME_TIME);
    }
}

//============================== Fature For Quit, Paused, Restart, Game over, win, settings Function ==============================//

//============================== Setup Terminal Function ==============================//

// lock console
void set_console_char_size(short cols, short rows)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;

    // step 1: Increase the buffer first (safe)
    COORD bufferSize = {cols, rows};
    SetConsoleScreenBufferSize(hOut, bufferSize);

    // step 2: Set the window size according to the buffer
    SMALL_RECT windowSize = {0, 0, cols - 1, rows - 1};
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);

    // step 3: lock resize & maximize
    HWND hwnd = GetConsoleWindow();
    if (!hwnd)
        return;

    // setup feature for windows terminal
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_MAXIMIZEBOX; // remove maximize button
    style &= ~WS_SIZEBOX;     // disabled resize via drag
    SetWindowLong(hwnd, GWL_STYLE, style);

    // idk man what is this i just ai it for this whole function🥀🥀🥀
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOZORDER | SWP_FRAMECHANGED);
}

// initialization function
void init()
{
    // initialize extended ascii character
    setlocale(LC_ALL, "");

    // initialize random generator
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

    // custom colors list
    init_color(COLOR_GREENGRASS, 78, 639, 114);
    init_color(COLOR_NAVYBLUE, 27, 12, 369);

    // list if colors to be used in function
    can_change_color();
    use_default_colors();
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(10, COLOR_NAVYBLUE, -1);
    init_pair(11, COLOR_GREENGRASS, -1);

    // spawn fruit for the first time
    spawn_fruits();

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

    // input for left
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

    // input for right
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

    // input for up
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

    // input for down
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

    // input for space bar
    else if (pressed == ' ')
    {
        if (!is_running)
            restart_game();
    }

    // input for esc
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

    // input for backspace
    else if (pressed == 8)
    {
        is_paused = !is_paused;
    }
    
    // input for s (save)
    else if (pressed == 's')
    {
        return 10;
    }

    return 0;
}

//============================== User Input Function ==============================//

//============================== User Interface Function ==============================//

// centered text function
int center_x(const char *text, int w)
{
    return (w) - (strlen(text) / 2);
}

// menu function
int menu()
{
    int menu_width = 20;
    int menu_height = 6;
    int start_y = screen_height / 2 + 4;
    int start_x = screen_width / 2 + 10;

    const char *choices[5] = {"START GAME", "SETTINGS", "HALL OF FAME", "CREDIT", "QUIT"};
    int num_choice = 5;
    int highlight = 0;
    int input_menu;
    int extra_space = 0;

    timeout(50);

    while (1)
    {
        erase();

        // draw main menu box
        attron(COLOR_PAIR(3));
        draw_border2(0, 0, screen_width, screen_height);         // outer box
        draw_border2(start_y, start_x, menu_width, menu_height + 1); // inner box
        attroff(COLOR_PAIR(3));

        // draw creator name
        mvprintw(screen_height, 1, "By : D.R.A.G.S");

        // draw ascii art
        print_art(screen_height / 2 - 9, screen_width / 2 - 14);

        // print menu choicesP
        for (int i = 0; i < num_choice; i++)
        {
            // center
            int text_len = strlen(choices[i]);
            int centered_x = (start_x + (menu_width - text_len) / 2) + 9;

            // highlight
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

        refresh();
        input_menu = getch();

        switch (input_menu)
        {
        case KEY_UP:
            highlight--;
            if (highlight < 0)
                highlight = 4;
            break;
        case KEY_DOWN:
            highlight++;
            if (highlight >= 5)
                highlight = 0;
            break;
        case 10: // Enter
            if (highlight == 0)
            {
                return 1; // start game
            }
            if (highlight == 1)
            {
                return 2; // settings
            }
            if (highlight == 2)
            {
                return 3; // HALL OF FAME
            }
            if (highlight == 3)
            {
                return 4; // credit
            }
            if (highlight == 4)
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

        if (event == 1)
        {
            return 1;
        }

        if (is_paused)
        {
            pause();
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
    int mid_y_border = screen_height - 15;

    int mid_x = (screen_width * 2 + 2) / 2;
    int mid_y = screen_height;

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

        // draw credit menu box
        attron(COLOR_PAIR(3));
        draw_border2(0, 0, screen_width, screen_height);       // outer box
        draw_border2(mid_y_border, mid_x_border - 24, 24, 10); // inner box
        attroff(COLOR_PAIR(3));

        // draw ascii art
        print_art(screen_height / 2 - 13, screen_width / 2 - 14);

        // draw content
        mvprintw(mid_y_border + 1, center_x("CREDIT", mid_x), "CREDIT"); // credit text

        for (int i = 0; i < num_nama; i++)
        {
            mvprintw(mid_y_border + 3 + i, center_x(nama[i], mid_x), "%s", nama[i]); // name text
        }

        mvprintw(mid_y_border + 9, center_x("THANKS FOR PLAYING", mid_x), "THANKS FOR PLAYING"); // thanks text
        mvprintw(mid_y - 1, 3, "[ESC] to return");                                               // esc text

        refresh();

        // back to main menu
        int pressed = getch();
        if (pressed == 27)
        {
            return 1;
        }
    }
}

// info function
int info()
{
    erase();

    int mid_x_border = (screen_width * 2 + 1) / 2;
    int mid_y_border = screen_height - 16;

    int mid_x_h = (screen_width * 2 + 2) / 2;
    int mid_x = ((screen_width * 2 + 2) / 2) - 6;
    int mid_y = screen_height;

    timeout(50);

    while (1)
    {
        erase();

        // draw settings menu box
        attron(COLOR_PAIR(3));
        draw_border2(0, 0, screen_width, screen_height);       // outer box
        draw_border2(mid_y_border, mid_x_border - 24, 24, 11); // inner box
        attroff(COLOR_PAIR(3));

        // draw ascii art
        print_art(screen_height / 2 - 13, screen_width / 2 - 14);

        // draw content
        mvprintw(mid_y_border + 1, center_x("KEYBINDS INFORMATION", mid_x_h), "KEYBINDS INFORMATION"); // keybind text

        mvprintw(mid_y_border + 3, center_x("Move Up        : ", mid_x), "Move Up        : "); // from here
        addch(ACS_UARROW);
        mvprintw(mid_y_border + 4, center_x("Move Down      : ", mid_x), "Move Down      : ");
        addch(ACS_DARROW);
        mvprintw(mid_y_border + 5, center_x("Move Right     : ", mid_x), "Move Right     : ");
        addch(ACS_RARROW);
        mvprintw(mid_y_border + 6, center_x("Move Left      : ", mid_x), "Move Left      : ");
        addch(ACS_LARROW);
        mvprintw(mid_y_border + 8, center_x("Pause Game      : [BACKSPACE]", mid_x + 6), "Pause Game     : [BACKSPACE]");
        mvprintw(mid_y_border + 9, center_x("Return Menu     : [ESC]      ", mid_x + 6), "Return Menu    : [ESC]      ");
        mvprintw(mid_y_border + 10, center_x("Start/Enter    : [ENTER]     ", mid_x + 6), "Start/Enter    : [ENTER]     ");
        mvprintw(mid_y - 1, 3, "[ESC] to return"); // to here is keybind information

        refresh();

        // back to main menu
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

    // set up windows terimnal and initialize game
    set_console_char_size(118, 56);
    init();

    // main menu logic
    while (1)
    {
        int action = menu();

        if (action == 0)
        {
            break; // quit game
        }

        if (action == 1)
        {
            GAME(); // game
        }

        if (action == 2)
        {
            info(); // setiings
        }
        if (action == 3)
        {
            loadLeaderboard();
            printLeaderboard(screen_height/2 - 12, screen_width - 25);
        }
        if (action == 4)
        {
            credit(); // credit
        }
    } 

    quit_game();
    return 0;
}

//============================== User main Function ==============================//