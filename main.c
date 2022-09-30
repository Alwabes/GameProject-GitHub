#include <stdio.h>
#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define NUM_SHOOTS 50
#define NUM_MAX_ENEMIES 60
#define FIRST_WAVE 20
#define SECOND_WAVE 30
#define THIRD_WAVE 50
#define BOSS_WAVE 50
#define SURVIVE_WAVE 60

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen
{
    LOGO = 0,
    TITLE,
    GAMEPLAY,
    NARRATIVE,
    ENDING
} GameScreen;
typedef enum
{
    FIRST = 0,
    SECOND,
    THIRD,
    BOSS,
    SURVIVE
} EnemyWave;

typedef struct Player
{
    Rectangle playerSrc;
    Rectangle playerDest;
    Vector2 origin;
    Vector2 speed;
    Texture2D playerSprite;
} Player;

typedef struct Playerscore
{
    char name[11];
    int fscore;
} Playerscore;

typedef struct Life
{
    Rectangle lifeSrc;
    Rectangle lifeDest;
    Vector2 origin;
    Texture2D life;
} Life;

typedef struct Enemy
{
    bool active;
    bool free; // for walking freely
    bool collided;
    int enemyFrame;
    int enemyDir;
    int life;
    int type;
    Rectangle enemySrc;
    Rectangle enemyDest;
    Vector2 speed;
    Vector2 origin;
    Texture2D enemySprite;
} Enemy;

typedef struct Shoot
{
    bool active;
    int bulletDirection;
    int bulletFrame;
    Rectangle shootSrc;
    Rectangle rec;
    Vector2 origin;
    Vector2 speed;
    Texture2D shootSprite;
} Shoot;

typedef struct Song
{
    bool musicPaused;
    Music song;
} Song;

typedef struct SoundEffect
{
    bool soundPaused;
    Sound sound;
} SoundEffect;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
int screenWidth = 1600;
int screenHeight = 900;

static bool gameOver = false;
static bool pause = false;
static bool victory = false;
static int score = 0;

static Player player = {0};
static Player shadow = {0};
static Life playerLife[3] = {0};
static Enemy enemy[NUM_MAX_ENEMIES] = {0};
static Shoot shoot[NUM_SHOOTS] = {0};
static EnemyWave wave = {0};
static Playerscore rankplayer[10] = {0};

static int shootRate = 0;
static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;
static bool load = true;

// Player's moving animation
bool moving;
bool canWalkR = true; // for collision (not fixed yet)
bool canWalkL = true;
bool canWalkD = true;
bool canWalkU = true;
bool alive = true;
int direction, dirImg, playerFrame, frameCount;

// Player's life count
int lifeCount = 3;
int invencibleCount;
int damageAnimCount;
bool colision = true;
bool damageAnim = false;

// Timer variables
int timerCount = 0;

// Music variables
Song backgroundMusic = {0};
Song backgroundMenu = {0};
SoundEffect gameOverSound = {0};
SoundEffect damageTaken = {0};
SoundEffect damageDone = {0};

// Button variables
bool btnAction = false;
bool isPressed = false;
Sound fxButton;
Texture2D button;
Rectangle sourceRec;
Rectangle btnBounds;
Vector2 mousePoint = {0, 0};

bool btnActionCredits = false;
bool isPressedCredits = false;
Texture2D buttonCredits;
Rectangle creditsRec;
Rectangle creditsBounds;

// Current Screen variables
GameScreen currentScreen = LOGO;
Texture2D backgroundLogo, backgroundTitle;
Rectangle bgSrc;
Rectangle bgDest;
Vector2 bgOrigin;

// Main background variables
Texture2D backgroundMain;

// Credits variables
bool opened = false;
Texture2D credits;

// Narrative variables
int countNarrative = 255;
int narrativeScreen = 0;
Texture2D narrative;
Texture2D loading;
Song narrativeMusic;
SoundEffect continueNarrative;

// Rules variables
bool rulesOpen = true;
Texture2D rules;

// text box
char name[11] = "\0"; // NOTE: One extra space required for null terminator char '\0'
int letterCount = 0;

Rectangle textBox = {675, 180, 250, 50};
bool mouseOnText = false;

int framesCounter = 0;
bool endcount = false;
Playerscore player1;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UpdateLogo(void);
void DrawLogo(void);
void UpdateTitle(void);
void DrawTitle(void);
void UpdateNarrative(void);
void DrawNarrative(void);
void UnloadGame(void);
void UpdateDrawFrame(void);
void DrawScreen(void);
void scorerank(void);
void Input_text(void);
void UpdateEnd(void);
void DrawEnd(void);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Config for resizable screen
    // More screen size not implemented, for now just 1600:900
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    Image windowIcon = LoadImage("Assets/NinjaAdventure/icon.png");

    InitWindow(screenWidth, screenHeight, "NINJA DEFENDERS");
    SetWindowIcon(windowIcon);
    InitAudioDevice();
    InitGame();

    int framesCounter = 0;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 144, 1);
#else

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        switch (currentScreen)
        {
        case LOGO:
        {
            UpdateLogo();

            framesCounter++;

            // Wait for 3 seconds (180 frames) before jumping to TITLE screen
            if (framesCounter == 180)
            {
                currentScreen = TITLE;
            }
        }
        break;

        case TITLE:
        {
            UpdateTitle();

            // If button play is pressed, change to GAMEPLAY screen
            if (isPressed)
            {
                currentScreen = NARRATIVE;
                isPressed = false;
            }
        }
        break;

        case NARRATIVE:
        {
            UpdateNarrative();

            // If button play is pressed, change to GAMEPLAY screen
            if (narrativeScreen == 3)
            {
                currentScreen = GAMEPLAY;
                narrativeScreen = 0; // To replay the narrative
            }
        }
        break;

        case GAMEPLAY:
        {
            UpdateGame();

            // Press R to change to ENDING screen
            if (IsKeyPressed(KEY_R))
            {
                currentScreen = ENDING;
            }
        }
        break;

        case ENDING:
        {
            UpdateEnd();

            // Press enter to return to TITLE screen
            if (IsKeyPressed(KEY_Y))
            {
                currentScreen = TITLE;
            }
        }
        break;

        default:
            break;
        }

        // Draw the current screen
        DrawScreen();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------
    UnloadGame();       // Unload loaded data (textures, sounds, models...)
    CloseAudioDevice(); // Close audio device
    CloseWindow();      // Close window and OpenGL context
    //--------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Initialize game variables
