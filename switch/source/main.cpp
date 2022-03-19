#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <vector>
#include <algorithm>
#include <deque>
#include <time.h>

int main(int argc, char *argv[])
{
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;

    enum Direction
    {
        DOWN,
        LEFT,
        RIGHT,
        UP
    };

    // Defines the Snake Body Container
    std::deque<SDL_Rect> rq;
    int size = 1;

    // Defines the Apples Container and the variables nessecary to say where it can Spawn
    srand(time(0));
    SDL_Rect apples {rand()%1281,rand()%721,10,10};

    int done = 0, x = 0, w = 1280, h = 720;
    int dir = 0;

    // mandatory at least on switch, else gfx is not properly closed
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    // create an SDL window (OpenGL ES2 always enabled)
    // when SDL_FULLSCREEN flag is not set, viewport is automatically handled by SDL (use SDL_SetWindowSize to "change resolution")
    // available switch SDL2 video modes :
    // 1920 x 1080 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
    // 1280 x 720 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
    window = SDL_CreateWindow("sdl2_gles2", 0, 0, 1280, 720, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // create a renderer (OpenGL ES2)
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // open CONTROLLER_PLAYER_1 and CONTROLLER_PLAYER_2
    // when railed, both joycons are mapped to joystick #0,
    // else joycons are individually mapped to joystick #0, joystick #1, ...
    // https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L45
    for (int i = 0; i < 2; i++) {
        if (SDL_JoystickOpen(i) == NULL) {
            SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
            SDL_Quit();
            return -1;
        }
    }

    PadState pad;
    SDL_Rect head {1280/2,720/2,10,10};
    while (!done) {
        // Scans the Gamepad once each frame
        padUpdate(&pad);

        // Declares the Value to check if the Player pressed a button
        u64 kDown = padGetButtonsDown(&pad);

        // Checks if the Player has pressed one of the Keys below
        if (kDown & HidNpadButton_Plus) { done = true; }    
        if (kDown & HidNpadButton_Up && dir != DOWN || kDown & HidNpadButton_StickLUp && dir != DOWN) { dir = UP; }
        if (kDown & HidNpadButton_Left && dir != RIGHT || kDown & HidNpadButton_StickLLeft && dir != RIGHT) { dir = LEFT; }
        if (kDown & HidNpadButton_Down && dir != UP || kDown & HidNpadButton_StickLDown && dir != UP) { dir = DOWN; }
        if (kDown & HidNpadButton_Right && dir != LEFT || kDown & HidNpadButton_StickLRight && dir != LEFT) { dir = RIGHT; }

        // Checks if the Player is going at a specific Direction
        switch (dir) {
            case DOWN:
                head.y += 10; break;
            case UP:
                head.y -= 10; break;
            case LEFT:
                head.x -= 10; break;
            case RIGHT:
                head.x += 10; break;
            default:
                dir = DOWN; break;
        }

        // Apple collision detection
        if(head.x < apples.x + 11 && head.x > apples.x - 11 && head.y < apples.y + 11 && head.y > apples.y - 11)
        {
            size += 1;
            apples.x = rand()%1281;
            apples.y = rand()%721;
        }

        // Snake collision detection
        std::for_each(rq.begin(), rq.end(), [&](auto& snake_segment) {
            if(head.x == snake_segment.x && head.y == snake_segment.y)
            {
                size = 1;
            }
        });

        // Detects if the Player is outside the barrier and then sets him back
        if (head.x < 0) {
            head.x = 1280;
        } else if (head.x > 1280) {
            head.x = 0;
        }

        if (head.y < 0) {
            head.y = 720;
        } else if (head.y > 720) {
            head.y = 0;
        }

        // Adds a new Head to the Snake
        rq.push_front(head);
        
        while (rq.size() > size)
            rq.pop_back();

        // Clears the Screen
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        // Draws the Body
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        std::for_each(rq.begin(), rq.end(), [&](auto& snake_segment) {
            SDL_RenderFillRect(renderer, &snake_segment);
        });

        // Draws the Apples
        SDL_SetRenderDrawColor(renderer,255,0,0,255);
        SDL_RenderFillRect(renderer,&apples);

        // Display
        SDL_RenderPresent(renderer);
        SDL_Delay(35);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
