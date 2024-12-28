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

struct Frame
{
    Vector2Int origin;
    Vector2Int size;
};

struct Grid
{
    Vector2Int origin;
    Vector2Int dim;
    Vector2Int elemSize;
    std::vector<int> colPos;
    std::vector<int> rowPos;

    void MakeColPosArray()
    {
        // Make the col pos array
        for (int i = 0; i < dim.x; i++) 
        {
            colPos.push_back(i * elemSize.x + origin.x);
            std::cout << "Col Element: " 
                    << std::to_string(i * elemSize.x + origin.x)
                    << std::endl;
        }
    }

    void MakeRowPosArray()
    {
        // Make the col pos array
        for (int i = 0; i < dim.y; i++) 
        {
            rowPos.push_back(i * elemSize.y + origin.y);
            std::cout << "Row Element: " 
                    << std::to_string(i * elemSize.y + origin.y)
                    << std::endl;
        }
    }
};

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ENGINE TOOLS
class SDL_General
{
public:
    const int width = 960; 
    const int height = 720;
    const Vector2Int pos = Vector2Int();
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
        const Uint32& flags) 
    {
        // Create the SDL window
        window = SDL_CreateWindow(
            title,
            pos.x,
            pos.y,
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

// -----------------------------------------------------------------------------
// Objects

class Scene
{
    public:
    Frame mainFrame;
    Grid mainGrid;
    std::vector<SDL_Rect*> projectiles;
    std::vector<SDL_Rect*> enemies;

    Scene()
    {
        mainFrame.origin = Vector2Int(8, 8);
        mainFrame.size = Vector2Int(720, 704);

        mainGrid.dim = Vector2Int(8, 6);
        mainGrid.elemSize = Vector2Int(90, 90);
        mainGrid.MakeColPosArray();
        mainGrid.MakeRowPosArray();
    }
};

class GameObject : public SDL_Rect{
    public:
    std::string name;
    SDL_Texture* tex;
    SDL_General* SDL_Gen;
    Scene* scene;
    std::vector<GameObject*> children;

    GameObject(const Vector2Int& inPos = Vector2Int(0, 0),
               const char* spriteFile = NULL,
               SDL_General* SDL_GenPtr = NULL,
               Scene* scenePtr = NULL) :
        children()
    {
        // Set the position of the game object
        x = inPos.x;
        y = inPos.y;

        SDL_Gen = SDL_GenPtr;

        // Set the game object's sprite
        if (spriteFile != NULL) {
            if (SDL_Gen == NULL) std::cerr << "SDL Gen is NULL" << std::endl;

            // Load in the following texture
            tex = SDL_Gen->LoadTexture(spriteFile);

            // Get the dimensions of the sprite image
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        }

        // Set the pointer to the scene object
        scene = scenePtr;
    }

    virtual void Process(const float& deltaTime) {}

    virtual void Destroy() 
    {
        delete this;
    }

    bool GetDestroyQueuedVal() { return destroyQueued; }
    void SetDestroyQueuedVal(bool inVal) 
    { 
        destroyQueued = inVal; 
        if (inVal) std::cout << "Queue deletion..." << std::endl;
    }

    protected: 
    bool destroyQueued = false;
};

class Laser : public GameObject {
    public:
    Laser(const Vector2Int& inPos = Vector2Int(0, 0),
          const char* spriteFile = NULL,
          SDL_General* SDL_GenPtr = NULL,
          Scene* scenePtr = NULL)
        : GameObject(inPos, spriteFile, SDL_GenPtr, scenePtr)
    {
        pos.x = (float) x;
        pos.y = (float) y;
    }

    void Process(const float& deltaTime) override
    {
        // Update the Up Time
        upTime += deltaTime;
        if (upTime >= maxTime) SetDestroyQueuedVal(true);

        // Check if the laer is colliding with an enemy
        if (IsCollidingWithEnemy()) SetDestroyQueuedVal(true);

        // Update the position
        pos.y -= speed * deltaTime;

        // Set the position of the dest
        y = (int) pos.y;
    }

    private:
    Vector2 pos;
    float speed = 600;
    float upTime = 0;
    float maxTime = 2.0f;

    bool IsCollidingWithEnemy()
    {
        for (int i = 0; i < scene->enemies.size(); i++) 
        {
            if (SDL_HasIntersection(this, scene->enemies[i])) return true;
        }

        return false;
    }
};

class Alien : public GameObject
{
    public:
    Alien(const Vector2Int& inPos = Vector2Int(0, 0),
          const char* spriteFile = NULL,
          SDL_General* SDL_GenPtr = NULL,
          Scene* scenePtr = NULL)
        : GameObject(inPos, spriteFile, SDL_GenPtr, scenePtr)
    {}

    void Process(const float& deltaTime) override
    {
        // Check if the alien is being hit by a projectile
        if (IsHit()) TakeDamage();

        // Check if the alien is dead
        if (health <= 0) destroyQueued = true;
    }

    private:
    bool IsHit()
    {
        // Loop over the projectiles
        for (int i = 0; i < scene->projectiles.size(); i++) {
            if (SDL_HasIntersection(this, scene->projectiles[i])) return true;
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
    int health = 2;
};

class EnemySpawner : public GameObject
{
    public:
    EnemySpawner(const Vector2Int& inPos = Vector2Int(0, 0),
                 const char* spriteFile = NULL,
                 SDL_General* SDL_GenPtr = NULL,
                 Scene* scenePtr = NULL)
        : GameObject(inPos, spriteFile, SDL_GenPtr, scenePtr)
    {}

    void Process(const float& deltaTime) override
    {
        elapsedTime += deltaTime;
        if (elapsedTime < spawnDelay) return;

        // Move the current enemies up a level
        for (int i = 0; i < scene->enemies.size(); i++) 
        {
            IncrementEnemyRowPos(scene->enemies[i]);
        }

        // Spawn a new enemy in a random col
        int col = 0 + ( std::rand() % ( scene->mainGrid.dim.x - 0 + 1 ) );
        std::cout << "New Col: " << std::to_string(col) << std::endl;
        Alien* alien = new Alien(
            Vector2Int(scene->mainGrid.colPos[col], scene->mainGrid.rowPos[0]),
            "resources/enemy-01.png",
            SDL_Gen,
            scene);
        alien->name = "Alien";
        alien->w *= 3;
        alien->h *= 3;
        children.push_back(alien);
        scene->enemies.push_back(alien);

        elapsedTime = 0;
    }

    private:
    const float spawnDelay = 2.0;
    float elapsedTime = 0;

    void IncrementEnemyRowPos(SDL_Rect* enemy) 
    {
        // Update the alien's world pos in accordance with cord
        enemy->y += scene->mainGrid.elemSize.y;
    }
};

class Ship : public GameObject
{
    public:
    Ship(const Vector2Int& inPos = Vector2Int(0, 0),
          const char* spriteFile = NULL,
          SDL_General* SDL_GenPtr = NULL,
          Scene* scenePtr = NULL)
        : GameObject(inPos, spriteFile, SDL_GenPtr, scenePtr)
    {
        float startPos = (float) x;
        float targetPos = startPos;

        timerTimeLeft = timerTotalTime;
    }

    void Process(const float& deltaTime) override
    {
        // Grid movement

        // Check for the user input
        SDL_Event event;
        for (int i = 0; i < SDL_Gen->events.size(); i++) 
        {
            event = SDL_Gen->events[i];
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
    }

    void UpdateTargetPos(int increment) 
    {
        // Check if the target is still translating
        if (timerTimeLeft < timerTotalTime) return;

        // Check for bounds
        if (targetIndex + increment < 0) return;
        if (targetIndex + increment >= (int) scene->mainGrid.colPos.size()) return;

        // Set the target and start positions
        targetIndex += increment;
        targetPos = (float) scene->mainGrid.colPos[targetIndex];
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
        Laser* laser = new Laser(
            Vector2Int(x, y), 
            "resources/laser-01.png",
            SDL_Gen,
            scene);
        laser->name = "laser";
        laser->w *= 3;
        laser->h *= 3;
        children.push_back(laser);

        // Add the laser bolt to the project list
        scene->projectiles.push_back(laser);
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
        node->SDL_Gen->rend, 
        node->tex, 
        NULL, 
        node);
}

void ProcessObjectTree(GameObject* node, float delta)
{
    // Dig down the root's children
    for (int i = node->children.size() - 1; i >= 0; i--) 
    {
        ProcessObjectTree(node->children[i], delta);
    }

    // Process the current node
    node->Process(delta);
}

// Step through the scene tree and destroy any marked objects
void DestoryQueuedObjects(GameObject* node)
{
    // Dig down the node's children
    for (int i = node->children.size() - 1; i >= 0; i--) 
    {
        DestoryQueuedObjects(node->children[i]);

        // Destory the object
        if (node->children[i]->GetDestroyQueuedVal()) 
        {
            std::cout << "Deleting object!" << std::endl;
            node->children[i]->Destroy();
            node->children.erase(node->children.begin() + i);
        }
    }    
}

// -----------------------------------------------------------------------------
// MAIN
int main() 
{
    // Set the game loop frame rate
    const int frameRate = 60;
    const float frameRateF = (float) frameRate;

    // Init the SDL enviroment
    SDL_General SDL_Gen;
    SDL_Gen.Init();

    // Create the game Window
    SDL_Gen.CreateWindow(
        "Hello SDL!",
        0);

    // Create the renderer for the game window
    SDL_Gen.CreateRenderer(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Create the Scene Object
    Scene scene = Scene();

    // Create the scene tree list
    GameObject root = GameObject(
        Vector2Int(0, 0),
        NULL,
        &SDL_Gen,
        &scene);
    root.name = "Root";

    // Create the Background object
    GameObject background = GameObject(
        Vector2Int(0, 0), 
        "resources/main-game-bckg.png",
        &SDL_Gen,
        &scene);
    background.name = "Background";
    root.children.push_back(&background);

    // Create the ship object
    Ship ship = Ship(
        Vector2Int(scene.mainGrid.colPos[3], 595), 
        "resources/ship-01.png",
        &SDL_Gen,
        &scene);
    ship.name = "Ship";
    ship.w *= 3;
    ship.h *= 3;
    root.children.push_back(&ship);

    // Create the enemy spawner
    EnemySpawner spawner = EnemySpawner(
        Vector2Int(0, 0),
        NULL,
        &SDL_Gen,
        &scene);
    spawner.name = "Enemy-Spawner";
    root.children.push_back(&spawner);

    // Create the test alien
    Alien* alien = new Alien(
        Vector2Int(scene.mainGrid.colPos[3], 8),
        "resources/enemy-01.png",
        &SDL_Gen,
        &scene);
    alien->name = "Alien";
    alien->w *= 3;
    alien->h *= 3;
    spawner.children.push_back(alien);
    scene.enemies.push_back(alien);

    // Set to 1 when close window button pressed
    int closeRequested = 0;

    // Main Loop
    while (!closeRequested) 
    {
        float deltaTime = 1 / frameRateF;

        // Process Events
        SDL_Event event;
        SDL_Gen.events.clear();
        while (SDL_PollEvent(&event)) {
            // Append the events list
            SDL_Gen.events.push_back(event);

            switch (event.type) {
                case SDL_QUIT:
                    closeRequested = 1;
                    break;
            }
        }

        // Process our game objects events
        ProcessObjectTree(&root, deltaTime);

        // Destroy the queued objects
        DestoryQueuedObjects(&root);

        // Clear the window by setting it black
        SDL_RenderClear(SDL_Gen.rend);

        // Draw the ship to the render window
        RenderGameObjects(&root);

        // Swaps the render from the back buffer to the front
        SDL_RenderPresent(SDL_Gen.rend);

        // Wait frame delay
        SDL_Delay(1000 / frameRateF);
    }

    SDL_Gen.Quit();

    return 0;
}
// -----------------------------------------------------------------------------