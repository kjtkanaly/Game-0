#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

using namespace std;

// -----------------------------------------------------------------------------
// Globals
const int gWidth = 960; 
const int gHeight = 720;
const int gFrameRate = 60;
const float gFrameRateF = (float) gFrameRate;
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
SDL_GLContext* gOpenGLContext = nullptr;
vector<SDL_Event> gEvents;

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
            // std::cout << "Col Element: " 
            //         << std::to_string(i * elemSize.x + origin.x)
            //         << std::endl;
        }
    }

    void MakeRowPosArray()
    {
        // Make the col pos array
        for (int i = 0; i < dim.y; i++) 
        {
            rowPos.push_back(i * elemSize.y + origin.y);
            // std::cout << "Row Element: " 
            //         << std::to_string(i * elemSize.y + origin.y)
            //         << std::endl;
        }
    }
};

// -----------------------------------------------------------------------------
// Framework methods
void InitSDLEnviroment()
{   
    // Init the SDL/Error catch
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) 
    { 
        std::cerr << "Error with Init: " << SDL_GetError() << std::endl; 
    }

    if(TTF_Init() < 0) 
    {
        std::cerr << "Couldn't initialize TTF lib: " << TTF_GetError() << std::endl;
    }

    std::cout << "Init successful!!!" 
                << std::endl;
}

void CreateWindow(
    const Vector2Int& pos,
    const char* title, 
    const Uint32& flags) 
{
    // Create the SDL window
    gWindow = SDL_CreateWindow(
        title,
        pos.x,
        pos.y,
        gWidth,
        gHeight,
        flags);

    // Window Error Catch
    if (gWindow == nullptr) 
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
    if (gWindow == nullptr) 
    {
        SDL_Quit();
        std::cerr << "No Window defined to attach the renderer too"
                    << std::endl;
    }

    // Create the Render Pointer
    gRenderer = SDL_CreateRenderer(gWindow, -1, renderFlags);

    // Render Error Catch
    if (!gRenderer) 
    {
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        std::cerr << "Error Creating the renderer: " 
                    << SDL_GetError() 
                    << std::endl;
    }

    // Clear the Window by setting it black
    SDL_RenderClear(gRenderer);
}

SDL_Texture* CreateTextTexture(
    TTF_Font* font,
    const char* text,
    SDL_Color color)
{
    if (gWindow == NULL) 
    {
        SDL_Quit();
        std::cerr << "No Window Defined"
                    << std::endl;
    }

    if (gRenderer == NULL) 
    {
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        std::cerr << "No Renderer Defined"
                    << std::endl;
    }

    // Create the surface
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(
        font, 
        text, 
        color); 

    // now you can convert it into a texture
    SDL_Texture* textureMessage = SDL_CreateTextureFromSurface(
        gRenderer, 
        surfaceMessage);

    // Disposing of the surface now that it's in grahpic memory
    SDL_FreeSurface(surfaceMessage);  

    // Return message texture
    return textureMessage;
}

SDL_Texture* LoadTexture(
    const char* filePath)
{
    if (gWindow == NULL) 
    {
        SDL_Quit();
        std::cerr << "No Window Defined"
                    << std::endl;
    }

    if (gRenderer == NULL) 
    {
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        std::cerr << "No Renderer Defined"
                    << std::endl;
    }

    // Load the image into main memory using the SDL library
    SDL_Surface* surf = IMG_Load(filePath);

    // If the texture file path wasn't found
    if (!surf) 
    {
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        std::cerr << "Error Loading the image: " 
                    << SDL_GetError() 
                    << std::endl;
    }

    // Load the image into graphic memory using the SDL library
    SDL_Texture* tex = SDL_CreateTextureFromSurface(gRenderer, surf);

    // Disposing of the surface now that it's in grahpic memory
    SDL_FreeSurface(surf);  

    // If the texture wasn't loaded into graphical memory correctly
    if (!tex) 
    {
        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        std::cerr << "Error Creating the texture: " 
                    << SDL_GetError() 
                    << std::endl;
    }

    return tex;
}

