#include <iostream>
#include <vector>
#include <map>
#include <cmath>
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
    std::vector<int> colSpace;
    std::vector<SDL_Rect*> projectiles;

    SDL_General():
        window(NULL),
        rend(NULL)
    {}

    void Init(const int& colCount, const int& colSize, const int& windowOffset)
    {   
        // Init the SDL/Error catch
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) 
        { 
            std::cerr << "Error with Init: " << SDL_GetError() << std::endl; 
        }

    // Make the col space array
    for (int i = 0; i < colCount; i++) 
    {
        colSpace.push_back(i * colSize + windowOffset);
        std::cout << "Grid Element: " 
                  << std::to_string(i * colSize + windowOffset)
                  << std::endl;
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
const Vector2Int MAIN_FRAME_ORIGIN = Vector2Int(8, 8), 
                 MAIN_FRAME_SIZE = Vector2Int(720, 704),
                 GRID_DIM = Vector2Int(8, 6), 
                 GRID_ELEM_SIZE = Vector2Int(90, 90);
SDL_General GLOBAL_GEN;

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Game Objects
class GameObject : public SDL_Rect{
    public:
    std::string name;
    SDL_Texture* tex;
    std::vector<GameObject*> children;

    GameObject(const Vector2& inPos = Vector2(0, 0),
               const char* spriteFile = NULL) :
        children()

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

    virtual int Process(const float& deltaTime) { return 0; }

    virtual void Destroy() 
    {
        delete this;
    }
};

class Laser : public GameObject {
    public:
    Laser(
        const Vector2& inPos = Vector2(0, 0),
        const char* spriteFile = NULL)
        : GameObject(inPos, spriteFile)
    {
        pos.x = (float) x;
        pos.y = (float) y;
    }

    int Process(const float& deltaTime) override
    {
        // Update the Up Time
        upTime += deltaTime;
        if (upTime >= maxTime) {
            return 1;
        }

        // Update the position
        pos.y -= speed * deltaTime;

        // Set the position of the dest
        y = (int) pos.y;

        return 0;
    }

    private:
    Vector2 pos;
    float speed = 600;
    float upTime = 0;
    float maxTime = 2.0f;
};

class Alien : public GameObject
{
    public:
    Alien(
        const Vector2& inPos = Vector2(0, 0),
        const char* spriteFile = NULL)
        : GameObject(inPos, spriteFile)
    {}

    int Process(const float& deltaTime) override
    {
        // Check if the alien is being hit by a projectile
        if (IsHit()) TakeDamage();

        // Check if the alien is dead
        if (health <= 0) return 1;

        return 0;
    }

    private:
    bool IsHit()
    {
        // Loop over the projectiles
        for (int i = 0; i < GLOBAL_GEN.projectiles.size(); i++) {
            if (SDL_HasIntersection(this, GLOBAL_GEN.projectiles[i])
                && colliders.find(GLOBAL_GEN.projectiles[i]) == colliders.end())
            {
                // colliders.push_back(GLOBAL_GEN.projectiles[i]);
                colliders[GLOBAL_GEN.projectiles[i]] = 1;
                return true;
            }
        }

        // No hits
        return false;
    }

    void TakeDamage()
    {
        std::cout << "Alien takes damage!" << std::endl;

        // Tick the health 
        health -= 1;
    }

    // Props
    std::map<SDL_Rect*, int> colliders; 
    int health = 2;
};

class Ship : public GameObject
{

    public:
    Ship(
        const Vector2& inPos = Vector2(0, 0),
        const char* spriteFile = NULL)
        : GameObject(inPos, spriteFile)
    {
        float startPos = (float) x;
        float targetPos = startPos;

        timerTimeLeft = timerTotalTime;
    }

    int Process(const float& deltaTime) override
    {
        // Grid movement

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
                            UpdateTargetPos(1);
                            break;
                        case SDL_SCANCODE_LEFT:
                        case SDL_SCANCODE_A:
                            UpdateTargetPos(-1);
                            break;
                        case SDL_SCANCODE_SPACE:
                            ShootLaser();
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }

        NewTranslateToPos(deltaTime);
        
        return 0;
    }

    void UpdateTargetPos(int increment) 
    {
        // Check if the target is still translating
        if (timerTimeLeft < timerTotalTime) return;

        // Check for bounds
        if (targetIndex + increment < 0) return;
        if (targetIndex + increment >= (int) GLOBAL_GEN.colSpace.size()) return;

        // Set the target and start positions
        targetIndex += increment;
        targetPos = (float) GLOBAL_GEN.colSpace[targetIndex];
        startPos = (float) x;

        // Reset the timer clock
        timerTimeLeft = 0;  // Reset the timer

        // std::cout << "Target Index: " << std::to_string(targetIndex) << std::endl;
    }

    float easeInOutSine(float x) 
    {
        float pi = 3.14;
        return -(cos(pi * x) - 1) / 2;
    }

    float easeOutElastic(float x) 
    {
        const float pi = 3.14;
        const double c4 = (2 * M_PI) / 3;

        if (x == 0) {
            return 0;
        } else if (x == 1) {
            return 1;
        } else {
            return pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
        }
    }

    float easeOutBack(float x)
    {
        const float c1 = 1.70158;
        const float c3 = c1 + 1;

        return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
    }

    void NewTranslateToPos(float deltaTime)
    {
        timerTimeLeft += deltaTime;

        if (timerTimeLeft >= timerTotalTime) return;

        float ease = easeOutBack(timerTimeLeft / timerTotalTime);
        float newPos = (targetPos - startPos) * ease + startPos;

        // std::cout << "Ease: " << std::to_string(newPos) << std::endl;

        x = (int) newPos;
    }

    void ShootLaser()
    {
        // Spawn a laser bolt
        Laser* laser = new Laser(Vector2(x, y), "resources/laser-01.png");
        laser->name = "laser";
        laser->w *= 3;
        laser->h *= 3;
        children.push_back(laser);

        // Add the laser bolt to the project list
        GLOBAL_GEN.projectiles.push_back(laser);
    }

    private:
    int targetIndex = 3;
    float timerTimeLeft = 0;
    float timerTotalTime = 0.2;
    float startPos;
    float targetPos;
};