//------------------------------------------------------------------------------------
void InitGame(void)
{
    // Secure that the game will start properly
    alive = true;
    damageAnim = false;
    lifeCount = 3;
    timerCount = 0;

    // Initialize game variables
    shootRate = 0;
    pause = false;
    gameOver = false;
    victory = false;
    smooth = false;
    wave = FIRST;
    activeEnemies = FIRST_WAVE;
    enemiesKill = 0;
    score = 0;
    alpha = 0;

    // Initialize background variables
    backgroundMain = LoadTexture("Assets/NinjaAdventure/Backgrounds/backgroundMain.png");
    backgroundLogo = LoadTexture("Assets/NinjaAdventure/Backgrounds/background.png");
    backgroundTitle = LoadTexture("Assets/NinjaAdventure/Backgrounds/backgroud_titlescreen.png");
    credits = LoadTexture("Assets/NinjaAdventure/Backgrounds/credits.png");
    narrative = LoadTexture("Assets/NinjaAdventure/Backgrounds/narrative.png");
    loading = LoadTexture("Assets/NinjaAdventure/Backgrounds/loading.png");
    rules = LoadTexture("Assets/NinjaAdventure/Backgrounds/rules.png");
    bgSrc.x = 0;
    bgSrc.y = 0;
    bgSrc.width = 1280;
    bgSrc.height = 720;
    bgDest.x = 0;
    bgDest.y = 0;
    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();
    bgOrigin.x = 0;
    bgOrigin.y = 0;

    // Initialize audio variables
    backgroundMusic.song = LoadMusicStream("Assets/NinjaAdventure/Musics/4 - Village.ogg");
    SetMusicVolume(backgroundMusic.song, 0.2);

    backgroundMenu.song = LoadMusicStream("Assets/NinjaAdventure/Musics/1 - Adventure Begin.ogg");
    SetMusicVolume(backgroundMenu.song, 0.2);

    narrativeMusic.song = LoadMusicStream("Assets/NinjaAdventure/Musics/13 - Mystical.ogg");
    SetMusicVolume(narrativeMusic.song, 0.4);

    gameOverSound.sound = LoadSound("Assets/NinjaAdventure/Sounds/Game/GameOver.wav");
    SetSoundVolume(gameOverSound.sound, 0.5);

    damageTaken.sound = LoadSound("Assets/NinjaAdventure/Sounds/Game/Hit4.wav");
    SetSoundVolume(damageTaken.sound, 0.2);

    damageDone.sound = LoadSound("Assets/NinjaAdventure/Sounds/Game/Sword2.wav");
    SetSoundVolume(damageDone.sound, 0.2);

    continueNarrative.sound = LoadSound("Assets/NinjaAdventure/Sounds/Menu/Menu1.wav");
    SetSoundVolume(continueNarrative.sound, 0.6);

    fxButton = LoadSound("Assets/NinjaAdventure/Sounds/Menu/Menu9.wav"); // Load button sound
    SetSoundVolume(fxButton, 0.4);

    // Initialize Button variables
    button = LoadTexture("Assets/NinjaAdventure/HUD/play_c.png"); // Load button texture
    sourceRec.x = 0;
    sourceRec.y = 0;
    sourceRec.width = 160;
    sourceRec.height = 52;
    btnBounds.x = GetScreenWidth() / 1.985 - button.width / 2;
    btnBounds.y = GetScreenHeight() / 1.65 + button.height / 2;
    btnBounds.width = 160;
    btnBounds.height = 52;

    buttonCredits = LoadTexture("Assets/NinjaAdventure/HUD/credits_a.png"); // Load button texture
    creditsRec.x = 0;
    creditsRec.y = 0;
    creditsRec.width = 50;
    creditsRec.height = 50;
    creditsBounds.x = GetScreenWidth() - 75;
    creditsBounds.y = GetScreenHeight() - 75;
    creditsBounds.width = 40;
    creditsBounds.height = 40;

    // Initialize player
    player.playerSrc.x = 0;
    player.playerSrc.y = 0;
    player.playerSrc.width = 16;
    player.playerSrc.height = 16;
    player.playerDest.x = GetScreenWidth() / 2;
    player.playerDest.y = GetScreenHeight() / 2;
    player.playerDest.width = 32;
    player.playerDest.height = 32;
    player.origin.x = player.playerDest.width / 2;
    player.origin.y = player.playerDest.height / 2;
    player.speed.x = 4;
    player.speed.y = 4;
    player.playerSprite = LoadTexture("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/walk.png");

    // Initialize player's shadow
    shadow.playerSrc.x = 0;
    shadow.playerSrc.y = 0;
    shadow.playerSrc.width = 12;
    shadow.playerSrc.height = 7;
    shadow.playerDest.x = player.playerDest.x;
    shadow.playerDest.y = player.playerDest.y + 14;
    shadow.playerDest.width = 24;
    shadow.playerDest.height = 14;
    shadow.origin.x = shadow.playerDest.width / 2;
    shadow.origin.y = shadow.playerDest.height / 2;
    shadow.speed.x = 0;
    shadow.speed.y = 0;
    shadow.playerSprite = LoadTexture("Assets/NinjaAdventure/Actor/Characters/Shadow.png");

    // Initialize player's life
    playerLife[0].life = LoadTexture("Assets/NinjaAdventure/HUD/Heart.png");
    playerLife[0].lifeSrc.x = 0;
    playerLife[0].lifeSrc.y = 0;
    playerLife[0].lifeSrc.width = 16.2;
    playerLife[0].lifeSrc.height = 16.2;
    playerLife[0].lifeDest.x = 40;
    playerLife[0].lifeDest.y = GetScreenHeight() - 60;
    playerLife[0].lifeDest.width = 32;
    playerLife[0].lifeDest.height = 32;
    playerLife[0].origin.x = 0;
    playerLife[0].origin.y = 0;

    playerLife[1].life = LoadTexture("Assets/NinjaAdventure/HUD/Heart.png");
    playerLife[1].lifeSrc.x = 0;
    playerLife[1].lifeSrc.y = 0;
    playerLife[1].lifeSrc.width = 16.2;
    playerLife[1].lifeSrc.height = 16.2;
    playerLife[1].lifeDest.x = 85;
    playerLife[1].lifeDest.y = GetScreenHeight() - 60;
    playerLife[1].lifeDest.width = 32;
    playerLife[1].lifeDest.height = 32;
    playerLife[1].origin.x = 0;
    playerLife[1].origin.y = 0;

    playerLife[2].life = LoadTexture("Assets/NinjaAdventure/HUD/Heart.png");
    playerLife[2].lifeSrc.x = 0;
    playerLife[2].lifeSrc.y = 0;
    playerLife[2].lifeSrc.width = 16.2;
    playerLife[2].lifeSrc.height = 16.2;
    playerLife[2].lifeDest.x = 130;
    playerLife[2].lifeDest.y = GetScreenHeight() - 60;
    playerLife[2].lifeDest.width = 32;
    playerLife[2].lifeDest.height = 32;
    playerLife[1].origin.x = 0;
    playerLife[1].origin.y = 0;

    // Initialize right side enemies
    for (int i = 0; i < NUM_MAX_ENEMIES; i += 4)
    {
        enemy[i].enemySrc.width = 16;
        enemy[i].enemySrc.height = 16;
        enemy[i].enemySrc.x = 0;
        enemy[i].enemySrc.y = 0;
        enemy[i].enemyDest.x = GetRandomValue(GetScreenWidth(), GetScreenWidth() + 1000);
        enemy[i].enemyDest.y = GetRandomValue(0, GetScreenHeight() - enemy[i].enemyDest.height);
        enemy[i].enemyDest.width = 16;
        enemy[i].enemyDest.height = 16;
        enemy[i].speed.x = 0.5;
        enemy[i].speed.y = 0.5;
        enemy[i].origin.x = enemy[i].enemyDest.width / 2;
        enemy[i].origin.y = enemy[i].enemyDest.height / 2;
        enemy[i].active = true;
        enemy[i].free = false;
        enemy[i].collided = false;
    }

    // Initialize left side enemies
    for (int i = 1; i < NUM_MAX_ENEMIES; i += 4)
    {
        enemy[i].enemySrc.width = 16;
        enemy[i].enemySrc.height = 16;
        enemy[i].enemySrc.x = 0;
        enemy[i].enemySrc.y = 0;
        enemy[i].enemyDest.x = GetRandomValue(-1000, 0);
        enemy[i].enemyDest.y = GetRandomValue(0, GetScreenHeight() - enemy[i].enemyDest.height);
        enemy[i].enemyDest.width = 16;
        enemy[i].enemyDest.height = 16;
        enemy[i].speed.x = 0.5;
        enemy[i].speed.y = 0.5;
        enemy[i].origin.x = enemy[i].enemyDest.width / 2;
        enemy[i].origin.y = enemy[i].enemyDest.height / 2;
        enemy[i].active = true;
        enemy[i].free = false;
        enemy[i].collided = false;
    }

    // Initialize bottom side enemies
    for (int i = 2; i < NUM_MAX_ENEMIES; i += 4)
    {
        enemy[i].enemySrc.width = 16;
        enemy[i].enemySrc.height = 16;
        enemy[i].enemySrc.x = 0;
        enemy[i].enemySrc.y = 0;
        enemy[i].enemyDest.x = GetRandomValue(0, GetScreenWidth() - enemy[i].enemyDest.width);
        enemy[i].enemyDest.y = GetRandomValue(GetScreenHeight(), GetScreenHeight() + 1000);
        enemy[i].enemyDest.width = 16;
        enemy[i].enemyDest.height = 16;
        enemy[i].speed.x = 0.5;
        enemy[i].speed.y = 0.5;
        enemy[i].origin.x = enemy[i].enemyDest.width / 2;
        enemy[i].origin.y = enemy[i].enemyDest.height / 2;
        enemy[i].active = true;
        enemy[i].free = false;
        enemy[i].collided = false;
    }

    // Initialize top side enemies
    for (int i = 3; i < NUM_MAX_ENEMIES; i += 4)
    {
        enemy[i].enemySrc.width = 16;
        enemy[i].enemySrc.height = 16;
        enemy[i].enemySrc.x = 0;
        enemy[i].enemySrc.y = 0;
        enemy[i].enemyDest.x = GetRandomValue(0, GetScreenWidth() - enemy[i].enemyDest.width);
        enemy[i].enemyDest.y = GetRandomValue(-1000, 0);
        enemy[i].enemyDest.width = 16;
        enemy[i].enemyDest.height = 16;
        enemy[i].speed.x = 0.5;
        enemy[i].speed.y = 0.5;
        enemy[i].origin.x = enemy[i].enemyDest.width / 2;
        enemy[i].origin.y = enemy[i].enemyDest.height / 2;
        enemy[i].active = true;
        enemy[i].free = false;
        enemy[i].collided = false;
    }

    // Initialize shoots
    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        shoot[i].shootSrc.x = 0;
        shoot[i].shootSrc.y = 0;
        shoot[i].shootSrc.width = 16;
        shoot[i].shootSrc.height = 16;
        shoot[i].rec.x = player.playerDest.x;
        shoot[i].rec.y = player.playerDest.y;
        shoot[i].rec.width = 16;
        shoot[i].rec.height = 16;
        shoot[i].origin.x = shoot[i].rec.width / 2;
        shoot[i].origin.y = shoot[i].rec.height / 2;
        shoot[i].speed.x = 7;
        shoot[i].speed.y = 7;
        shoot[i].bulletFrame = 0;
        shoot[i].active = false;
        shoot[i].shootSprite = LoadTexture("Assets/NinjaAdventure/HUD/Shuriken_anim.png");
    }
}

