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
    GREEN,
    BLUE,
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
    [ROAD] = {{{0, 10}, {0, 40}, {5, 60}}},
    [GRASS] = {{{0, 1}, {100, 50}, {0, 10}}},
    [SAND] = {{{200,10}, {100, 10}, {0, 1}}}, 
};
