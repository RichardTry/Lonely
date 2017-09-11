#include <SDL/SDL.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <ctime>
#include <cmath>

using namespace std;

typedef struct
{
    int r;
    int g;
    int b;
} biome;

const biome arctic = {40000, 40000, 65536};
const biome tundra = {100, 0, 0};
const biome taiga = {0, 2500, 0};
const biome wasteland = {900, 900, 900};
const biome forest = {0, 10000, 0};
const biome desert = {40000, 40000, 0};
const biome savanna = {65536, 40000, 0};
const biome veld = {65536, 30000, 0};
const biome eq = {0, 10000, 1000};
const biome ocean = {0, 0, 65536};

const biome biome_map_earth[64] = {
    arctic, arctic, arctic, arctic, arctic, arctic, arctic, arctic,
    arctic, arctic, arctic, arctic, arctic, tundra, tundra, tundra,
    arctic, arctic, arctic, tundra, tundra, wasteland, taiga, taiga,
    arctic, arctic, tundra, taiga, wasteland, wasteland, forest, forest,
    arctic, tundra, taiga, taiga, wasteland, wasteland, veld, veld,
    arctic, tundra, taiga, forest, forest, veld, desert, desert,
    arctic, tundra, taiga, forest, veld, desert, savanna, savanna,
    arctic, tundra, taiga, forest, veld, desert, savanna, eq
};

biome average(const biome& a, double A, const biome& b, double B)
{
    return {(a.r * A + b.r * B) / (A + B), (a.g * A + b.g * B) / (A + B), (a.b * A + b.b * B) / (A + B)};
}

biome get(const biome* biome_map, int width, int height, double xd, double yd)
{
    xd *= width - 1;
    yd *= height - 1;
    int x = xd;
    int y = yd;
    int x2 = x + 1;
    int y2 = y + 1;
    biome average_left = average(biome_map[y * width + x], y2 - yd, biome_map[min(height - 1, y2) * width + x], yd - y);
    biome average_right = average(biome_map[y * width + min(width - 1, x2)], y2 - yd, biome_map[min(height - 1, y2) * width + min(width - 1, x2)], yd - y);
    return average(average_left, x2 - xd, average_right, xd - x);
}

const int WIDTH = 1024;
const int HEIGHT = 1024;
const double OCEAN_LEVEL = 0.3;

map<pair<int, int>, bool> data_exists;
map<pair<int, int>, int> data;

double pow12(double x)
{
    return pow(x, 1.2);
}

int recursion(int a, int b, int mul)
{
    pair<int, int> key = make_pair((WIDTH - (a * mul) % WIDTH) % WIDTH, (HEIGHT - (b * mul) % HEIGHT) % HEIGHT);
    if(data_exists[key])
        return data[key];
    int ans;
    if(a % 2 == 0 && b % 2 == 0)
        ans = recursion(a / 2, b / 2, mul * 2);
    else if(a % 2 && b % 2)
        ans = (recursion(a - 1, b - 1, mul)+recursion(a - 1, b + 1, mul)+recursion(a + 1, b - 1, mul)+recursion(a + 1, b + 1, mul)) / 4 + rand() % mul - mul / 2;
    else
        ans = (recursion(a - 1, b, mul)+recursion(a + 1, b, mul)+recursion(a, b - 1, mul)+recursion(a, b + 1, mul)) / 4 + rand() % mul - mul / 2;
    ans = abs(ans);
    data_exists[key] = true;
    data[key] = ans;
    return ans;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    atexit(SDL_Quit);

    SDL_Surface* screen = SDL_SetVideoMode(640, 480, 24,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

    data_exists[make_pair(0, 0)] = data_exists[make_pair(0, WIDTH - 1)] = data_exists[make_pair(WIDTH - 1, 0)] = data_exists[make_pair(WIDTH - 1, HEIGHT - 1)] = true;
    vector<vector<int> > hlines(WIDTH, vector<int>(HEIGHT));
    int max_height = 0;
    for(int i = 0; i < WIDTH; i++)
        for(int j = 0; j < HEIGHT; j++)
        {
            hlines[i][j] = min(255, recursion(i, j, 1));
            max_height = max(max_height, hlines[i][j]);
        }
    double rot_angle = 0;
    bool rot_mode = false;
    bool color_mode = false;
    bool done = false;
    while (!done)
    {
        if(rot_angle > 2 * M_PI)
            rot_angle -= 2 * M_PI;
        cout << "PNZ" << endl;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_KEYDOWN:
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = true;
                    else if(event.key.keysym.sym == SDLK_SPACE)
                        rot_mode = !rot_mode;
                    else if(event.key.keysym.sym == SDLK_e)
                        color_mode = !color_mode;
                    break;
                }
            }
        }
        char* ptr = (char*)screen->pixels;
        int step = screen->pitch / screen->w;
        for(int j = 0; j < 480; j++)
            for(int i = 0; i < 640; i++)
            { 
                int value;
                int hyp = hypot(i - 320, j - 240);
                if(hyp == 200)
                {
                    *(ptr++) = 255;
                    *(ptr++) = 255;
                    *(ptr++) = 255;
                }
                else if(hyp > 200)
                {
                    *(ptr++) = 0;
                    *(ptr++) = 0;
                    *(ptr++) = 0;
                }
                else
                {
                    double x = (i - 320) / 200.0;
                    double y = (j - 240) / 200.0;
                    double latitude, longitude;
                    if(rot_mode)
                    {
                        latitude = asin(hypot(x, y));
                        longitude = atan(y / x);
                        if(x < 0)
                            longitude += M_PI;
                    }
                    else
                    {
                        latitude = asin(y);
                        x /= cos(latitude);
                        latitude += M_PI / 2;
                        longitude = asin(x);
                    }
                    longitude += rot_angle;
                    latitude /= M_PI;
                    longitude /= 2 * M_PI;
                    longitude -= floor(longitude);
                    int la = min(HEIGHT - 1., latitude * HEIGHT);
                    int lo = min(WIDTH - 1., longitude * WIDTH);
                    double h = pow12(hlines[lo][la] / (double)max_height);
                    biome b;
                    if(h >= OCEAN_LEVEL)
                        b = get(biome_map_earth, 8, 8, 1 - abs(2 * latitude - 1), 1 - h);
                    else
                        b = ocean;
                    //lo = (WIDTH + lo % WIDTH) % WIDTH;
                    if(color_mode)
                    {
                        *(ptr++) = min(255., h * 256);
                        *(ptr++) = min(255., h * 256);
                        *(ptr++) = min(255., h * 256);
                    }
                    else
                    {
                        *(ptr++) = min(255, (int)sqrt(b.b));
                        *(ptr++) = min(255, (int)sqrt(b.g));
                        *(ptr++) = min(255, (int)sqrt(b.r));
                    }
                }
            }
        SDL_Flip(screen);
        rot_angle += 0.03;
    }
    return 0;
}