//------------------------------------------------------------------------------------
// Update game (one frame)
//------------------------------------------------------------------------------------
void UpdateGame(void)
{
    // Adjusting visual elements on resizabled window
    if (IsWindowResized())
    {
        // Adjust background size
        bgDest.width = GetScreenWidth();
        bgDest.height = GetScreenHeight();

        // player's life
        playerLife[0].lifeDest.y = GetScreenHeight() - 60;
        playerLife[1].lifeDest.y = GetScreenHeight() - 60;
        playerLife[2].lifeDest.y = GetScreenHeight() - 60;
    }

    // Time counter (60|1sec)
    frameCount++;

    // Rules screen
    mousePoint = GetMousePosition();

    if (CheckCollisionPointRec(mousePoint, (Rectangle){717, 680, 167, 43}))
    {
        if (rulesOpen && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            rulesOpen = false;
            PlaySound(fxButton);
        }
    }

    if (!gameOver && !rulesOpen)
    {
        // Background music
        UpdateMusicStream(backgroundMusic.song);
        PlayMusicStream(backgroundMusic.song);

        if (IsKeyPressed('P'))
            pause = !pause;

        if (!pause)
        {
            switch (wave)
            {
            case FIRST:
            {
                if (load)
                {
                    // Initialize enemy sprite
                    for (int i = 0; i < activeEnemies; i += 2)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam2/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    for (int i = 1; i < activeEnemies; i += 2)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    load = false;
                }

                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f)
                        smooth = true;
                }

                if (smooth)
                    alpha -= 0.02f;

                if (enemiesKill == activeEnemies)
                {
                    enemiesKill = 0;

                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active)
                            enemy[i].active = true;
                    }

                    activeEnemies = SECOND_WAVE;
                    wave = SECOND;
                    smooth = false;
                    load = true;
                    alpha = 0.0f;
                }
            }
            break;

            case SECOND:
            {
                if (load)
                {
                    // Initialize enemy sprite
                    for (int i = 0; i < activeEnemies; i += 3)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Cyclope/SpriteSheet.png");
                        enemy[i].life = 2;
                        enemy[i].type = 2;
                    }

                    for (int i = 1; i < activeEnemies; i += 3)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    for (int i = 2; i < activeEnemies; i += 3)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam2/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    load = false;
                }

                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f)
                        smooth = true;
                }

                if (smooth)
                    alpha -= 0.02f;

                if (enemiesKill == activeEnemies)
                {
                    enemiesKill = 0;

                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active)
                            enemy[i].active = true;
                    }

                    activeEnemies = THIRD_WAVE;
                    wave = THIRD;
                    smooth = false;
                    load = true;
                    alpha = 0.0f;
                }
            }
            break;

            case THIRD:
            {
                if (load)
                {
                    // Initialize enemy sprite
                    for (int i = 0; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Reptile.png");
                        enemy[i].life = 3;
                        enemy[i].type = 3;
                        enemy[i].enemyDest.width = 32;
                        enemy[i].enemyDest.height = 32;
                    }

                    for (int i = 1; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Cyclope/SpriteSheet.png");
                        enemy[i].life = 2;
                        enemy[i].type = 2;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 2; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 3; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam2/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 4; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Snake.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    load = false;
                }

                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f)
                        smooth = true;
                }

                if (smooth)
                    alpha -= 0.02f;

                if (enemiesKill == activeEnemies)
                {
                    enemiesKill = 0;

                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active)
                            enemy[i].active = true;
                    }

                    activeEnemies = BOSS_WAVE;
                    wave = BOSS;
                    smooth = false;
                    load = true;
                    alpha = 0.0f;
                }
            }
            break;

            case BOSS:
            {
                if (load)
                {
                    // Initialize enemy sprite
                    // Initialize enemy sprite
                    for (int i = 0; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Reptile.png");
                        enemy[i].life = 3;
                        enemy[i].type = 3;
                        enemy[i].enemyDest.width = 32;
                        enemy[i].enemyDest.height = 32;
                    }

                    for (int i = 1; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Cyclope/SpriteSheet.png");
                        enemy[i].life = 2;
                        enemy[i].type = 2;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 2; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 3; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam2/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 4; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Snake.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    load = false;
                }

                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f)
                        smooth = true;
                }

                if (smooth)
                    alpha -= 0.02f;

                if (enemiesKill == activeEnemies)
                {
                    enemiesKill = 0;

                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active)
                            enemy[i].active = true;
                    }

                    victory = true;
                    activeEnemies = SURVIVE_WAVE;
                    wave = SURVIVE;
                    smooth = false;
                    load = true;
                    alpha = 0.0f;
                }
            }
            break;

            case SURVIVE:
            {
                if (load)
                {
                    // Initialize enemy sprite
                    for (int i = 0; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Reptile.png");
                        enemy[i].life = 3;
                        enemy[i].type = 3;
                        enemy[i].enemyDest.width = 32;
                        enemy[i].enemyDest.height = 32;
                    }

                    for (int i = 1; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Cyclope/SpriteSheet.png");
                        enemy[i].life = 2;
                        enemy[i].type = 2;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 2; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 3; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Flam2/SpriteSheet.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                        enemy[i].enemyDest.width = 16;
                        enemy[i].enemyDest.height = 16;
                    }

                    for (int i = 4; i < activeEnemies; i += 5)
                    {
                        enemy[i].enemySprite = LoadTexture("Assets/NinjaAdventure/Actor/Monsters/Snake.png");
                        enemy[i].life = 1;
                        enemy[i].type = 1;
                    }

                    load = false;
                }

                if (!smooth)
                {
                    alpha += 0.02f;

                    if (alpha >= 1.0f)
                        smooth = true;
                }

                if (smooth)
                    alpha -= 0.02f;

                if (enemiesKill == activeEnemies)
                {
                    enemiesKill = 0;
                    for (int i = 0; i < activeEnemies; i++)
                    {
                        if (!enemy[i].active)
                            enemy[i].active = true;
                    }

                    activeEnemies = SURVIVE_WAVE;
                    wave = SURVIVE;
                    smooth = false;
                    load = true;
                    alpha = 0.0f;
                }
            }
            break;

            default:
                break;
            }

            // Player movement
            moving = false;

            if (alive)
            {
                if (IsKeyDown(KEY_UP) && IsKeyDown(KEY_LEFT))
                {
                    if (canWalkU && canWalkL)
                    {
                        player.playerDest.x -= player.speed.x;
                        player.playerDest.y -= player.speed.y;
                    }

                    direction = 7;
                    dirImg = 1;
                    moving = true;
                }

                else if (IsKeyDown(KEY_UP) && IsKeyDown(KEY_RIGHT))
                {
                    if (canWalkU && canWalkR)
                    {
                        player.playerDest.x += player.speed.x;
                        player.playerDest.y -= player.speed.y;
                    }
                    direction = 6;
                    dirImg = 1;
                    moving = true;
                }

                else if (IsKeyDown(KEY_DOWN) && IsKeyDown(KEY_LEFT))
                {
                    if (canWalkD && canWalkL)
                    {
                        player.playerDest.x -= player.speed.x;
                        player.playerDest.y += player.speed.y;
                    }
                    direction = 5;
                    dirImg = 0;
                    moving = true;
                }

                else if (IsKeyDown(KEY_DOWN) && IsKeyDown(KEY_RIGHT))
                {
                    if (canWalkD && canWalkR)
                    {
                        player.playerDest.x += player.speed.x;
                        player.playerDest.y += player.speed.y;
                    }
                    direction = 4;
                    dirImg = 0;
                    moving = true;
                }

                else if (IsKeyDown(KEY_RIGHT))
                {
                    if (canWalkR)
                        player.playerDest.x += player.speed.x;

                    direction = 3;
                    dirImg = 3;
                    moving = true;
                }

                else if (IsKeyDown(KEY_LEFT))
                {
                    if (canWalkL)
                        player.playerDest.x -= player.speed.x;

                    direction = 2;
                    dirImg = 2;
                    moving = true;
                }

                else if (IsKeyDown(KEY_UP))
                {
                    if (canWalkU)
                        player.playerDest.y -= player.speed.y;

                    direction = 1;
                    dirImg = 1;
                    moving = true;
                }

                else if (IsKeyDown(KEY_DOWN))
                {
                    if (canWalkD)
                        player.playerDest.y += player.speed.y;

                    direction = 0;
                    dirImg = 0;
                    moving = true;
                }
            }

            /*
            Vector2 p;
            Vector2 q;

            switch (direction)
            {
                case 0:
                    p.x = player.playerDest.x;
                    p.y = player.playerDest.y + player.playerDest.height/2 + 10;
                    break;

                case 1:
                    p.x = player.playerDest.x;
                    p.y = player.playerDest.y - player.playerDest.height/2 - 10;
                    break;

                case 2:
                    p.x = player.playerDest.x - player.playerDest.width/2 - 10;
                    p.y = player.playerDest.y;
                    break;

                case 3:
                    p.x = player.playerDest.x + player.playerDest.width/2 + 10;
                    p.y = player.playerDest.y;
                    break;

                case 4:
                    p.x = player.playerDest.x + player.playerDest.width/2 + 10;
                    p.y = player.playerDest.y + player.playerDest.height/2 + 10;
                    q.x = player.playerDest.x;
                    q.y = player.playerDest.y + player.playerDest.height/2 + 10;
                    break;

                case 5:
                    p.x = player.playerDest.x - player.playerDest.width/2 - 10;
                    p.y = player.playerDest.y + player.playerDest.height/2 + 10;
                    q.x = player.playerDest.x;
                    q.y = player.playerDest.y + player.playerDest.height/2 + 10;
                    break;

                case 6:
                    p.x = player.playerDest.x + player.playerDest.width/2 + 10;
                    p.y = player.playerDest.y - player.playerDest.height/2 - 10;
                    q.x = player.playerDest.x;
                    q.y = player.playerDest.y - player.playerDest.height/2 - 10;
                    break;

                case 7:
                    p.x = player.playerDest.x - player.playerDest.width/2 - 10;
                    p.y = player.playerDest.y - player.playerDest.height/2 - 10;
                    q.x = player.playerDest.x;
                    q.y = player.playerDest.y - player.playerDest.height/2 - 10;
                    break;

                default: break;
            }

            // Map collision behaviour
            if (CheckCollisionPointRec(p, statue) || CheckCollisionPointRec(q, statue))
            {
                switch (direction)
                {
                    case 0:
                    case 4:
                    case 5: canWalkD = false;
                            canWalkU = true;
                            canWalkL = true;
                            canWalkR = true;
                            break;

                    case 1:
                    case 6:
                    case 7: canWalkU = false;
                            canWalkD = true;
                            canWalkL = true;
                            canWalkR = true;
                            break;

                    case 2: canWalkL = false;
                            canWalkD = true;
                            canWalkU = true;
                            canWalkR = true;
                            break;

                    case 3: canWalkR = false;
                            canWalkD = true;
                            canWalkU = true;
                            canWalkL = true;
                            break;

                    default: break;
                }
            }
            else
            {
                canWalkD = true;
                canWalkU = true;
                canWalkL = true;
                canWalkR = true;
            }
            */

            // In case the player is moving diagonaly and stop, shoot won't bug
            if (!moving)
            {
                if (direction == 4 || direction == 5)
                    direction = 0;

                if (direction == 6 || direction == 7)
                    direction = 1;
            }

            // Player's movement animation
            player.playerSrc.y = 0;

            if (moving)
            {
                if (frameCount % 10 == 1)
                    playerFrame++;

                player.playerSrc.y = player.playerSrc.width * playerFrame;
            }

            // Reset the animation
            if (playerFrame > 3)
                playerFrame = 0;

            player.playerSrc.x = player.playerSrc.width * dirImg;

            // Player collision with enemy
            for (int i = 0; i < activeEnemies; i++)
            {
                if (alive)
                {
                    if (CheckCollisionRecs(player.playerDest, enemy[i].enemyDest) && colision)
                    {
                        playerLife[lifeCount - 1].lifeSrc.x = (playerLife[lifeCount - 1].lifeSrc.width * 4) - 0.8;
                        PlaySound(damageTaken.sound);
                        lifeCount--;
                        colision = false;
                        damageAnim = true;
                        damageAnimCount = 0;
                        invencibleCount = 0;
                    }

                    // Damage "animation" indicator
                    if (damageAnim)
                    {
                        if (damageAnimCount == 0 || damageAnimCount == 200)
                        {
                            player.playerSprite = LoadTexture("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/Damage.png");
                        }
                        else if (damageAnimCount == 100 || damageAnimCount == 300)
                        {
                            player.playerSprite = LoadTexture("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/walk.png");
                        }

                        damageAnimCount++;

                        if (damageAnimCount > 300)
                        {
                            damageAnim = false;
                            damageAnimCount = 0;
                        }
                    }

                    // Player can't take damage while "invencible" is activated
                    invencibleCount++;
                    if (invencibleCount > 300)
                    {
                        colision = true;
                    }
                }

                // When player is dead
                if (lifeCount == 0)
                {
                    StopMusicStream(backgroundMusic.song);

                    player.playerSprite = LoadTexture("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/Dead.png");
                    player.playerSrc.x = 0;
                    player.playerSrc.y = 0;
                    alive = false;

                    PlaySound(gameOverSound.sound);

                    timerCount++;

                    if (timerCount > 500)
                    {
                        gameOver = true;
                    }
                }
            }

            // Initial right enemy behaviour
            for (int i = 0; i < activeEnemies; i += 4)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].enemyDest.x > GetScreenWidth() - 25)
                        enemy[i].enemyDest.x -= enemy[i].speed.x;
                    if (enemy[i].enemyDest.x <= GetScreenWidth() - 25)
                        enemy[i].free = true;
                }
            }

            // Initial left enemy behaviour
            for (int i = 1; i < activeEnemies; i += 4)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].enemyDest.x < 25)
                        enemy[i].enemyDest.x += enemy[i].speed.x;

                    if (enemy[i].enemyDest.x >= 25)
                        enemy[i].free = true;
                }
            }

            // Initial bottom enemy behaviour
            for (int i = 2; i < activeEnemies; i += 4)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].enemyDest.y > GetScreenHeight() - 25)
                        enemy[i].enemyDest.y -= enemy[i].speed.y;
                    if (enemy[i].enemyDest.y <= GetScreenHeight() - 25)
                        enemy[i].free = true;
                }
            }

            // Initial top enemy behavior
            for (int i = 3; i < activeEnemies; i += 4)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].enemyDest.y < 25)
                        enemy[i].enemyDest.y += enemy[i].speed.y;
                    if (enemy[i].enemyDest.y >= 25)
                        enemy[i].free = true;
                }
            }

            // General enemy behaviour (follow player)
            for (int i = 0; i < activeEnemies; i++)
            {
                int indice;
                enemy[i].collided = false;

                for (int j = 0; j < activeEnemies; j++)
                {
                    if (i != j)
                    {
                        if (CheckCollisionRecs(enemy[i].enemyDest, enemy[j].enemyDest))
                        {
                            enemy[i].collided = true;
                            indice = j;
                        }
                    }
                }

                bool yMenor = true;
                bool yMaior = true;
                if (enemy[i].active && enemy[i].free && !enemy[i].collided)
                {
                    if (player.playerDest.x < enemy[i].enemyDest.x)
                    {
                        enemy[i].enemyDest.x -= enemy[i].speed.x;
                    }

                    if (player.playerDest.x > enemy[i].enemyDest.x)
                    {
                        enemy[i].enemyDest.x += enemy[i].speed.x;
                    }

                    if (player.playerDest.y < enemy[i].enemyDest.y)
                    {
                        enemy[i].enemyDest.y -= enemy[i].speed.y;
                        enemy[i].enemyDir = 1; // Top
                    }
                    else
                    {
                        yMenor = false;
                    }

                    if (player.playerDest.y > enemy[i].enemyDest.y)
                    {
                        enemy[i].enemyDest.y += enemy[i].speed.y;
                        enemy[i].enemyDir = 0; // Bottom
                    }
                    else
                    {
                        yMaior = false;
                    }

                    // For horizonatal animation
                    if (!yMenor && !yMaior)
                    {
                        if (player.playerDest.x < enemy[i].enemyDest.x)
                            enemy[i].enemyDir = 2; // Left

                        if (player.playerDest.x > enemy[i].enemyDest.x)
                            enemy[i].enemyDir = 3; // Right
                    }
                }
                else if (enemy[i].active && enemy[i].free && enemy[i].collided)
                {
                    if (enemy[i].enemyDest.x < enemy[indice].enemyDest.x)
                    {
                        enemy[i].enemyDest.x -= enemy[i].speed.x;
                    }

                    if (enemy[i].enemyDest.x > enemy[indice].enemyDest.x)
                    {
                        enemy[i].enemyDest.x += enemy[i].speed.x;
                    }

                    if (enemy[i].enemyDest.y < enemy[indice].enemyDest.y)
                    {
                        enemy[i].enemyDest.y -= enemy[i].speed.y;
                    }

                    if (enemy[i].enemyDest.y > enemy[indice].enemyDest.y)
                    {
                        enemy[i].enemyDest.y += enemy[i].speed.y;
                    }
                }
            }

            // Enemy movement animation
            for (int i = 0; i < activeEnemies; i++)
            {
                enemy[i].enemySrc.y = 0;

                if (enemy[i].active)
                {
                    if (frameCount % 10 == 1)
                        enemy[i].enemyFrame++;

                    enemy[i].enemySrc.y = enemy[i].enemySrc.height * enemy[i].enemyFrame;
                }

                // Reset the animation
                if (enemy[i].enemyFrame > 3)
                    enemy[i].enemyFrame = 0;

                enemy[i].enemySrc.x = enemy[i].enemySrc.width * enemy[i].enemyDir;
            }

            // Wall behaviour
            if (player.playerDest.x - player.playerDest.width / 2 <= 0)
                player.playerDest.x = player.playerDest.width / 2;
            if (player.playerDest.x + player.playerDest.width / 2 >= GetScreenWidth())
                player.playerDest.x = GetScreenWidth() - player.playerDest.width / 2;
            if (player.playerDest.y - player.playerDest.height / 2 <= 0)
                player.playerDest.y = player.playerDest.height / 2;
            if (player.playerDest.y + player.playerDest.height / 2 >= GetScreenHeight())
                player.playerDest.y = GetScreenHeight() - player.playerDest.height / 2;

            // Shadow behaviour
            shadow.playerDest.x = player.playerDest.x;
            shadow.playerDest.y = player.playerDest.y + 14;

            // Shoot initialization
            if (IsKeyDown(KEY_SPACE))
            {
                shootRate += 2;

                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (!shoot[i].active && shootRate % 40 == 0)
                    {
                        shoot[i].rec.x = player.playerDest.x;
                        shoot[i].rec.y = player.playerDest.y + 10;
                        shoot[i].active = true;

                        // Bullet Movement
                        // Using variable direction to see where's the player shooting.
                        // Using bulletDirection to define where's the bullet going.

                        switch (direction)
                        {
                        case 7:
                            shoot[i].bulletDirection = 7;
                            break;

                        case 6:
                            shoot[i].bulletDirection = 6;
                            break;

                        case 5:
                            shoot[i].bulletDirection = 5;
                            break;

                        case 4:
                            shoot[i].bulletDirection = 4;
                            break;

                        case 3:
                            shoot[i].bulletDirection = 3;
                            break;

                        case 2:
                            shoot[i].bulletDirection = 2;
                            break;

                        case 1:
                            shoot[i].bulletDirection = 1;
                            break;

                        case 0:
                            shoot[i].bulletDirection = 0;
                            break;

                        default:
                            break;
                        }

                        break;
                    }
                }
            }

            // Shoot logic
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active)
                {
                    // Shuriken throw animation
                    if (frameCount % 4 == 0)
                    {
                        shoot[i].shootSrc.x = shoot[i].bulletFrame * 16;
                        shoot[i].bulletFrame++;

                        if (shoot[i].bulletFrame > 1)
                            shoot[i].bulletFrame = 0;
                    }

                    // bulletDirection:
                    switch (shoot[i].bulletDirection)
                    {
                    // [Top-Left]
                    case 7:
                        shoot[i].rec.x -= shoot[i].speed.x;
                        shoot[i].rec.y -= shoot[i].speed.y;
                        break;

                    // [Top-Right]
                    case 6:
                        shoot[i].rec.x += shoot[i].speed.x;
                        shoot[i].rec.y -= shoot[i].speed.y;
                        break;

                    // [Bottom-Left]
                    case 5:
                        shoot[i].rec.x -= shoot[i].speed.x;
                        shoot[i].rec.y += shoot[i].speed.y;
                        break;

                    // [Bottom-Right]
                    case 4:
                        shoot[i].rec.x += shoot[i].speed.x;
                        shoot[i].rec.y += shoot[i].speed.y;
                        break;

                    // [Right]
                    case 3:
                        shoot[i].rec.x += shoot[i].speed.x;
                        break;

                    // [Left]
                    case 2:
                        shoot[i].rec.x -= shoot[i].speed.x;
                        break;

                    // [Top]
                    case 1:
                        shoot[i].rec.y -= shoot[i].speed.y;
                        break;

                    // [Bottom]
                    case 0:
                        shoot[i].rec.y += shoot[i].speed.y;
                        break;

                    default:
                        break;
                    }

                    // Collision with enemy
                    for (int j = 0; j < activeEnemies; j++)
                    {
                        if (enemy[j].active)
                        {
                            if (CheckCollisionRecs(shoot[i].rec, enemy[j].enemyDest))
                            {
                                PlaySound(damageDone.sound);
                                shoot[i].active = false;
                                enemy[j].life--;

                                if (enemy[j].life == 0)
                                {
                                    if (j % 4 == 0)
                                    {
                                        enemy[j].enemyDest.x = GetRandomValue(GetScreenWidth(), GetScreenWidth() + 1000);
                                        enemy[j].enemyDest.y = GetRandomValue(0, GetScreenHeight() - enemy[i].enemyDest.height);
                                        enemy[j].active = false;
                                    }

                                    else if (j % 4 == 1)
                                    {
                                        enemy[j].enemyDest.x = GetRandomValue(-1000, 0);
                                        enemy[j].enemyDest.y = GetRandomValue(0, GetScreenHeight() - enemy[i].enemyDest.height);
                                        enemy[j].active = false;
                                    }

                                    else if (j % 4 == 2)
                                    {
                                        enemy[j].enemyDest.x = GetRandomValue(0, GetScreenWidth() - enemy[i].enemyDest.width);
                                        enemy[j].enemyDest.y = GetRandomValue(GetScreenHeight(), GetScreenHeight() + 1000);
                                        enemy[j].active = false;
                                    }

                                    else if (j % 4 == 3)
                                    {
                                        enemy[j].enemyDest.x = GetRandomValue(0, GetScreenWidth() - enemy[i].enemyDest.width);
                                        enemy[j].enemyDest.y = GetRandomValue(-1000, 0);
                                        enemy[j].active = false;
                                    }

                                    // Restore life
                                    switch (enemy[j].type)
                                    {
                                    case 1:
                                        enemy[j].life = 1;
                                        break;

                                    case 2:
                                        enemy[j].life = 2;
                                        break;

                                    case 3:
                                        enemy[j].life = 3;
                                        break;

                                    default:
                                        break;
                                    }

                                    enemiesKill++;
                                    score += 100;
                                }
                                // shootRate = 0;
                            }

                            if (shoot[i].rec.x >= GetScreenWidth())
                            {
                                shoot[i].active = false;
                                // shootRate = 0;
                            }

                            if (shoot[i].rec.x < -shoot[i].rec.width)
                            {
                                shoot[i].active = false;
                                // shootRate = 0;
                            }

                            if (shoot[i].rec.y < -shoot[i].rec.height)
                            {
                                shoot[i].active = false;
                                // shootRate = 0;
                            }

                            if (shoot[i].rec.y >= GetScreenHeight())
                            {
                                shoot[i].active = false;
                                // shootRate = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

//------------------------------------------------------------------------------------
// Draw game (one frame)
//------------------------------------------------------------------------------------
void DrawGame(void)
{
    ClearBackground(DARKGRAY);

    if (!gameOver)
    {
        DrawTexturePro(backgroundMain, bgSrc, bgDest, bgOrigin, 0, WHITE);

        // Rectangle for tracking character position (testes!)
        // DrawRectangle(player.playerDest.x, player.playerDest.y, player.playerDest.width, player.playerDest.height, BLUE);
        DrawTexturePro(shadow.playerSprite, shadow.playerSrc, shadow.playerDest, shadow.origin, 0, WHITE);
        DrawTexturePro(player.playerSprite, player.playerSrc, player.playerDest, player.origin, 0, WHITE);

        // Draw player's life
        DrawTexturePro(playerLife[0].life, playerLife[0].lifeSrc, playerLife[0].lifeDest, playerLife[0].origin, 0, WHITE);
        DrawTexturePro(playerLife[1].life, playerLife[1].lifeSrc, playerLife[1].lifeDest, playerLife[1].origin, 0, WHITE);
        DrawTexturePro(playerLife[2].life, playerLife[2].lifeSrc, playerLife[2].lifeDest, playerLife[2].origin, 0, WHITE);

        if (wave == FIRST)
            DrawText("FIRST WAVE", GetScreenWidth() / 2 - MeasureText("FIRST WAVE", 40) / 2, GetScreenHeight() / 2 - 40, 40, Fade(RAYWHITE, alpha));
        else if (wave == SECOND)
            DrawText("SECOND WAVE", GetScreenWidth() / 2 - MeasureText("SECOND WAVE", 40) / 2, GetScreenHeight() / 2 - 40, 40, Fade(RAYWHITE, alpha));
        else if (wave == THIRD)
            DrawText("THIRD WAVE", GetScreenWidth() / 2 - MeasureText("THIRD WAVE", 40) / 2, GetScreenHeight() / 2 - 40, 40, Fade(RAYWHITE, alpha));
        else if (wave == BOSS)
            DrawText("SURVIVE!", GetScreenWidth() / 2 - MeasureText("SURVIVE!", 40) / 2, GetScreenHeight() / 2 - 40, 40, Fade(RAYWHITE, alpha));

        for (int i = 0; i < activeEnemies; i++)
        {
            if (enemy[i].active)
                DrawTexturePro(enemy[i].enemySprite, enemy[i].enemySrc, enemy[i].enemyDest, enemy[i].origin, 0, WHITE);
        }

        for (int i = 0; i < NUM_SHOOTS; i++)
        {
            // Draw Shuriken (character basic atk)
            if (shoot[i].active)
                DrawTexturePro(shoot[i].shootSprite, shoot[i].shootSrc, shoot[i].rec, shoot[i].origin, 0, WHITE);
        }

        DrawText(TextFormat("%04i", score), 40, 40, 40, RAYWHITE);

        if (victory)
            DrawText("YOU WIN", GetScreenWidth() / 2 - MeasureText("YOU WIN", 40) / 2, GetScreenHeight() / 2 - 40, 40, RAYWHITE);

        if (pause)
            DrawText("GAME PAUSED", GetScreenWidth() / 2 - MeasureText("GAME PAUSED", 40) / 2, GetScreenHeight() / 2 - 40, 40, GRAY);

        if (rulesOpen)
        {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){0, 0, 0, 160});
            DrawTexturePro(rules, (Rectangle){0, 0, 415, 618}, (Rectangle){GetScreenWidth() / 2, GetScreenHeight() / 2, 415, 618}, (Vector2){207.5, 309}, 0, WHITE);
        }
    }
    else
        DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);
}

//------------------------------------------------------------------------------------
// Update Logo (one frame)
//------------------------------------------------------------------------------------
void UpdateLogo(void)
{
    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();
    bgSrc.width = 890;
    bgSrc.height = 470;

    // Background music for logo screen
    UpdateMusicStream(backgroundMenu.song);
    PlayMusicStream(backgroundMenu.song);
}

//------------------------------------------------------------------------------------
// Draw Logo (one frame)
//------------------------------------------------------------------------------------
void DrawLogo(void)
{
    DrawTexturePro(backgroundLogo, bgSrc, bgDest, bgOrigin, 0, WHITE);
}

//------------------------------------------------------------------------------------
// Update Title (one frame)
//------------------------------------------------------------------------------------
void UpdateTitle(void)
{
    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();
    bgSrc.width = 890;
    bgSrc.height = 470;

    // Keeps the music playing
    UpdateMusicStream(backgroundMenu.song);
    PlayMusicStream(backgroundMenu.song);

    btnBounds.x = GetScreenWidth() / 1.985 - button.width / 2;
    btnBounds.y = GetScreenHeight() / 1.65 + button.height / 2;

    creditsBounds.x = GetScreenWidth() - 75;
    creditsBounds.y = GetScreenHeight() - 75;

    mousePoint = GetMousePosition();
    btnAction = false;
    btnActionCredits = false;

    // Check start button state
    if (CheckCollisionPointRec(mousePoint, btnBounds))
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            button = LoadTexture("Assets/NinjaAdventure/HUD/play_d.png");
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            btnAction = true;
    }
    else
    {
        button = LoadTexture("Assets/NinjaAdventure/HUD/play_c.png");
    }

    // Check credits button state
    if (CheckCollisionPointRec(mousePoint, creditsBounds))
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !opened)
        {
            buttonCredits = LoadTexture("Assets/NinjaAdventure/HUD/credits_d.png");
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            btnActionCredits = true;
    }
    else
    {
        buttonCredits = LoadTexture("Assets/NinjaAdventure/HUD/credits_a.png");
    }

    if (btnAction)
    {
        PlaySound(fxButton);
        isPressed = true;
        rulesOpen = true;
        InitGame();
        gameOver = false;
    }

    if (btnActionCredits && !opened)
    {
        PlaySound(fxButton);
        isPressedCredits = true;
        opened = true;
    }

    // Close credits
    if (CheckCollisionPointRec(mousePoint, (Rectangle){1010, 205, 13, 13}))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isPressedCredits = false;
            opened = false;
        }
    }
}

