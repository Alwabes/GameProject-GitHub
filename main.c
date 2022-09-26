#include <stdio.h>
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define NUM_SHOOTS 50
#define NUM_MAX_ENEMIES 50
#define FIRST_WAVE 10
#define SECOND_WAVE 20
#define THIRD_WAVE 50

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen { LOGO = 0, TITLE, GAMEPLAY, ENDING } GameScreen;
typedef enum { FIRST = 0, SECOND, THIRD } EnemyWave;

typedef struct Player{
    Rectangle playerDest;
    Rectangle playerSrc;
    Vector2 origin;
    Vector2 speed;
    Color color;
    Texture2D playerSprite;
} Player;

typedef struct Life{
    Texture2D life;
    Rectangle lifeDest;
    Rectangle lifeSrc;
    Vector2 origin;
} Life;

typedef struct Enemy{
    Rectangle rec;
    Vector2 speed;
    bool active;
    bool free; // for walking freely
    Color color;
} Enemy;

typedef struct Shoot{
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
    int bulletDirection;
} Shoot;

typedef struct Song{
    bool musicPaused;
    Music song;
} Song;

typedef struct SoundEffect{
    bool soundPaused;
    Sound sound;
} SoundEffect;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------

// Not static const for fullscreen option
int screenWidth = 1200;
int screenHeight = 720;

static bool gameOver = false;
static bool pause =  false;
static int score = 0;
static bool victory = false;

static Player player = { 0 };
static Life playerLife[3] = { 0 };
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Shoot shoot[NUM_SHOOTS] = { 0 };
static EnemyWave wave = { 0 };

static int shootRate = 0;
static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;

// Moving animation variables
bool moving, alive = true;
int direction, frameCount, playerFrame;

// Player's life count
int count = 2;
int invencibleCount;
int damageAnimCount;
bool colision = true;
bool damageAnim = false;

// Timer variables
int timerCount = 0;

// Music variables
Song backgroundMusic = { 0 };
Song backgroundMenu = { 0 };
SoundEffect gameOverSound = { 0 };
SoundEffect damageTaken = { 0 };

// Button variables
Sound fxButton; 
Texture2D button;
Rectangle sourceRec; // Based on size of image
Rectangle btnBounds;
bool btnAction = false; 
bool isPressed = false;
Vector2 mousePoint = { 0, 0 };

// Current Screen variable
GameScreen currentScreen = LOGO;
Texture2D backgroundLogo, backgroundTitle;
Rectangle bgSrc;
Rectangle bgDest;
Vector2 bgOrigin;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
void InitGame(void);         // Initialize game
void UpdateGame(void);       
void DrawGame(void);        
void UpdateLogo(void);         
void DrawLogo(void);         
void UpdateTitle(void);    
void DrawTitle(void);      
void UnloadGame(void);       
void UpdateDrawFrame(void);  
void DrawScreen(void);       // Draw the current screen

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    Image windowIcon = LoadImage("Assets/NinjaAdventure/icon.png");

    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "NINJA DEFENDERS");
    SetWindowIcon(windowIcon);
    InitAudioDevice();
    InitGame();

    int framesCounter = 0; 

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 144, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        switch(currentScreen)
        {
            case LOGO:
            {
                UpdateLogo();

                framesCounter++; 

                // Wait for 4 seconds (240 frames) before jumping to TITLE screen
                if (framesCounter == 240)
                {
                    currentScreen = TITLE;
                }
            } break;
            case TITLE:
            {
                UpdateTitle();

                // Press enter to change to GAMEPLAY screen
                if (isPressed)
                {
                    currentScreen = GAMEPLAY;
                    isPressed = false;
                }
            } break;
            case GAMEPLAY:
            {
                UpdateGame();

                // Press R to change to ENDING screen
                if (IsKeyPressed(KEY_R))
                {
                    currentScreen = ENDING;
                }
            } break;
            case ENDING:
            {
                // TODO: Update ENDING screen variables here!

                // Press enter to return to TITLE screen
                if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
                {
                    currentScreen = TITLE;
                }
            } break;
            default: break;
        }

        //Draw the current screen
        DrawScreen();
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)
    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