void QuitSDLEnviroment() 
{
    // Quit the SDL
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

// -----------------------------------------------------------------------------
// Objects

class Scene
{
    public:
    Frame mainFrame;
    Frame killLogFrame;
    Grid mainGrid;
    Grid killLogGrid;
    std::vector<std::vector<SDL_Rect*>> killLogList;

    Scene()
    {
        mainFrame.origin = Vector2Int(8, 8);
        mainFrame.size = Vector2Int(720, 704);

        mainGrid.dim = Vector2Int(8, 7);
        mainGrid.elemSize = Vector2Int(90, 90);
        mainGrid.MakeColPosArray();
        mainGrid.MakeRowPosArray();

        killLogFrame.origin = Vector2Int(750, 16);
        killLogFrame.size = Vector2Int(208, 489);

        killLogGrid.origin = killLogFrame.origin;
        killLogGrid.dim = Vector2Int(1, 10);
        killLogGrid.elemSize = Vector2Int(195, 48);
        killLogGrid.MakeColPosArray();
        killLogGrid.MakeRowPosArray();
    }

    void AppendKillLog(SDL_Rect* logName, SDL_Rect* logValue)
    {
        // Kill Log entry
        std::vector<SDL_Rect*> killLogEntry;
        killLogEntry.push_back(logValue);
        killLogEntry.push_back(logName);

        // for (int i = 0; i < killLogList.size(); i++)
        for (int i = killLogList.size() - 1; i >= 0; i--)
        {
            killLogList[i][0]->y -= killLogGrid.elemSize.y;
            killLogList[i][1]->y -= killLogGrid.elemSize.y;
        }
        
        killLogList.push_back(killLogEntry);
    }
};

class GameObject : public SDL_Rect
{
    public:
    enum Type
    {
        ENEMY,
        PROJECTILE,
        PLAYER,
        ROOT,
        DEFAULT
    };

    std::string name;
    Type type = Type::DEFAULT;
    SDL_Texture* tex;
    Scene* scene;
    GameObject* root;
    std::vector<GameObject*> children;

    GameObject(const Vector2Int& inPos = Vector2Int(0, 0),
               Scene* scenePtr = NULL,
               GameObject* rootPtr = NULL) :
        children()
    {
        // Set the position of the game object
        x = inPos.x;
        y = inPos.y;

        // Set the pointer to the scene object
        scene = scenePtr;

        // Set the root pointer
        root = rootPtr;
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
    }

    protected: 
    bool destroyQueued = false;
};

class TextObject : public GameObject
{
    public:
    enum HorzAlign
    {
        LEFT,
        CENTER,
        RIGHT
    };

    TTF_Font* font;
    HorzAlign horzAlign;
    SDL_Color color;
    Vector2Int pos;

    TextObject(const Vector2Int& inPos = Vector2Int(0, 0),
          Scene* scenePtr = NULL,
          GameObject* rootPtr = NULL,
          const char* message = NULL,
          const char* fontFile = NULL,
          int size = 24,
          SDL_Color inColor = SDL_Color(),
          HorzAlign inHorzAlign = HorzAlign::LEFT)
        : GameObject(inPos, scenePtr, rootPtr)
    {
        // If no message passed in
        if (message == NULL) cerr << "No message given for text object!" << endl;

        // This opens a font style and sets a size
        font = TTF_OpenFont(fontFile, size);

        if ( !font ) 
        {
            std::cerr << "Error loading font: " << TTF_GetError() << std::endl;
        }

        // Set the color
        color = inColor;

        // Log the pos
        pos = inPos;

        // Get the text as a SDL Texture
        tex = CreateTextTexture(
            font,
            message,
            color);

        // Get the dimensions of the sprite image
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);

        // Check alignment
        horzAlign = inHorzAlign;
        AdjustToHorzAlignment();
    }
    
    protected:
    void AdjustToHorzAlignment()
    {
        switch(horzAlign) 
        {
            case HorzAlign::RIGHT:
                x = pos.x - w;
                break;
            case HorzAlign::CENTER:
                x = pos.x - w / 2;
                break;
            default:
                break;
        }
    }

    private:
};

class ScoreText : public TextObject
{
    public:
    int value = 0;

