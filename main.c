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
typedef enum { FIRST = 0, SECOND, THIRD } EnemyWave;

typedef struct Player{
    Rectangle playerDest;
    Rectangle playerSrc;
    Vector2 origin;
    Vector2 speed;
    Color color;
    Texture2D playerSprite;
} Player;

typedef struct Enemy{
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Enemy;

typedef struct Shoot{
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Shoot;

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
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Shoot shoot[NUM_SHOOTS] = { 0 };
static EnemyWave wave = { 0 };

static int shootRate = 0;
static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;

// Variables for moving animation 
bool moving;
int direction, frameCount, playerFrame;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "classic game: space invaders");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 144, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

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
    player.speed.x = 4;
    player.speed.y = 4;
    player.color = BLUE;
    player.playerSprite = LoadTexture ("Assets/NinjaAdventure/Actor/Characters/GreenNinja/SeparateAnim/walk.png");

    // Initialize enemies
    for (int i = 0; i < NUM_MAX_ENEMIES; i++)
    {
        enemy[i].rec.width = 10;
        enemy[i].rec.height = 10;
        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
        enemy[i].speed.x = 5;
        enemy[i].speed.y = 5;
        enemy[i].active = true;
        enemy[i].color = GRAY;
    }

    // Initialize shoots
    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        shoot[i].rec.x = player.playerDest.x;
        shoot[i].rec.y = player.playerDest.y + player.playerDest.height/2;
        shoot[i].rec.width = 10;
        shoot[i].rec.height = 5;
        shoot[i].speed.x = 7;
        shoot[i].speed.y = 0;
        shoot[i].active = false;
        shoot[i].color = MAROON;
    }
}

// Update game (one frame)
void UpdateGame(void)
{

    if (!gameOver)
    {
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
                if (CheckCollisionRecs(player.playerDest, enemy[i].rec)) gameOver = true;
            }

            // Enemy behaviour
            for (int i = 0; i < activeEnemies; i++)
            {
                if (enemy[i].active)
                {
                    enemy[i].rec.x -= enemy[i].speed.x;

                    if (enemy[i].rec.x < 0)
                    {
                        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
                    }
                }
            }

            // Wall behaviour
            if (player.playerDest.x <= 0) player.playerDest.x = 0;
            if (player.playerDest.x + player.playerDest.width >= screenWidth) player.playerDest.x = screenWidth - player.playerDest.width;
            if (player.playerDest.y <= 0) player.playerDest.y = 0;
            if (player.playerDest.y + player.playerDest.height >= screenHeight) player.playerDest.y = screenHeight - player.playerDest.height;

            // Shoot initialization
            if (IsKeyDown(KEY_SPACE))
            {
                shootRate += 5;

                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (!shoot[i].active && shootRate%20 == 0)
                    {
                        shoot[i].rec.x = player.playerDest.x;
                        shoot[i].rec.y = player.playerDest.y + player.playerDest.height/2;
                        shoot[i].active = true;
                        break;
                    }
                }
            }

            // Shoot logic
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active)
                {
                    // Movement
                    shoot[i].rec.x += shoot[i].speed.x;

                    // Collision with enemy
                    for (int j = 0; j < activeEnemies; j++)
                    {
                        if (enemy[j].active)
                        {
                            if (CheckCollisionRecs(shoot[i].rec, enemy[j].rec))
                            {
                                shoot[i].active = false;
                                enemy[j].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                                enemy[j].rec.y = GetRandomValue(0, screenHeight - enemy[j].rec.height);
                                shootRate = 0;
                                enemiesKill++;
                                score += 100;
                            }

                            if (shoot[i].rec.x + shoot[i].rec.width >= screenWidth)
                            {
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

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            // Rectangle for tracking character position (testes!)
            //DrawRectangle(player.playerDest.x, player.playerDest.y, player.playerDest.width, player.playerDest.height, BLUE);
            DrawTexturePro(player.playerSprite, player.playerSrc, player.playerDest, player.origin, 0, WHITE);

            if (wave == FIRST) DrawText("FIRST WAVE", screenWidth/2 - MeasureText("FIRST WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
            else if (wave == SECOND) DrawText("SECOND WAVE", screenWidth/2 - MeasureText("SECOND WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));
            else if (wave == THIRD) DrawText("THIRD WAVE", screenWidth/2 - MeasureText("THIRD WAVE", 40)/2, screenHeight/2 - 40, 40, Fade(BLACK, alpha));

            for (int i = 0; i < activeEnemies; i++)
            {
                if (enemy[i].active) DrawRectangleRec(enemy[i].rec, enemy[i].color);
            }

            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active) DrawRectangleRec(shoot[i].rec, shoot[i].color);
            }

            DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);

            if (victory) DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 40)/2, screenHeight/2 - 40, 40, BLACK);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
    UnloadTexture(player.playerSprite);

}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}
