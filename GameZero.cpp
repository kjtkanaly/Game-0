#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

using namespace std;

// -----------------------------------------------------------------------------
// GENERAL TOOLS
// Useful data type
struct Vector2
{
    float x;
    float y;

    Vector2(float inX = 0, float inY = 0):
        x(0),
        y(0)
    {
        x = inX;
        y = inY;
    }
};

struct Vector2Int
{
    int x;
    int y;

    Vector2Int(int inX = 0, int inY = 0):
        x(0),
        y(0)
    {
        x = inX;
        y = inY;
    }
};

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ENGINE TOOLS
class SDL_General
{
public:
    SDL_Window* window;
    SDL_Renderer* rend;
    vector<SDL_Event> events;

    SDL_General():
        window(NULL),
        rend(NULL)
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

    void CreateWindow(
        const char* title, 
        const int& posX,
        const int& posY,
        const int& width,
        const int& height,
        const Uint32& flags) 
    {
        // Create the SDL window
        window = SDL_CreateWindow(
            title,
            posX,
            posY,
            width,
            height,
            flags);

        // Window Error Catch
        if (window == NULL) 
        {
            SDL_Quit();
            std::cerr << "Error Creating Window: " 
                      << SDL_GetError() 
                      << std::endl;
        }
    }

    void CreateRenderer(
        const Uint32& renderFlags)
    {
        // Error Catch
        if (window == NULL) 
        {
            SDL_Quit();
            std::cerr << "No Window defined to attach the renderer too"
                      << std::endl;
        }

        // Create the Render Pointer
        rend = SDL_CreateRenderer(window, -1, renderFlags);

        // Render Error Catch
        if (!rend) 
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            std::cerr << "Error Creating the renderer: " 
                      << SDL_GetError() 
                      << std::endl;
        }

        // Clear the Window by setting it black
        SDL_RenderClear(rend);
    }

    SDL_Texture* LoadTexture(
        const char* filePath)
    {
        if (window == NULL) 
        {
            SDL_Quit();
            std::cerr << "No Window Defined"
                      << std::endl;
        }

        if (rend == NULL) 
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            std::cerr << "No Renderer Defined"
                      << std::endl;
        }

        // Load the image into main memory using the SDL library
        SDL_Surface* surf = IMG_Load(filePath);

        // If the texture file path wasn't found
        if (!surf) 
        {
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
        if (!tex) 
        {
            SDL_DestroyRenderer(rend);
            SDL_DestroyWindow(window);
            SDL_Quit();
            std::cerr << "Error Creating the texture: " 
                      << SDL_GetError() 
                      << std::endl;
        }

        return tex;
    }

    void Quit() 
    {
        // Quit the SDL
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// GLOBALS
const int WIDTH = 960, HEIGHT = 720, FRAME_RATE = 60;
const float SPEED = 300;
const Vector2Int MAIN_FRAME_ORIGIN, MAIN_FRAME_SIZE = Vector2Int(720, 720);
SDL_General GLOBAL_GEN;

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Game Objects
class Object : public SDL_Rect{
    public:
    SDL_Texture* tex;

    Object(
        const Vector2& inPos = Vector2(0, 0),
        const char* spriteFile = NULL)
    {
        x = inPos.x;
        y = inPos.y;

        if (spriteFile != NULL) {
            // Load in the following texture
            tex = GLOBAL_GEN.LoadTexture(spriteFile);

            // Get the dimensions of the sprite image
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        }
    }

    virtual void Process(const float& deltaTime) {}
};

class Ship : public Object{

    public:
    Ship(
        const Vector2& inPos = Vector2(0, 0),
        const char* spriteFile = NULL)
        : Object(inPos, spriteFile)
    {
        pos.x = (float) x;
    }

    void Process(const float& deltaTime) override
    {
        // Check for the user input
        SDL_Event event;
        for (int i = 0; i < GLOBAL_GEN.events.size(); i++) 
        {
            event = GLOBAL_GEN.events[i];
            switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_RIGHT:
                        case SDL_SCANCODE_D:
                            right = 1;
                            break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_A:
                            left = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_RIGHT:
                        case SDL_SCANCODE_D:
                            right = 0;
                            break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_A:
                            left = 0;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }

        // Do stuff depending on the user's selections
        // Determine the Velocity
        vel.x = 0;
        if (right && !left) vel.x = SPEED;
        if (left && !right) vel.x = -SPEED;

        // Update the position
        pos.x += vel.x * deltaTime;

        // Collision Detections
        if (pos.x <= MAIN_FRAME_ORIGIN.x) pos.x = 0;
        if (pos.x >= (MAIN_FRAME_ORIGIN.x + MAIN_FRAME_SIZE.x) - w) pos.x = 
            (MAIN_FRAME_ORIGIN.x + MAIN_FRAME_SIZE.x) - w;

        // Set the Positions of the dest
        x = (int) pos.x;
    }

    private:
    // For keep track of the keyboard inputs
    int right;
    int left;
    // The float position
    Vector2 pos;
    // The X Velocity
    Vector2 vel;
};

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// MAIN
int main() 
{
    // Init the SDL enviroment
    GLOBAL_GEN.Init();

    // Create the game Window
    GLOBAL_GEN.CreateWindow(
        "Hello SDL!",
        0,
        0,
        WIDTH,
        HEIGHT,
        0);

    // Create the renderer for the game window
    GLOBAL_GEN.CreateRenderer(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Create the Background object
    Object background = Object(Vector2(0, 0), "resources/main-game-bckg.png");

    // Create the ship object
    Ship ship = Ship(Vector2(312, 595), "resources/ship-01.png");
    ship.w *= 3;
    ship.h *= 3;

    // Set to 1 when close window button pressed
    int closeRequested = 0;

    // Main Loop
    while (!closeRequested) 
    {
        float deltaTime = 1 / ((float) FRAME_RATE);

        // Kind of need to store a list of the events each frame

        // Then step through all of the game objects' process

        // Process Events
        SDL_Event event;
        GLOBAL_GEN.events.clear();
        while (SDL_PollEvent(&event)) {
            // Append the events list
            GLOBAL_GEN.events.push_back(event);

            switch (event.type) {
                case SDL_QUIT:
                    closeRequested = 1;
                    break;
            }
        }

        // Process our game objects events
        ship.Process(deltaTime);

        // Clear the Window by setting it black
        SDL_RenderClear(GLOBAL_GEN.rend);

        // Draw the ship to the render window
        SDL_RenderCopy(GLOBAL_GEN.rend, background.tex, NULL, &background);
        SDL_RenderCopy(GLOBAL_GEN.rend, ship.tex, NULL, &ship);

        // Swaps the render from the back buffer to the front
        SDL_RenderPresent(GLOBAL_GEN.rend);

        // Wait frame delay
        SDL_Delay(1000 / ((float) FRAME_RATE));
    }

    GLOBAL_GEN.Quit();

    return 0;
}
// -----------------------------------------------------------------------------