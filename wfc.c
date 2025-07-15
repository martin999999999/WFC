#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CELLS 1000
#define CELL_SIZE 40

#define WINDOW_WIDTH_START 1000
#define WINDOW_HEIGHT_START 800

#define GRID_WIDTH_START WINDOW_WIDTH_START / CELL_SIZE
#define GRID_HEIGHT_START WINDOW_HEIGHT_START / CELL_SIZE

typedef struct
{
    int x, y;
    COLORREF color;
} Cell;

Cell squares[MAX_CELLS];
int square_count = 0;

Cell **grid;

// Generate a random RGB color
COLORREF GenerateRandomColor()
{
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

int AllignToGrid(int num)
{
    return num - (num % CELL_SIZE);
}

void InitGrid()
{
    grid = calloc(WINDOW_HEIGHT_START, sizeof(Cell *));
    for (size_t i = 0; i < WINDOW_HEIGHT_START; i++)
    {
        grid[i] = calloc(WINDOW_WIDTH_START, sizeof(Cell));

        for (size_t j = 0; j < WINDOW_WIDTH_START; j++)
        {
            grid[i][j].x = i * 40;
            grid[i][j].y = j * 40;
        }
    }
}

void DeleteGrid()
{
    for (size_t i = 0; i < WINDOW_HEIGHT_START; i++)
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
        if (square_count < MAX_CELLS)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            squares[square_count].x = AllignToGrid(x);
            squares[square_count].y = AllignToGrid(y);
            squares[square_count].color = GenerateRandomColor();
            square_count++;

            InvalidateRect(hwnd, NULL, TRUE); // Redraw window
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        for (int i = 0; i < square_count; ++i)
        {
            HBRUSH brush = CreateSolidBrush(squares[i].color);
            HBRUSH old = SelectObject(hdc, brush);

            int x = squares[i].x;
            int y = squares[i].y;
            Rectangle(hdc, x, y, x + CELL_SIZE, y + CELL_SIZE);

            SelectObject(hdc, old);
            DeleteObject(brush);
        }

        for (int i = 0; i < square_count; ++i)
        {
            for (int j = 0; j < square_count; ++j)
            {
                HBRUSH brush = CreateSolidBrush(squares[i].color);
                HBRUSH old = SelectObject(hdc, brush);

                int x = grid[i][j].x;
                int y = grid[i][j].y;

                // Draw label inside square
                SetTextColor(hdc, RGB(0, 0, 0)); // Black text
                TextOut(hdc, x, y, "x", lstrlenA("x"));

                SelectObject(hdc, old);
                DeleteObject(brush);
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

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Draw Squares on Click",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // No resize, no maximize
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH_START, WINDOW_HEIGHT_START,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return 0;

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