void InitGame(void)
{

    // Secure that the game will start properly
    alive = true;
    count = 2;
    timerCount = 0;
    damageAnim = false;

    // Initialize background variables
    backgroundLogo = LoadTexture("Assets/NinjaAdventure/Backgrounds/background.png");
    backgroundTitle = LoadTexture("Assets/NinjaAdventure/Backgrounds/backgroud_titlescreen.png");
    bgSrc.x = 0;
    bgSrc.y = 0;
    bgSrc.width = 890;
    bgSrc.height = 470;
    bgDest.x = 0;
    bgDest.y = 0;
    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();
    bgOrigin.x = 0;
    bgOrigin.y = 0;

    // Initialize audio variables
    backgroundMusic.song = LoadMusicStream("Assets/NinjaAdventure/Musics/4 - Village.ogg");
    SetMusicVolume(backgroundMusic.song,  0.3);

    backgroundMenu.song = LoadMusicStream("Assets/NinjaAdventure/Musics/1 - Adventure Begin.ogg");
    SetMusicVolume(backgroundMenu.song,  0.3);
    
    gameOverSound.sound = LoadSound("Assets/NinjaAdventure/Sounds/Game/GameOver.wav");
    SetSoundVolume(gameOverSound.sound, 0.5);

    damageTaken.sound = LoadSound("Assets/NinjaAdventure/Sounds/Game/Hit4.wav");
    SetSoundVolume(damageTaken.sound, 0.4);

    // Initialize Button variables
    fxButton = LoadSound("Assets/NinjaAdventure/Sounds/Menu/Menu9.wav");   // Load button sound
    button = LoadTexture("Assets/NinjaAdventure/HUD/Play_Unpressed.png"); // Load button texture
    sourceRec.x = 0;
    sourceRec.y = 0;
    sourceRec.width = 160;
    sourceRec.height = 81; 
    btnBounds.x = GetScreenWidth()/1.985 - button.width/2; 
    btnBounds.y = GetScreenHeight()/1.85 + button.height/2;
    btnBounds.width = 160;
    btnBounds.height = 81;

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

    // Initialize player
    player.playerSrc.x =  0;
    player.playerSrc.y = 0;
    player.playerSrc.width = 16;
    player.playerSrc.height = 16.2;
    player.playerDest.x =  100;
    player.playerDest.y = 100;
    player.playerDest.width = 34;
    player.playerDest.height = 34;
    player.origin.x = 0;
    player.origin.y = 0;
    player.speed.x = 3.5;
    player.speed.y = 3.5;
    player.color = BLUE;
    player.playerSprite = LoadTexture ("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/walk.png");

    // Initialize player's life
    playerLife[0].life = LoadTexture("Assets/NinjaAdventure/HUD/Heart.png");
    playerLife[0].lifeSrc.x = 0;
    playerLife[0].lifeSrc.y = 0;
    playerLife[0].lifeSrc.width = 16.2;
    playerLife[0].lifeSrc.height = 16.2;
    playerLife[0].lifeDest.x = 40;
    playerLife[0].lifeDest.y = 0;
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
    playerLife[1].lifeDest.y = 0;
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
    playerLife[2].lifeDest.y = 0;
    playerLife[2].lifeDest.width = 32;
    playerLife[2].lifeDest.height = 32;
    playerLife[1].origin.x = 0;
    playerLife[1].origin.y = 0;

    // Initialize right side enemies
    for (int i = 0; i < NUM_MAX_ENEMIES; i += 3)
    {
        enemy[i].rec.width = 10;
        enemy[i].rec.height = 10;
        enemy[i].rec.x = GetRandomValue(GetScreenWidth(), GetScreenWidth() + 1000);
        enemy[i].rec.y = GetRandomValue(0, GetScreenHeight() - enemy[i].rec.height);
        enemy[i].speed.x = 1.2;
        enemy[i].speed.y = 1.2;
        enemy[i].active = true;
        enemy[i].color = GRAY;
        enemy[i].free = false;
    }

    // Initialize left side enemies
    for (int i = 1; i < NUM_MAX_ENEMIES; i += 3)
    {
        enemy[i].rec.width = 10;
        enemy[i].rec.height = 10;
        enemy[i].rec.x = GetRandomValue(-1000, 0);
        enemy[i].rec.y = GetRandomValue(0, GetScreenHeight() - enemy[i].rec.height);
        enemy[i].speed.x = 1.2;
        enemy[i].speed.y = 1.2;
        enemy[i].active = true;
        enemy[i].color = GRAY;
        enemy[i].free = false;
    }

    // Initialize bottom side enemies
    for (int i = 2; i < NUM_MAX_ENEMIES; i += 3)
    {
        enemy[i].rec.width = 10;
        enemy[i].rec.height = 10;
        enemy[i].rec.x = GetRandomValue(0, GetScreenWidth() - enemy[i].rec.width);
        enemy[i].rec.y = GetRandomValue(GetScreenHeight(), GetScreenHeight() + 1000);
        enemy[i].speed.x = 1.2;
        enemy[i].speed.y = 1.2;
        enemy[i].active = true;
        enemy[i].color = GRAY;
        enemy[i].free = false;
    }

    // Initialize shoots
    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        shoot[i].rec.x = player.playerDest.x + player.playerDest.width/2.5;
        shoot[i].rec.y = player.playerDest.y + player.playerDest.height/2;
        shoot[i].rec.width = 7;
        shoot[i].rec.height = 7;
        shoot[i].speed.x = 7;
        shoot[i].speed.y = 7;
        shoot[i].active = false;
        shoot[i].color = MAROON;
    }
}