//------------------------------------------------------------------------------------
// Draw Title (one frame)
//------------------------------------------------------------------------------------
void DrawTitle(void)
{
    DrawTexturePro(backgroundTitle, bgSrc, bgDest, bgOrigin, 0, WHITE);

    // Draw button frame
    DrawTextureRec(button, sourceRec, (Vector2){btnBounds.x, btnBounds.y}, WHITE);
    DrawTextureRec(buttonCredits, creditsRec, (Vector2){creditsBounds.x, creditsBounds.y}, WHITE);

    if (isPressedCredits)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){0, 0, 0, 100});
        DrawTexturePro(credits, (Rectangle){0, 0, 500, 540}, (Rectangle){GetScreenWidth() / 2, GetScreenHeight() / 2, 500, 540}, (Vector2){250, 270}, 0, WHITE);
    }
}

//------------------------------------------------------------------------------------
// Update Narrative (one frame)
//------------------------------------------------------------------------------------
void UpdateNarrative(void)
{

    UpdateMusicStream(narrativeMusic.song);
    PlayMusicStream(narrativeMusic.song);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        narrativeScreen++;
        PlaySound(continueNarrative.sound);
    }
}

//------------------------------------------------------------------------------------
// Draw Narrative (one frame)
//------------------------------------------------------------------------------------
void DrawNarrative(void)
{
    DrawTexturePro(narrative, (Rectangle){0, 900 * narrativeScreen, 1600, 900}, (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, (Vector2){0, 0}, 0, WHITE);

    countNarrative -= 0.1;
    if (countNarrative >= 0)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), CLITERAL(Color){23, 29, 23, countNarrative});
    }
}

