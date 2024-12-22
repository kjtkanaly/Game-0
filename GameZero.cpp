#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

using namespace std;

const int WIDTH = 960, HEIGHT = 720;

// -----------------------------------------------------------------------------
// Engine Stuff
// -----------------------------------------------------------------------------

class SDL_General
{
public: 
    int testVal;

    SDL_General():
        testVal(420)
    {}

    void Init()
    {   
        // Init the SDL/Error catch
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) 
        { 
            std::cerr << "Error with Init: " << SDL_GetError() << std::endl; 
        }

        std::cout << "Init successful!!!" 
                  << std::endl;
    }

    SDL_Window* CreateWindow(
        const char* title, 
        const int& posX,
        const int& posY,
        const int& width,
        const int& height,
        const Uint32& flags) 
    {
        // Create the SDL window
        SDL_Window* window = SDL_CreateWindow(
            title,
            posX,
            posY,
            width,
            height,
            flags);

        // Window Error Catch
        if (window == NULL) {
            SDL_Quit();
            std::cerr << "Error Creating Window: " 
                      << SDL_GetError() 
                      << std::endl;
        }

        return window;
    }

    SDL_Renderer* CreateRenderer(
        SDL_Window* window,
        const Uint32& renderFlags)
    {
        // Create the Render Pointer
        SDL_Renderer* rend = SDL_CreateRenderer(window, -1, renderFlags);

        // Render Error Catch
        if (!rend) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            std::cerr << "Error Creating the renderer: " 
                      << SDL_GetError() 
                      << std::endl;
        }
        
        return rend;
    }

    SDL_Texture* LoadTexture(
        const char* filePath,
        SDL_Renderer* rend,
        SDL_Window* window)
    {
        // Load the image into main memory using the SDL library
        SDL_Surface* surf = IMG_Load(filePath);

        // If the texture file path wasn't found
        if (!surf) {
            SDL_DestroyRenderer(rend);
            SDL_DestroyWindow(window);
            SDL_Quit();
            std::cerr << "Error Loading the image: " 
                      << SDL_GetError() 
                      << std::endl;
        }

        // Load the image into graphic memory using the SDL library
        SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);

        // Disposing of the surface now that it's in grahpic memory
        SDL_FreeSurface(surf);  

        // If the texture wasn't loaded into graphical memory correctly
        if (!tex) {
            SDL_DestroyRenderer(rend);
            SDL_DestroyWindow(window);
            SDL_Quit();
            std::cerr << "Error Creating the texture: " 
                      << SDL_GetError() 
                      << std::endl;
        }

        return tex;
    }
};

int main() {
    
    // Make the general SDL object
    SDL_General gen;

    // Init the SDL enviroment
    gen.Init();

    // Create the game Window
    SDL_Window* window = gen.CreateWindow(
        "Hello SDL!",
        0,
        0,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI);

    // Create the renderer for the game window
    SDL_Renderer* rend = gen.CreateRenderer(
        window,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // Load in the following texture
    SDL_Texture* tex = gen.LoadTexture(
        "resources/ship-01.png",
        rend,
        window);

    // Clear the Window by setting it black
    SDL_RenderClear(rend);

    // Draw the image to the window
    SDL_RenderCopy(rend, tex, NULL, NULL);

    // Swaps the render from the back buffer to the front
    SDL_RenderPresent(rend);

    // Main Loop
    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT) {
            break;
        }
        else if (event.type == SDL_KEYDOWN) {
            break;
        }
    }

    // Quit the SDL
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}