    ScoreText(const Vector2Int& inPos = Vector2Int(0, 0),
          Scene* scenePtr = NULL,
          GameObject* rootPtr = NULL,
          const char* message = NULL,
          const char* fontFile = NULL,
          int size = 24,
          SDL_Color color = SDL_Color(),
          HorzAlign inHorzAlign = HorzAlign::LEFT)
        : TextObject(inPos, scenePtr, rootPtr, message, fontFile, size, color, inHorzAlign)
    {}

    void UpdateValue(const int& inValue)
    {
        // Increment the value
        value += inValue;

        // Get the text as a SDL Texture
        tex = CreateTextTexture(
            font,
            to_string(value).c_str(),
            color);

        // Get the dimensions of the sprite image
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);

        // Set the alignment
        AdjustToHorzAlignment();
    }

    private:
};

class SpriteObject : public GameObject
{
    public:
    SpriteObject(const Vector2Int& inPos = Vector2Int(0, 0),
          Scene* scenePtr = NULL,
          GameObject* rootPtr = NULL,
          const char* spriteFile = NULL)
        : GameObject(inPos, scenePtr, rootPtr)
    {
        // Set the game object's sprite
        if (spriteFile != NULL) {
            // Load in the following texture
            tex = LoadTexture(spriteFile);

            // Get the dimensions of the sprite image
            SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        }
    }

    private:
};

class Laser : public SpriteObject {
    public:
    Laser(const Vector2Int& inPos = Vector2Int(0, 0),
          Scene* scenePtr = NULL,
          GameObject* rootPtr = NULL,
          const char* spriteFile = NULL)
        : SpriteObject(inPos, scenePtr, rootPtr, spriteFile)
    {
        pos.x = (float) x;
        pos.y = (float) y;

        type = Type::PROJECTILE;
    }

    void Process(const float& deltaTime) override
    {
        // Update the Up Time
        upTime += deltaTime;
        if (upTime >= maxTime) SetDestroyQueuedVal(true);

        // Check if the laer is colliding with an enemy
        if (IsCollidingWithEnemy(root)) SetDestroyQueuedVal(true);

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

    bool IsCollidingWithEnemy(GameObject* node)
    {
        // Loop through the children 
        for (int i = 0; i < node->children.size(); i++) 
        {
            if (IsCollidingWithEnemy(node->children[i])) return true;
        }

        // If not an enemy then return false
        if (node->type != GameObject::Type::ENEMY) return false;

        // No hits
        return (SDL_HasIntersection(this, node));
    }
};

class StatusBar : public GameObject
{
    public:
    StatusBar(
        const Vector2Int& inPos = Vector2Int(0, 0),
        Scene* scenePtr = NULL,
        GameObject* rootPtr = NULL)
        : GameObject(inPos, scenePtr, rootPtr)
    {
        // Init the initial herts
        for (int i = 0; i < initHeartCount; i++) {
            Vector2Int pos = Vector2Int(
                spacing.x * i + origin.x, 
                origin.y);

            cout << to_string(pos.x) << endl;

            // Create the heart object
            SpriteObject* heart = new SpriteObject(
                pos,
                scenePtr,
                rootPtr,
                heartSpritePath);
            heart->name = "Heart";
            heart->w *= 3;
            heart->h *= 3;
            children.push_back(heart);

            // Append the heart icons vector
            heartIcons.push_back(heart);
        }
    }

    void RemoveHeart()
    {
        if (children.size() == 0) return;

        children[children.size() - 1]->SetDestroyQueuedVal(true);
    }

    private:
    int initHeartCount = 3;
    const char* heartSpritePath = "resources/heart.png";
    const Vector2Int origin = Vector2Int(18, 659);
    const Vector2Int spacing = Vector2Int(64, 64);
    vector<SpriteObject*> heartIcons;
};

class Alien : public SpriteObject
{
    public:
    Alien(const Vector2Int& inPos = Vector2Int(0, 0),
          Scene* scenePtr = NULL,
          GameObject* rootPtr = NULL,
          const char* spriteFile = NULL)
        : SpriteObject(inPos, scenePtr, rootPtr, spriteFile)
    {
        // Set the game object type
        type = Type::ENEMY;

        // Get the score value ptr
        DigForScoreValue(root);

        // Get the status bar ptr
        DigForStatusBar(root);
    }