//------------------------------------------------------------------------------------
// Drawn Different Screens [general]
//------------------------------------------------------------------------------------
void DrawScreen()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    switch (currentScreen)
    {
    case LOGO:
    {
        DrawLogo();
    }
    break;
    case TITLE:
    {
        DrawTitle();
    }
    break;
    case NARRATIVE:
    {
        DrawNarrative();
    }
    break;
    case GAMEPLAY:
    {
        DrawGame();
    }
    break;
    case ENDING:
    {
        DrawEnd();
        // TODO: Create and Instantiate drawEnding function;
    }
    break;
    default:
        break;
    }

    EndDrawing();
}

//------------------------------------------------------------------------------------
// Unload game variables
//------------------------------------------------------------------------------------
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
    UnloadTexture(player.playerSprite);
    UnloadTexture(shadow.playerSprite);
    UnloadTexture(playerLife[0].life);
    UnloadTexture(playerLife[1].life);
    UnloadTexture(playerLife[2].life);
    UnloadTexture(backgroundLogo);
    UnloadTexture(backgroundTitle);
    UnloadTexture(backgroundMain);
    UnloadTexture(narrative);
    UnloadTexture(credits);
    UnloadTexture(button);
    UnloadTexture(rules);
    UnloadMusicStream(backgroundMusic.song);
    UnloadMusicStream(backgroundMenu.song);
    UnloadMusicStream(narrativeMusic.song);
    UnloadSound(gameOverSound.sound);
    UnloadSound(damageTaken.sound);
    UnloadSound(fxButton);

    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        // Unload all shurikens
        UnloadTexture(shoot[i].shootSprite);
    }

    for (int i = 0; i < NUM_MAX_ENEMIES; i++)
    {
        UnloadTexture(enemy[i].enemySprite);
    }
}