//------------------------------------------------------------------------------------
// Update game (one frame)
//------------------------------------------------------------------------------------
void UpdateGame(void)
{

    // Adjusting visual elements on resizabled window 
    // !player's life
    playerLife[0].lifeDest.y = GetScreenHeight() - 60;
    playerLife[1].lifeDest.y = GetScreenHeight() - 60;
    playerLife[2].lifeDest.y = GetScreenHeight() - 60;

    if (!gameOver)
    {
        // Background music
        UpdateMusicStream(backgroundMusic.song);
        PlayMusicStream(backgroundMusic.song);

        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            switch (wave)
            {
                case FIRST:
                {
                    if (!smooth)
                    {
                        alpha += 0.02f;

                        if (alpha >= 1.0f) smooth = true;
                    }

                    if (smooth) alpha -= 0.02f;

                    if (enemiesKill == activeEnemies)
                    {
                        enemiesKill = 0;

                        for (int i = 0; i < activeEnemies; i++)
                        {
                            if (!enemy[i].active) enemy[i].active = true;
                        }

                        activeEnemies = SECOND_WAVE;
                        wave = SECOND;
                        smooth = false;
                        alpha = 0.0f;
                    }
                } break;
                case SECOND:
                {
                    if (!smooth)
                    {
                        alpha += 0.02f;

                        if (alpha >= 1.0f) smooth = true;
                    }

                    if (smooth) alpha -= 0.02f;

                    if (enemiesKill == activeEnemies)
                    {
                        enemiesKill = 0;

                        for (int i = 0; i < activeEnemies; i++)
                        {
                            if (!enemy[i].active) enemy[i].active = true;
                        }

                        activeEnemies = THIRD_WAVE;
                        wave = THIRD;
                        smooth = false;
                        alpha = 0.0f;
                    }
                } break;
                case THIRD:
                {
                    if (!smooth)
                    {
                        alpha += 0.02f;

                        if (alpha >= 1.0f) smooth = true;
                    }

                    if (smooth) alpha -= 0.02f;

                    if (enemiesKill == activeEnemies) victory = true;

                } break;
                default: break;
            }

            // Player movement
            moving = false;

            if (alive){
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
                    player.playerDest.x += player.speed.x;
                    direction = 3;
                    moving = true;
                }

                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
                    player.playerDest.x -= player.speed.x;
                    direction = 2;
                    moving = true;
                }

                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
                    player.playerDest.y -= player.speed.y;
                    direction = 1;
                    moving = true;
                }

                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
                    player.playerDest.y += player.speed.y;
                    direction = 0;
                    moving = true;
                }
            }

            player.playerSrc.y = 0;

            if (moving){
                if (frameCount % 10 == 1)
                    playerFrame++;

                player.playerSrc.y = player.playerSrc.width * playerFrame;
            }

            frameCount++;

            // Reset the animation
            if (playerFrame > 3)
                playerFrame = 0;

            player.playerSrc.x = player.playerSrc.width * direction;

            // Player collision with enemy
            for (int i = 0; i < activeEnemies; i++)
            {
                if (alive) {
                    if (CheckCollisionRecs(player.playerDest, enemy[i].rec) && colision){
                        playerLife[count].lifeSrc.x = (playerLife[count].lifeSrc.width * 4) - 0.8;
                        PlaySound(damageTaken.sound);
                        
                        count--;
                        colision = false;
                        damageAnim = true;
                        damageAnimCount = 0;
                        invencibleCount = 0;
                    }

                    // Damage "animation" indicator
                    if (damageAnim){
                        if (damageAnimCount == 0 || damageAnimCount == 200)
                            player.playerSprite = LoadTexture ("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/Damage.png");
                        else if (damageAnimCount == 100 || damageAnimCount == 300)
                            player.playerSprite = LoadTexture ("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/walk.png");

                        damageAnimCount++;

                        if (damageAnimCount > 300)
                            damageAnim = false;
                    }

                    // Player can't take damage while "invencible" is activated
                    invencibleCount++;
                    if (invencibleCount > 300){
                        colision = true;
                    }
                }

                // When player is dead
                if (count == -1){
                    StopMusicStream(backgroundMusic.song);

                    player.playerSprite = LoadTexture ("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/Dead.png");
                    player.playerSrc.x = 0;
                    player.playerSrc.y = 0;
                    alive = false;

                    PlaySound(gameOverSound.sound);

                    timerCount++;

                    if (timerCount > 500)
                        gameOver = true;
                }

            }

            // Enemies will only follow after a certain time 

            // Right enemy behaviour
            for (int i = 0; i < activeEnemies; i += 3)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].rec.x > GetScreenWidth() - 25)
                        enemy[i].rec.x -= enemy[i].speed.x;
                    if (enemy[i].rec.x < GetScreenWidth() - 25)
                        enemy[i].free = true;
                }
            }

            // Left enemy behaviour
            for (int i = 1; i < activeEnemies; i += 3)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].rec.x < 25)
                        enemy[i].rec.x += enemy[i].speed.x;

                    if (enemy[i].rec.x > 25)
                        enemy[i].free = true;
                }
            }

            
            // Bottom enemy behaviour
            for (int i = 2; i < activeEnemies; i += 3)
            {
                if (enemy[i].active)
                {
                    if (enemy[i].rec.y > GetScreenHeight() - 25)
                        enemy[i].rec.x -= enemy[i].speed.x;
                    if (enemy[i].rec.x < GetScreenHeight() - 25)
                        enemy[i].free = true;
                }
            }

            // General enemy behaviour (follow player)
            for (int i = 0; i < activeEnemies; i ++)
            {
                if (enemy[i].active && enemy[i].free)
                {   
                    if (player.playerDest.x < enemy[i].rec.x)
                        enemy[i].rec.x -= enemy[i].speed.x;

                    if (player.playerDest.x > enemy[i].rec.x)
                        enemy[i].rec.x += enemy[i].speed.x;

                    if (player.playerDest.y < enemy[i].rec.y)
                        enemy[i].rec.y -= enemy[i].speed.y;

                    if (player.playerDest.y > enemy[i].rec.y)
                        enemy[i].rec.y += enemy[i].speed.y;

                }
            }

            // Wall behaviour
            if (player.playerDest.x <= 0) player.playerDest.x = 0;
            if (player.playerDest.x + player.playerDest.width >= GetScreenWidth()) player.playerDest.x = GetScreenWidth() - player.playerDest.width;
            if (player.playerDest.y <= 0) player.playerDest.y = 0;
            if (player.playerDest.y + player.playerDest.height >= GetScreenHeight()) player.playerDest.y = GetScreenHeight() - player.playerDest.height;

            // Shoot initialization
            if (IsKeyDown(KEY_SPACE))
            {
                shootRate += 5;

                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (!shoot[i].active && shootRate%20 == 0)
                    {
                        shoot[i].rec.x = player.playerDest.x + player.playerDest.width/2.5;
                        shoot[i].rec.y = player.playerDest.y + player.playerDest.height/2;
                        shoot[i].active = true;

                        // Bullet Movement
                        // Using variable direction to see where's the player shooting
                        // Using bulletDirection to define where's the bullet going.
                        if (direction == 0)
                            shoot[i].bulletDirection = 0; 

                        if (direction == 1)
                            shoot[i].bulletDirection = 1; 

                        if (direction == 2)
                            shoot[i].bulletDirection = 2; 

                        if (direction == 3)
                            shoot[i].bulletDirection = 3;

                        break; 
                    }
                }
            }

            // Shoot logic
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active)
                {
                    // bulletDirection: 
                    // [Bottom]
                    if (shoot[i].bulletDirection == 0)
                        shoot[i].rec.y += shoot[i].speed.y;

                    // [Top]
                    if (shoot[i].bulletDirection == 1)
                        shoot[i].rec.y -= shoot[i].speed.y;

                    // [Left]
                    if (shoot[i].bulletDirection == 2)
                        shoot[i].rec.x -= shoot[i].speed.x;

                    // [Right]
                    if (shoot[i].bulletDirection == 3)
                        shoot[i].rec.x += shoot[i].speed.x;
                
                    // Collision with enemy
                    for (int j = 0; j < activeEnemies; j++)
                    {
                        if (enemy[j].active)
                        {
                            if (CheckCollisionRecs(shoot[i].rec, enemy[j].rec))
                            {
                                shoot[i].active = false;
                                enemy[j].rec.x = GetRandomValue(GetScreenWidth(), GetScreenWidth() + 1000);
                                enemy[j].rec.y = GetRandomValue(0, GetScreenHeight() - enemy[j].rec.height);
                                shootRate = 0;
                                enemiesKill++;
                                score += 100;
                            }

                            if (shoot[i].rec.x + shoot[i].rec.width >= GetScreenWidth())
                            {
                                shoot[i].active = false;
                                shootRate = 0;
                            }

                            if (shoot[i].rec.x < -shoot[i].rec.width){
                                shoot[i].active = false;
                                shootRate = 0;
                            }

                            if (shoot[i].rec.y < -shoot[i].rec.height){
                                shoot[i].active = false;
                                shootRate = 0;
                            }

                            if (shoot[i].rec.y + shoot[i].rec.height >= GetScreenHeight()){
                                shoot[i].active = false;
                                shootRate = 0;
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
// Update Logo (one frame)
//------------------------------------------------------------------------------------
void UpdateLogo(void){

    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();

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
void UpdateTitle(void){

    bgDest.width = GetScreenWidth();
    bgDest.height = GetScreenHeight();

    // Keeps the music playing
    UpdateMusicStream(backgroundMenu.song);
    PlayMusicStream(backgroundMenu.song);

    btnBounds.x = GetScreenWidth()/1.985 - button.width/2; 
    btnBounds.y = GetScreenHeight()/1.85 + button.height/2;

    mousePoint = GetMousePosition();
    btnAction = false;

    // Check button state
    if (CheckCollisionPointRec(mousePoint, btnBounds))
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            button = LoadTexture("Assets/NinjaAdventure/HUD/Play_Pressed_n.png");
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) 
            btnAction = true;
    }
    else 
    {
        button = LoadTexture("Assets/NinjaAdventure/HUD/Play_Unpressed_n.png");
    }

    if (btnAction)
    {
        PlaySound(fxButton);
        isPressed = true;
        InitGame();
        gameOver = false;
    }

    // Calculate button frame rectangle to draw depending on button state
}

//------------------------------------------------------------------------------------
// Draw Title (one frame)
//------------------------------------------------------------------------------------
void DrawTitle(void)
{
    DrawTexturePro(backgroundTitle, bgSrc, bgDest, bgOrigin, 0, WHITE);

    // Draw button frame
    DrawTextureRec(button, sourceRec, (Vector2){ btnBounds.x, btnBounds.y }, WHITE); 
}

//------------------------------------------------------------------------------------
// Draw game (one frame)
//------------------------------------------------------------------------------------
void DrawGame(void)
{
    ClearBackground(RAYWHITE);

    if (!gameOver)
        {
            // Rectangle for tracking character position (testes!)
            // DrawRectangle(player.playerDest.x, player.playerDest.y, player.playerDest.width, player.playerDest.height, BLUE);
            DrawTexturePro(player.playerSprite, player.playerSrc, player.playerDest, player.origin, 0, WHITE);

            // Draw player's life
            DrawTexturePro(playerLife[0].life, playerLife[0].lifeSrc, playerLife[0].lifeDest, playerLife[0].origin, 0, WHITE);
            DrawTexturePro(playerLife[1].life, playerLife[1].lifeSrc, playerLife[1].lifeDest, playerLife[1].origin, 0, WHITE);
            DrawTexturePro(playerLife[2].life, playerLife[2].lifeSrc, playerLife[2].lifeDest, playerLife[2].origin, 0, WHITE);

            if (wave == FIRST) DrawText("FIRST WAVE", GetScreenWidth()/2 - MeasureText("FIRST WAVE", 40)/2, GetScreenHeight()/2 - 40, 40, Fade(BLACK, alpha));
            else if (wave == SECOND) DrawText("SECOND WAVE",GetScreenWidth()/2 - MeasureText("SECOND WAVE", 40)/2, GetScreenHeight()/2 - 40, 40, Fade(BLACK, alpha));
            else if (wave == THIRD) DrawText("THIRD WAVE", GetScreenWidth()/2 - MeasureText("THIRD WAVE", 40)/2, GetScreenHeight()/2 - 40, 40, Fade(BLACK, alpha));

            for (int i = 0; i < activeEnemies; i++)
            {
                if (enemy[i].active) DrawRectangleRec(enemy[i].rec, enemy[i].color);
            }

            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active) DrawRectangleRec(shoot[i].rec, shoot[i].color);
            }

            DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);

            if (victory) DrawText("YOU WIN", GetScreenWidth()/2 - MeasureText("YOU WIN", 40)/2, GetScreenHeight()/2 - 40, 40, BLACK);

            if (pause) DrawText("GAME PAUSED", GetScreenWidth()/2 - MeasureText("GAME PAUSED", 40)/2, GetScreenHeight()/2 - 40, 40, GRAY);
        }
    else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

}

//------------------------------------------------------------------------------------
// Drawn Different Screens [general]
//------------------------------------------------------------------------------------
void DrawScreen()
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        switch(currentScreen)
        {
            case LOGO:
            {
                DrawLogo();

            } break;
            case TITLE:
            {
                DrawTitle();

            } break;
            case GAMEPLAY:
            {   
                DrawGame();

            } break;
            case ENDING:
            {
                // TODO: Create and Instantiate drawEnding function;
                DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
                DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);

            } break;
            default: break;
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------------
// Unload game variables
//------------------------------------------------------------------------------------
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
    UnloadTexture(player.playerSprite);
    UnloadTexture(playerLife[0].life);
    UnloadTexture(playerLife[1].life);
    UnloadTexture(playerLife[2].life);
    UnloadTexture(backgroundLogo);
    UnloadTexture(backgroundTitle);
    UnloadTexture(button);
    UnloadMusicStream(backgroundMusic.song);
    UnloadMusicStream(backgroundMenu.song);
    UnloadSound(gameOverSound.sound);
    UnloadSound(damageTaken.sound);
    UnloadSound(fxButton);
}