    void Process(const float& deltaTime) override
    {
        // Check if the alien is being hit by a projectile
        if (IsHit(root)) TakeDamage();

        // Check if the alien is dead
        if (health <= 0) 
        {
            // Update the hill log
            // SDL_Color color = {255, 100, 60, 255};
            SDL_Color color = {100, 255, 150, 255};

            TextObject* logName = new TextObject(
                Vector2Int(
                    scene->killLogGrid.colPos[0], 
                    scene->killLogGrid.rowPos[scene->killLogGrid.rowPos.size() - 1]),
                scene,
                root,
                name.c_str(),
                "resources/Born2bSportyV2.ttf",
                32,
                color);
            logName->name = "Kill-Log-Name";
            root->children.push_back(logName);

            std::string valMod = "+";

            TextObject* logValue = new TextObject(
                Vector2Int(
                    scene->killLogGrid.origin.x + scene->killLogGrid.elemSize.x, 
                    scene->killLogGrid.rowPos[scene->killLogGrid.rowPos.size() - 1]),
                scene,
                root,
                (valMod + to_string(pointValue)).c_str(),
                "resources/Born2bSportyV2.ttf",
                32,
                color,
                TextObject::HorzAlign::RIGHT);
            logValue->name = "Kill-Log-Value";
            root->children.push_back(logValue);

            scene->AppendKillLog(logName, logValue);

            if (scene->killLogList.size() > scene->killLogGrid.dim.y)
            {
                ((GameObject*) scene->killLogList[0][0])->SetDestroyQueuedVal(true);
                ((GameObject*) scene->killLogList[0][1])->SetDestroyQueuedVal(true);
                scene->killLogList.erase(scene->killLogList.begin(), scene->killLogList.begin()+1);
            }

            // Update the score value
            scoreValue->UpdateValue(pointValue);

            // Queue destruction
            SetDestroyQueuedVal(true);
        }
    }

    bool IsHit(GameObject* node)
    {
        // Loop over the projectiles
        for (int i = 0; i < node->children.size(); i++) {
            if (IsHit(node->children[i])) return true;
        }

        // If not projectile then return false
        if (node->type != GameObject::Type::PROJECTILE) return false;

        // No hits
        return (SDL_HasIntersection(this, node));
    }

    void TakeDamage()
    {
        // Tick the health 
        health -= 1;
    }

    void DamagePlayer()
    {
        // Update the hill log
        SDL_Color color = {255, 100, 60, 255};

        TextObject* logName = new TextObject(
            Vector2Int(
                scene->killLogGrid.colPos[0], 
                scene->killLogGrid.rowPos[scene->killLogGrid.rowPos.size() - 1]),
            scene,
            root,
            name.c_str(),
            "resources/Born2bSportyV2.ttf",
            32,
            color);
        logName->name = "Kill-Log-Name";
        root->children.push_back(logName);

        std::string valMod = "-";

        TextObject* logValue = new TextObject(
            Vector2Int(
                scene->killLogGrid.origin.x + scene->killLogGrid.elemSize.x, 
                scene->killLogGrid.rowPos[scene->killLogGrid.rowPos.size() - 1]),
            scene,
            root,
            (valMod + to_string(pointValue)).c_str(),
            "resources/Born2bSportyV2.ttf",
            32,
            color,
            TextObject::HorzAlign::RIGHT);
        logValue->name = "Kill-Log-Value";
        root->children.push_back(logValue);

        scene->AppendKillLog(logName, logValue);

        if (scene->killLogList.size() > scene->killLogGrid.dim.y)
        {
            ((GameObject*) scene->killLogList[0][0])->SetDestroyQueuedVal(true);
            ((GameObject*) scene->killLogList[0][1])->SetDestroyQueuedVal(true);
            scene->killLogList.erase(scene->killLogList.begin(), scene->killLogList.begin()+1);
        }

        // Update the score value
        scoreValue->UpdateValue(-1 * pointValue);

        // Removed a heart from the player
        statusBar->RemoveHeart();

        // Log the enemy to be destroyed
        SetDestroyQueuedVal(true);
    }