void scorerank(void)
{
    FILE *arq;
    Playerscore reg = {0};
    Playerscore temp = {0};

    player1.fscore = score;

    arq = fopen("rankscore.bin", "r+b");

    if (arq == NULL)
    {
        arq = fopen("rankscore.bin", "w+b");

        fwrite(&player1, sizeof(Playerscore), 1, arq);

        for (int i = 0; i < 9; i++)
        {
            fwrite(&reg, sizeof(Playerscore), 1, arq);
        }
        fclose(arq);
    }
    else
    {
        for (int i = 0; i < 10; i++)
        {
            fread(&reg, sizeof(Playerscore), 1, arq);
            if (player1.fscore > reg.fscore)
            {
                temp = reg;

                fseek(arq, -sizeof(Playerscore), 1);

                fwrite(&player1, sizeof(Playerscore), 1, arq);

                player1 = temp;
            }
        }
        fclose(arq);
    }
    for (int i = 0; i < 11; i++)
    {
        printf("%s", player1.name);
    }
}

void Input_text(void)
{
    if (CheckCollisionPointRec(GetMousePosition(), textBox))
        mouseOnText = true;
    else
        mouseOnText = false;

    if (mouseOnText)
    {
        // Set the window's cursor to the I-Beam
        SetMouseCursor(MOUSE_CURSOR_IBEAM);

        // Get char pressed (unicode character) on the queue
        int key = GetCharPressed();

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125) && (letterCount < 10))
            {
                player1.name[letterCount] = (char)key;
                player1.name[letterCount + 1] = '\0'; // Add null terminator at the end of the string.
                letterCount++;
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            letterCount--;
            if (letterCount < 0)
                letterCount = 0;
            player1.name[letterCount] = '\0';
        }
    }
    else
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (mouseOnText)
        framesCounter++;
    else
        framesCounter = 0;
}

