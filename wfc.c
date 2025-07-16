#include <windows.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>

#include "wfc.h"

#define CELL_FRAG_SIZE 20
#define FRAGMENTS_PER_CELL_SIDE 3
#define CELL_SIZE (CELL_FRAG_SIZE * FRAGMENTS_PER_CELL_SIDE)

#define GRID_WIDTH_START 15
#define GRID_HEIGHT_START 10

#define WINDOW_WIDTH_START (GRID_WIDTH_START * CELL_SIZE)
#define WINDOW_HEIGHT_START (GRID_HEIGHT_START * CELL_SIZE)

typedef struct
{
    int row, column;
    COLORREF color;
    enum Surface surface;
} Fragment;

typedef struct
{
    int x, y;
    Fragment fragments[FRAGMENTS_PER_CELL_SIDE][FRAGMENTS_PER_CELL_SIDE];
} Cell;

unsigned grid_height = GRID_HEIGHT_START;
unsigned grid_width = GRID_WIDTH_START;
Cell **grid;

FILE *logFile = NULL;

// Custom logging function
void log_C(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args); // Print to console

    if (logFile)
    {
        vfprintf(logFile, fmt, args); // Also write to file
        fflush(logFile);
    }

    va_end(args);
}

// Generate a random RGB color
COLORREF GenerateRandomColor()
{
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

COLORREF GenerateSurfaceColor(enum Surface surface)
{
    return RGB((rand() % surfaceColorToClamp[surface].clamps[RED].range) + surfaceColorToClamp[surface].clamps[RED].floor,
               (rand() % surfaceColorToClamp[surface].clamps[GREEN].range) + surfaceColorToClamp[surface].clamps[GREEN].floor,
               (rand() % surfaceColorToClamp[surface].clamps[BLUE].range) + surfaceColorToClamp[surface].clamps[BLUE].floor);
}

void GetCellSockets(enum Surface CellSocks[FRAGMENTS_PER_CELL_SIDE], int x, int y, enum Direction side)
{

    if (x < 0 || y < 0 || x >= grid_width || y >= grid_height)
    {
        for (size_t i = 0; i < FRAGMENTS_PER_CELL_SIDE; i++)
        {
            CellSocks[i] = rand() % SURFACE_COUNT;
            log_C("RAND s %d\n", side);
        }
        return;
    }

    int row_off = side == UP ? 2 : 0;
    int col_off = side == LEFT ? 2 : 0;

    int row_mul = side > RIGHT ? 0 : 1;
    int col_mul = 1 - row_mul;
    log_C("--- ro %d rm %d co %d cm %d side %d\n", row_off, row_mul, col_off, col_mul, side);

    for (size_t i = 0; i < FRAGMENTS_PER_CELL_SIDE; i++)
    {
        log_C(">> s: %d x %d y %d r %d c %d surf: %d\n", side, x, y, i * row_mul + row_off * col_mul, i * col_mul + col_off * row_mul, grid[x][y].fragments[i * row_mul + row_off * col_mul][i * col_mul + col_off * row_mul].surface);
        CellSocks[i] = grid[x][y].fragments[i * row_mul + row_off * col_mul][i * col_mul + col_off * row_mul].surface;
    }
}

void GenRoadShape(enum Surface Road[FRAGMENTS_PER_CELL_SIDE][FRAGMENTS_PER_CELL_SIDE], enum Surface Sockets[DIR_COUNT][FRAGMENTS_PER_CELL_SIDE])
{
    for (size_t side = 0; side < DIR_COUNT; side++)
    {
        for (size_t i = 0; i < FRAGMENTS_PER_CELL_SIDE; i++)
        {
            int row_off = side == UP ? 2 : 0;
            int col_off = side == LEFT ? 2 : 0;

            int row_mul = side > RIGHT ? 0 : 1;
            int col_mul = 1 - row_mul;
            log_C("!! ro %d rm %d co %d cm %d side %d\n", row_off, row_mul, col_off, col_mul, side);

            for (size_t i = 0; i < FRAGMENTS_PER_CELL_SIDE; i++)
            {
                if (Sockets[side][i] == ROAD)
                    Road[i * row_mul + row_off * col_mul][i * col_mul + col_off * row_mul] = ROAD;
            }
        }
    }
}

void GenerateTile(int x, int y)
{
    enum Surface Sockets[DIR_COUNT][FRAGMENTS_PER_CELL_SIDE];

    GetCellSockets(Sockets[UP], x - 1, y, UP);
    GetCellSockets(Sockets[DOWN], x + 1, y, DOWN);
    GetCellSockets(Sockets[LEFT], x, y - 1, LEFT);
    GetCellSockets(Sockets[RIGHT], x, y + 1, RIGHT);

    for (size_t d = 0; d < FRAGMENTS_PER_CELL_SIDE; d++)
    {
        log_C("u: %d \n", Sockets[UP][d]);
        log_C("d: %d \n", Sockets[DOWN][d]);
        log_C("l: %d \n", Sockets[LEFT][d]);
        log_C("r: %d \n", Sockets[RIGHT][d]);
    }

    enum Surface Road[FRAGMENTS_PER_CELL_SIDE][FRAGMENTS_PER_CELL_SIDE];

    GenRoadShape(Road, Sockets);

    for (size_t i = 0; i < FRAGMENTS_PER_CELL_SIDE; i++)
    {
        for (size_t j = 0; j < FRAGMENTS_PER_CELL_SIDE; j++)
        {
            grid[x][y].fragments[i][j].surface = Road[i][j];
            grid[x][y].fragments[i][j].color = GenerateSurfaceColor(Road[i][j]);
        }
    }
}

int AllignToGrid(int num)
{
    log_C("%d %d\n", num - (num % CELL_SIZE), num);
    return num - (num % CELL_SIZE);
}

void InitGrid()
{
    grid = calloc(grid_height, sizeof(Cell *));
    for (size_t i = 0; i < grid_height; i++)
    {
        grid[i] = calloc(grid_width, sizeof(Cell));

        for (size_t j = 0; j < grid_width; j++)
        {
            grid[i][j].y = i * CELL_SIZE;
            grid[i][j].x = j * CELL_SIZE;
            for (size_t r = 0; r < FRAGMENTS_PER_CELL_SIDE; r++)
            {
                for (size_t c = 0; c < FRAGMENTS_PER_CELL_SIDE; c++)
                {
                    grid[i][j].fragments[r][c].color = RGB(255, 255, 255);
                    grid[i][j].fragments[r][c].surface = SURFACE_COUNT;
                }
            }
        }
    }
}

void DeleteGrid()
{
    for (size_t i = 0; i < grid_height; i++)
    {
        free(grid[i]);
    }
    free(grid);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        srand((unsigned int)time(NULL)); // Seed RNG
        break;

    case WM_LBUTTONDOWN:
    {

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        int grid_x = AllignToGrid(x);
        int grid_y = AllignToGrid(y);

        int row = grid_y / CELL_SIZE;
        int column = grid_x / CELL_SIZE;

        log_C("%d %d %d %d\n", grid_x, grid_y, column, row);

        GenerateTile(row, column);

        InvalidateRect(hwnd, NULL, TRUE); // Redraw window

        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        for (int i = 0; i < grid_height; ++i)
        {
            for (int j = 0; j < grid_width; ++j)
            {
                int x = grid[i][j].x;
                int y = grid[i][j].y;
                for (size_t r = 0; r < FRAGMENTS_PER_CELL_SIDE; r++)
                {
                    for (size_t c = 0; c < FRAGMENTS_PER_CELL_SIDE; c++)
                    {
                        HBRUSH brush = CreateSolidBrush(grid[i][j].fragments[r][c].color);
                        HBRUSH oldBrush = SelectObject(hdc, brush);

                        HPEN pen = CreatePen(PS_SOLID, 0, grid[i][j].fragments[r][c].color);
                        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

                        Rectangle(hdc, x + c * CELL_FRAG_SIZE, y + r * CELL_FRAG_SIZE, x + c * CELL_FRAG_SIZE + CELL_FRAG_SIZE, y + r * CELL_FRAG_SIZE + CELL_FRAG_SIZE);

                        SetTextColor(hdc, RGB(0, 0, 0));
                        TextOut(hdc, x, y, "x", lstrlenA("x"));

                        SelectObject(hdc, oldPen);
                        SelectObject(hdc, oldBrush);
                        DeleteObject(pen);
                        DeleteObject(brush);
                    }
                }
            }
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "ClickDrawWindow";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    RECT rect = {0, 0, WINDOW_WIDTH_START, WINDOW_HEIGHT_START};
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Fixed-Size Client Area",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return 0;

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    log_C("Debug console initialized!\n");

    logFile = fopen("debuglog.txt", "w");
    if (!logFile)
    {
        log_C("Failed to open log file.\n");
    }

    InitGrid();

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteGrid();

    return 0;
}
