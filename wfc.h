enum Surface
{
    ROAD,
    GRASS,
    SAND,
    SURFACE_COUNT
};

enum Color
{
    RED,
    BLUE,
    GREEN,
    COLOR_COUNT
};

enum Direction
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    DIR_COUNT
};

typedef struct
{
    int floor;
    int range; // must be > 0
} Clamp;

typedef struct
{
    Clamp clamps[COLOR_COUNT];
} RgbClamps;

RgbClamps surfaceColorToClamp[SURFACE_COUNT] = {
    [ROAD] = {{{100, 1}, {0, 4}, {5, 6}}},
    [GRASS] = {{{0, 1}, {100, 1}, {0, 1}}},
    [SAND] = {{{0,1}, {0, 1}, {200, 1}}}, 
};