    private:
    int health = 2;
    int pointValue = 10;
    ScoreText* scoreValue;
    StatusBar* statusBar;

    void DigForScoreValue(GameObject* node)
    {
        for (int i = 0; i < node->children.size(); i++) 
        {
            DigForScoreValue(node->children[i]);
        }

        if (node->name == "Overall-Score-Value") scoreValue = (ScoreText*) node;
    }

    void DigForStatusBar(GameObject* node)
    {
        for (int i = 0; i < node->children.size(); i++) 
        {
            DigForStatusBar(node->children[i]);
        }

        if (node->name == "Status-Bar") statusBar = (StatusBar*) node;
    }
};

class EnemySpawner : public GameObject
{
    public:
    EnemySpawner(const Vector2Int& inPos = Vector2Int(0, 0),
                 Scene* scenePtr = NULL,
                 GameObject* rootPtr = NULL)
        : GameObject(inPos, scenePtr, rootPtr)
    {}

    void Process(const float& deltaTime) override
    {
        elapsedTime += deltaTime;
        if (elapsedTime < spawnDelay) return;

        // Move the current enemies up a level
        IncrementRowPosOfEnemies(root);

        // Spawn a new enemy in a random col
        int col = 0 + ( std::rand() % ( scene->mainGrid.dim.x - 0) );
        Alien* alien = new Alien(
            Vector2Int(scene->mainGrid.colPos[col], scene->mainGrid.rowPos[0]),
            scene,
            root,
            "resources/enemy-01.png");
        alien->name = "Alien";
        alien->w *= 3;
        alien->h *= 3;
        children.push_back(alien);

        elapsedTime = 0;
    }

    private:
    const float spawnDelay = 1.0;
    float elapsedTime = 0;

    void IncrementRowPosOfEnemies(GameObject* node) 
    {
        // Loop through the children
        for (int i = 0; i < node->children.size(); i++) 
        {
            IncrementRowPosOfEnemies(node->children[i]);
        }

        // If not an enemy then return false
        if (node->type != GameObject::Type::ENEMY) return;

        // Update the alien's world pos in accordance with cord
        node->y += scene->mainGrid.elemSize.y;

        // Cast the game object pointer to an alien pointer
        Alien* alien = (Alien*) node;

        // Check if the alien has reached the player's level
        int didDamageLevel = 
            scene->mainGrid.rowPos[scene->mainGrid.rowPos.size() - 1] 
            + scene->mainGrid.elemSize.y;
        if (node->y >= didDamageLevel) alien->DamagePlayer();

        // Check if they should be taking damage in it's new position
        if (alien->IsHit(root)) alien->TakeDamage();
    }
};

class Ship : public SpriteObject
{
    public:
    Ship(const Vector2Int& inPos = Vector2Int(0, 0),
         Scene* scenePtr = NULL,
         GameObject* rootPtr = NULL,
         const char* spriteFile = NULL)
        : SpriteObject(inPos, scenePtr, rootPtr, spriteFile)
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
        for (int i = 0; i < gEvents.size(); i++) 
        {
            event = gEvents[i];
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
            Vector2Int(x + w / 2, y),
            scene,
            root,
            "resources/laser-01.png");
        laser->name = "laser";
        laser->w *= 3;
        laser->h *= 3;
        children.push_back(laser);
    }

    private:
    int targetIndex = 3;
    float timerTimeLeft = 0;
    float timerTotalTime = 0.2;
    float startPos;
    float targetPos;
};