void UpdateEnd(void)
{
    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();
    bgSrc.width = 890;
    bgSrc.height = 470;

    // Background music for logo screen
    UpdateMusicStream(backgroundMenu.song);
    PlayMusicStream(backgroundMenu.song);

    if (IsKeyPressed(KEY_ENTER))
        endcount = true;

    Input_text();

    if (endcount)
    {
        if(gameOver == false)
        {
            scorerank();
            gameOver = true;
        }

        if (KEY_ENTER)
        {
            FILE *arq;
            arq = fopen("rankscore.bin", "rb");

            for (int i = 0; i < 10; i++)
            {
                fread(&rankplayer[i], sizeof(Playerscore), 1, arq);
            }
        }
    }
}

void DrawEnd(void)
{
    ClearBackground(RAYWHITE);
    if (IsKeyPressed(KEY_ENTER))
        endcount = true;

    if (endcount)
    {
        DrawText("RANK", GetScreenWidth() / 2 - MeasureText("RANK", 20) / 2, 40, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[0].name), 800, 80, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[0].fscore), 1000, 80, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[1].name), 800, 160, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[1].fscore), 1000, 160, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[2].name), 800, 240, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[2].fscore), 1000, 240, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[3].name), 800, 320, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[3].fscore), 1000, 320, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[4].name), 800, 400, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[4].fscore), 1000, 400, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[5].name), 800, 480, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[5].fscore), 1000, 480, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[6].name), 800, 560, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[6].fscore), 1000, 560, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[7].name), 800, 640, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[7].fscore), 1000, 640, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[8].name), 800, 720, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[8].fscore), 1000, 720, 20, GRAY);
        DrawText(TextFormat("%s", rankplayer[9].name), 800, 800, 20, GRAY);
        DrawText(TextFormat("%04i", rankplayer[9].fscore), 1000, 800, 20, GRAY);
    }

    if (endcount == false)
    {
        DrawText("PLACE MOUSE OVER INPUT BOX!", GetScreenWidth() / 2 - MeasureText("PLACE MOUSE OVER INPUT BOX!", 20) / 2, 140, 20, GRAY);

        DrawRectangleRec(textBox, LIGHTGRAY);
        if (mouseOnText)
            DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
        else
            DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);

        DrawText(player1.name, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);

        DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, 10), GetScreenWidth() / 2 - MeasureText("PLACE MOUSE OVER INPUT BOX!", 20) / 2, 250, 20, DARKGRAY);

        if (mouseOnText)
        {
            if (letterCount < 10)
            {
                // Draw blinking underscore char
                DrawText("_", (int)textBox.x + 8 + MeasureText(player1.name, 40), (int)textBox.y + 12, 40, MAROON);
            }
            else
                DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
        }
    }
}