// -----------------------------------------------------------------------------
void RenderGameObjects(GameObject* node) 
{
    // Dig down the root's children
    for (int i = 0; i < node->children.size(); i++) 
    {
        RenderGameObjects(node->children[i]);
    }

    // Render the node
    SDL_RenderCopy(
        GLOBAL_GEN.rend, 
        node->tex, 
        NULL, 
        node);
}

int ProcessObjectTree(GameObject* node, float delta)
{
    // Dig down the root's children
    for (int i = node->children.size() - 1; i >= 0; i--) 
    {
        if (ProcessObjectTree(node->children[i], delta))
        {
            std::cout << "Delete object" << std::endl;
            node->children[i]->Destroy();
            node->children.erase(node->children.begin() + i);
        }
    }

    // Render the node
    return node->Process(delta);
}

// -----------------------------------------------------------------------------
// MAIN
int main() 
{
    // Init the SDL enviroment
    GLOBAL_GEN.Init(GRID_DIM.x, GRID_ELEM_SIZE.x, MAIN_FRAME_ORIGIN.x);

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

    // Create the scene tree list
    GameObject sceneTree;
    sceneTree.name = "Tree";

    // Create the Background object
    GameObject background = GameObject(
        Vector2(0, 0), 
        "resources/main-game-bckg.png");
    background.name = "Background";
    sceneTree.children.push_back(&background);

    // Create the ship object
    Ship ship = Ship(
        Vector2(GLOBAL_GEN.colSpace[3], 595), 
        "resources/ship-01.png");
    ship.name = "Ship";
    ship.w *= 3;
    ship.h *= 3;
    sceneTree.children.push_back(&ship);

    // Create the test alien
    Alien* alien = new Alien(
        Vector2(GLOBAL_GEN.colSpace[3], 8),
        "resources/enemy-01.png");
    alien->name = "Alien";
    alien->w *= 3;
    alien->h *= 3;
    sceneTree.children.push_back(alien);

    // Set to 1 when close window button pressed
    int closeRequested = 0;

    // Main Loop
    while (!closeRequested) 
    {
        float deltaTime = 1 / ((float) FRAME_RATE);

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
        ProcessObjectTree(&sceneTree, deltaTime);

        // Clear the Window by setting it black
        SDL_RenderClear(GLOBAL_GEN.rend);

        // Draw the ship to the render window
        RenderGameObjects(&sceneTree);

        // Swaps the render from the back buffer to the front
        SDL_RenderPresent(GLOBAL_GEN.rend);

        // Wait frame delay
        SDL_Delay(1000 / ((float) FRAME_RATE));
    }

    GLOBAL_GEN.Quit();

    return 0;
}
// -----------------------------------------------------------------------------