// -----------------------------------------------------------------------------
// Engine Methods
GameObject* InitObjects(Scene* scene)
{
    // Create the scene tree list
    GameObject* root = new GameObject(
        Vector2Int(0, 0),
        scene,
        NULL);
    root->name = "Root";

    // Create the Background object
    SpriteObject* background = new SpriteObject(
        Vector2Int(0, 0),
        scene,
        root,
        "resources/main-game-bckg.png");
    background->name = "Background";
    root->children.push_back(background);

    // Create the ship object
    Ship* ship = new Ship(
        Vector2Int(
            scene->mainGrid.colPos[3], 
            scene->mainGrid.rowPos[scene->mainGrid.rowPos.size() - 1]),
        scene,
        root,
        "resources/ship-01.png");
    ship->name = "Ship";
    ship->w *= 3;
    ship->h *= 3;
    root->children.push_back(ship);

    // Create the damage zone bar
    StatusBar* statusBar = new StatusBar(
        Vector2Int(
            scene->mainFrame.origin.x, 
            651 - 8),
        scene,
        root);
    statusBar->name = "Status-Bar";
    root->children.push_back(statusBar);

    // Create the enemy spawner
    EnemySpawner* spawner = new EnemySpawner(
        Vector2Int(0, 0),
        scene,
        root);
    spawner->name = "Enemy-Spawner";
    root->children.push_back(spawner);

    // Create the color for the font
    SDL_Color color = {255, 255, 255, 255};

    // Create the score text
    const char* fontFile = "resources/Born2bSportyV2.ttf";
    TextObject* scoreText = new TextObject(
        Vector2Int(751, 505),
        scene,
        root,
        "Score:",
        fontFile,
        32,
        color);
    scoreText->name = "Score-Text";
    root->children.push_back(scoreText);

    // Create the score value
    ScoreText* scoreValue = new ScoreText(
        Vector2Int(945, 505),
        scene,
        root,
        "999",
        fontFile,
        32,
        color,
        TextObject::HorzAlign::RIGHT);
    scoreValue->name = "Overall-Score-Value";
    root->children.push_back(scoreValue);
    scoreValue->UpdateValue(0);

    // Create the active item text
    TextObject* activeItemText = new TextObject(
        Vector2Int(783, 555),
        scene,
        root,
        "Active Item",
        fontFile,
        32,
        color);
    activeItemText->name = "Active-Item-Text";
    root->children.push_back(activeItemText);

    // Create the active tiem slot
    SpriteObject* activeItemSlot = new SpriteObject(
        Vector2Int(803, 598),
        scene,
        root,
        "resources/active-item-slot.png");
    activeItemSlot->name = "Active-Item-Slot";
    root->children.push_back(activeItemSlot);

    return root;
}

void RenderGameObjects(GameObject* node) 
{
    // Dig down the root's children
    for (int i = 0; i < node->children.size(); i++) 
    {
        RenderGameObjects(node->children[i]);
    }

    // Render the node
    SDL_RenderCopy(
        gRenderer, 
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
            node->children[i]->Destroy();
            node->children.erase(node->children.begin() + i);
        }
    }    
}

// -----------------------------------------------------------------------------
// MAIN
int main() 
{
    InitSDLEnviroment();

    // Create the game Window
    CreateWindow(
        Vector2Int(0, 0),
        "Hello SDL!",
        0);

    // Create the renderer for the game window
    CreateRenderer(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Create the Scene Object
    Scene scene = Scene();

    GameObject* root = InitObjects(&scene);
    
    // Set to 1 when close window button pressed
    int closeRequested = 0;

    // Main Loop
    while (!closeRequested) 
    {
        float deltaTime = 1 / gFrameRateF;

        // Process Events
        SDL_Event event;
        gEvents.clear();
        while (SDL_PollEvent(&event)) {
            // Append the events list
            gEvents.push_back(event);

            switch (event.type) {
                case SDL_QUIT:
                    closeRequested = 1;
                    break;
            }
        }

        // Process our game objects events
        ProcessObjectTree(root, deltaTime);

        // Destroy the queued objects
        DestoryQueuedObjects(root);

        // Clear the window by setting it black
        SDL_RenderClear(gRenderer);

        // Draw the ship to the render window
        RenderGameObjects(root);

        // Swaps the render from the back buffer to the front
        SDL_RenderPresent(gRenderer);

        // Wait frame delay
        SDL_Delay(1000 / gFrameRateF);
    }

    QuitSDLEnviroment();

    return 0;
}
// -----------------------------------------------------------------------------