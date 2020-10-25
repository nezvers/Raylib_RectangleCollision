/*******************************************************************************************
*
*   raylib - sample game: platformer
*
*   Sample game developed by Agnis "NeZvers" Aldiòð
*
*   This game has been created using raylib v3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some Defines
//----------------------------------------------------------------------------------
#define FPS 60

//tile collision types
#define EMPTY -1
#define BLOCK 0 // start from zero, slopes can be added

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef struct Input {
    bool right;
    bool left;
    bool up;
    bool down;
    bool jump;
} Input;

typedef struct Entity {
    int width;
    int height;

    Vector2 position;
    float direction;
    float maxSpd;
    float acc;
    float dcc;
    float gravity;
    float jumpImpulse;
    float jumpRelease;
    Vector2 velocity;
    //carry stored subpixel values
    float hsp;
    float vsp;

    bool isGrounded;
    bool isJumping;
    
    Input *control;
} Entity;

typedef struct Coin{
    Vector2 position;
    bool visible;
} Coin;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
const int TILE_SIZE = 16;
const int TILE_SHIFT = 4;   //used in bitshift  | bit of TILE_SIZE
const int TILE_ROUND = 15;  //used in bitwise operation | TILE_SIZE - 1

#define TILE_MAP_WIDTH 20
#define TILE_MAP_HEIGHT 12

float screenScale;
int screenWidth;
static int screenHeight;

static float delta;
static bool win = false;
static int score = 0;

static int tiles [TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
static Entity player = { 0 };
static Input input = {false, false, false, false, false};
static Camera2D camera = {0};

#define coinCount 10
static Coin coins[coinCount] = {
    {(Vector2){1*16+6,7*16+6}, true},
    {(Vector2){3*16+6,5*16+6}, true},
    {(Vector2){4*16+6,5*16+6}, true},
    {(Vector2){5*16+6,5*16+6}, true},
    {(Vector2){8*16+6,3*16+6}, true},
    {(Vector2){9*16+6,3*16+6}, true},
    {(Vector2){10*16+6,3*16+6}, true},
    {(Vector2){13*16+6,4*16+6}, true},
    {(Vector2){14*16+6,4*16+6}, true},
    {(Vector2){15*16+6,4*16+6}, true},
};

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void GameInit(void);         // Initialize game
static void GameUpdate(void);       // Update game (one frame)
static void GameDraw(void);         // Draw game (one frame)
static void GameUnload(void);       // Unload game

//------------------------------------------------------------------------------------
// Movement Functions Declaration (local)
//------------------------------------------------------------------------------------
static void EntityMoveUpdate(Entity *intance);
static void GetDirection(Entity *instance);
static void GroundCheck(Entity *instance);
static void MoveCalc(Entity *instance);
static void GravityCalc(Entity *instance);
static void CollisionCheck(Entity *instance);

//------------------------------------------------------------------------------------
// Tile Functions Declaration (local)
//------------------------------------------------------------------------------------
static int MapGetTile(int x, int y);
static int MapGetTileWorld(int x, int y);
static int TileHeight(int x, int y, int tile);

static void MapInit(void);
static void PlayerInit(void);
static void InputUpdate(void);
static void PlayerUpdate(void);
static void PlayerDraw(void);
static void MapDraw(void);
static void CoinInit(void);
static void CoinDraw(void);
static void CoinUpdate(void);

//------------------------------------------------------------------------------------
// Utility Functions Declaration (local)
//------------------------------------------------------------------------------------
static int sign(float x);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void){
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    screenScale = 2.0;
    screenWidth = TILE_SIZE * TILE_MAP_WIDTH * (int)screenScale;
    screenHeight = TILE_SIZE * TILE_MAP_HEIGHT * (int)screenScale;
    
    SetConfigFlags(FLAG_VSYNC_HINT);    //movement jitters without it
    InitWindow(screenWidth, screenHeight, "sample game: platformer");
    GameInit();


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, FPS, 1);
#else
    SetTargetFPS(FPS);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        GameUpdate();
        GameDraw();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    GameUnload();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------
// Initialize game variables
void GameInit(void){
    win = false;
    score = 0;
    camera.offset = (Vector2){0.0, 0.0};
    camera.target = (Vector2){0.0, 0.0};
    camera.rotation = 0.0f;
    camera.zoom = screenScale;
    MapInit();
    PlayerInit();
    CoinInit();
}

// Update game (one frame)
void GameUpdate(void){
    delta = GetFrameTime();
    
    PlayerUpdate();
    CoinUpdate();
    
    if(win){
        if (IsKeyPressed(KEY_ENTER)){
            GameInit();
        }
    }
}

// Draw game (one frame)
void GameDraw(void){
    BeginDrawing();
    BeginMode2D(camera);
        ClearBackground(RAYWHITE);

        // Draw game
        MapDraw();
        CoinDraw();
        PlayerDraw();
        
        
        EndMode2D();
        DrawText(TextFormat("SCORE: %i", score), GetScreenWidth()/2 - MeasureText(TextFormat("SCORE: %i", score), 40)/2, GetScreenHeight()/5 - 50, 40, BLACK);

        if(win){
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);
        }
    EndDrawing();
}

// Unload game variables
void GameUnload(void){
    
}

void MapInit(void){
    //Set tiles - borders
    for (int y = 0; y < TILE_MAP_HEIGHT; y++){
        for (int x = 0; x < TILE_MAP_WIDTH; x++){
            // Solid tiles
            if (y == 0 || x == 0 || y == TILE_MAP_HEIGHT-1 || x == TILE_MAP_WIDTH-1){
                tiles[x+y * TILE_MAP_WIDTH] = BLOCK;
            }
            //Empty tiles
            else{
                tiles[x+y * TILE_MAP_WIDTH] = EMPTY;
            }
        }
    }
    
    //manual cell population for platforms
    tiles[3+8 * TILE_MAP_WIDTH] = BLOCK;
    tiles[4+8 * TILE_MAP_WIDTH] = BLOCK;
    tiles[5+8 * TILE_MAP_WIDTH] = BLOCK;
    
    tiles[8+6 * TILE_MAP_WIDTH] = BLOCK;
    tiles[9+6 * TILE_MAP_WIDTH] = BLOCK;
    tiles[10+6 * TILE_MAP_WIDTH] = BLOCK;
    
    tiles[13+7 * TILE_MAP_WIDTH] = BLOCK;
    tiles[14+7 * TILE_MAP_WIDTH] = BLOCK;
    tiles[15+7 * TILE_MAP_WIDTH] = BLOCK;
    
    tiles[1+10 * TILE_MAP_WIDTH] = BLOCK;
}

void MapDraw(void){
    //parse through tile map and draw rectangles
    for (int y = 0; y < TILE_MAP_HEIGHT; y++){
        for (int x = 0; x < TILE_MAP_WIDTH; x++){
            // Draw tiles
            if ( tiles[x+y*TILE_MAP_WIDTH] > EMPTY){
                DrawRectangle(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);
            }
        }
    }
}

int MapGetTileWorld(int x, int y){
    //Returns tile ID using world position
    x /= TILE_SIZE;
    y /= TILE_SIZE;
    if (x < 0 || x > TILE_MAP_WIDTH || y < 0 || y > TILE_MAP_HEIGHT) {
        return EMPTY;
    }
    return tiles[x+y*TILE_MAP_WIDTH];
}

int MapGetTile(int x, int y){
    //Returns tile ID using tile position withing tile map.
    if (x < 0 || x > TILE_MAP_WIDTH || y < 0 || y > TILE_MAP_HEIGHT) {
        return EMPTY;
    }
    return tiles[x+y*TILE_MAP_WIDTH];
}

int TileHeight(int x, int y, int tile){
    //returns one pixel above solid. Extendable for slopes.
    switch(tile){
        case EMPTY:
            break;
        case BLOCK:
            y = (y & ~TILE_ROUND) -1;
            break;
    }
    return y;
}

void InputUpdate(void){
    input.right = (float)( IsKeyDown('D') || IsKeyDown(KEY_RIGHT) );
    input.left = (float)( IsKeyDown('A') || IsKeyDown(KEY_LEFT) );
    input.up = (float)( IsKeyDown('W') || IsKeyDown(KEY_UP) );
    input.down = (float)( IsKeyDown('S') || IsKeyDown(KEY_DOWN) );

    //For jumping button needs to be toggled
    if (IsKeyPressed(KEY_SPACE)){
        input.jump = true;
    }
    else if (IsKeyReleased(KEY_SPACE)){
        input.jump = false;
    }
}

void PlayerInit(void){
    player.position.x = (float)(TILE_SIZE * TILE_MAP_WIDTH) * 0.5;
    player.position.y = TILE_MAP_HEIGHT*TILE_SIZE - 16.0 -1;
    player.direction = 1.0;
    //values taken from NES Mario but it's not how Mario muvement is done
    player.maxSpd = 0x1900 *60;
    player.acc = 0x01e4 *60 *60;
    player.dcc = 0x01d0 *60 *60;
    player.gravity = 0x05d0 * 60 * 60;
    player.jumpImpulse = -0x6900 *60;
    player.jumpRelease = player.jumpImpulse * 0.2;
    player.velocity = (Vector2){0.0, 0.0};
    player.hsp = 0;
    player.vsp = 0;

    player.width = 8;
    player.height = 16;

    player.isGrounded   = false;
    player.isJumping    = false;
    
    player.control = &input;
}

void PlayerDraw(void){
    DrawRectangle(player.position.x - player.width*0.5, player.position.y-player.height +1, player.width, player.height, RED);
}

void PlayerUpdate(void){
    InputUpdate();
    EntityMoveUpdate(&player);
}


void CoinInit(void){
    for(int i=0; i<coinCount; i++){
        coins[i].visible = true;
    }
}

void CoinDraw(void){
    for(int i=0; i<coinCount; i++){
        if(coins[i].visible){
            DrawRectangle((int)coins[i].position.x, (int)coins[i].position.y, 4.0, 4.0, GOLD);
        }
    }
}

void CoinUpdate(void){
    Rectangle playerRect = (Rectangle){player.position.x - player.width*0.5, player.position.y-player.height +1, player.width, player.height};
    for(int i=0; i<coinCount; i++){
        if(coins[i].visible){
            Rectangle coinRect = (Rectangle){coins[i].position.x, coins[i].position.y, 4.0, 4.0};
            if(CheckCollisionRecs(playerRect, coinRect)){
                coins[i].visible = false;
                score += 1;
            }
        }
    }
    win = score == coinCount;
}

//------------------------------------------------
// Physics functions
//------------------------------------------------
void EntityMoveUpdate(Entity *instance){
    GroundCheck(instance);
    GetDirection(instance);
    MoveCalc(instance);
    GravityCalc(instance);
    CollisionCheck(instance);
    
    float xVel = instance->velocity.x*delta + instance->hsp;
    float xsp = ((int)(abs(xVel)) >> 12) * sign(xVel);
    instance->hsp = instance->velocity.x*delta - ((abs(xsp) << 12)*sign(xsp));
    
    float yVel = instance->velocity.y*delta + instance->vsp;
    float ysp = ((int)(abs(yVel)) >> 12) * sign(yVel);
    instance->vsp = instance->velocity.y*delta - ((abs(ysp) << 12)*sign(ysp));
    
    
    instance->position.x += xsp;
    
    instance->position.y += ysp;
    
    //Prototyping Safety net
    instance->position.x = Clamp(instance->position.x, 0.0, TILE_MAP_WIDTH*TILE_SIZE);
    instance->position.y = Clamp(instance->position.y, 0.0, TILE_MAP_HEIGHT*TILE_SIZE);
}

void GetDirection(Entity *instance){
    instance->direction = instance->control->right - instance->control->left;
}

void GroundCheck(Entity *instance){
    int x = instance->position.x;
    int y = instance->position.y +1;
    instance->isGrounded = false;
    
    int c = MapGetTile(x>>TILE_SHIFT, y>>TILE_SHIFT);
    if(c!=EMPTY){
        int h = TileHeight(x, y, c);
        instance->isGrounded = (y >= h);
    }
    if(!instance->isGrounded){
        int xl = (x - instance->width/2);
        int l = MapGetTile(xl>>TILE_SHIFT, y>>TILE_SHIFT);
        if(l!=EMPTY){
            int h = TileHeight(xl, y, l);
            instance->isGrounded = (y >= h);
        }
        if(!instance->isGrounded){
            int xr = (x + instance->width/2-1);
            int r = MapGetTile(xr>>TILE_SHIFT, y>>TILE_SHIFT);
            if(r!=EMPTY){
                int h = TileHeight(xr, y, r);
                instance->isGrounded = (y >= h);
            }
        }
    }
}

void MoveCalc(Entity *instance){
    if (abs(instance->direction) > 0.01){
        instance->velocity.x += instance->direction * instance->acc * delta;
        instance->velocity.x = Clamp(instance->velocity.x, -instance->maxSpd, instance->maxSpd);
    }else{
        float hsp = instance->velocity.x;
        if(abs(0-hsp) < instance->dcc *delta){
            instance->velocity.x = 0;
        }
        else if(hsp > 0){
            instance->velocity.x -= instance->dcc *delta;
        }
        else{
            instance->velocity.x += instance->dcc *delta;
        }
    }
}

void Jump(Entity *instance){
    instance->velocity.y = instance->jumpImpulse;
    instance->isJumping = true;
    instance->isGrounded = false;
}

void GravityCalc(Entity *instance){
    if (instance->isGrounded){
        if (instance->isJumping){
            instance->isJumping = false;
            instance->control->jump = false;             //cancel input button
        }
        else if (!instance->isJumping && instance->control->jump){
            Jump(instance);
        }
    }
    else{
        if(instance->isJumping){
            if(!instance->control->jump){
                instance->isJumping = false;
                if (instance->velocity.y < instance->jumpRelease){
                    instance->velocity.y = instance->jumpRelease;
                }
            }
        }
    }
    instance->velocity.y += instance->gravity * delta;
    if(instance->velocity.y > -instance->jumpImpulse){
        instance->velocity.y = -instance->jumpImpulse;
    }
}

static void CollisionHorizontalBlocks(Entity *instance);
static void CollisionVerticalBlocks(Entity *instance);
void CollisionCheck(Entity *instance){
    CollisionHorizontalBlocks(instance);
    CollisionVerticalBlocks(instance);
}

void CollisionHorizontalBlocks(Entity *instance){
    //get horizontal speed in pixels
    float xVel = instance->velocity.x*delta + instance->hsp;
    int xsp = ((int)(abs(xVel)) >> 12) * sign(xVel);
    
    //get bounding box side offset
    int side;
    if (xsp > 0){
        side = instance->width/2-1;
    }
    else if (xsp < 0){
        side = -instance->width/2;
    }
    else{
        return;
    }
    int x   = instance->position.x;
    int y   = instance->position.y;
    int mid = -instance->height/2;
    int top = -instance->height +1;
    
    //3 point check
    int b = MapGetTile((x+side+xsp)>>TILE_SHIFT, y >>TILE_SHIFT)        > EMPTY;
    int m = MapGetTile((x+side+xsp)>>TILE_SHIFT, (y+mid) >>TILE_SHIFT)  > EMPTY;
    int t = MapGetTile((x+side+xsp)>>TILE_SHIFT, (y+top) >>TILE_SHIFT)  > EMPTY;
    //if using slopes it's better to disable b & m if (x,y) is in the slope tile
    if(b || m || t){
        if(xsp > 0){
            x = ((x+side+xsp) & ~TILE_ROUND) -1 -side;
        }
        else{
            x = ((x+side+xsp) & ~TILE_ROUND) +TILE_SIZE -side;
        }
        instance->position.x = x;
        instance->velocity.x = 0.0;
        instance->hsp = 0.0;
    }
}

void CollisionVerticalBlocks(Entity *instance){
    //get vertical speed in pixels
    float yVel = instance->velocity.y*delta + instance->vsp;
    int ysp = ((int)(abs(yVel)) >> 12) * sign(yVel);
    
    //get bounding box side offset
    int side;
    if (ysp > 0){
        side = 0;
    }
    else if (ysp < 0){
        side = -instance->height+1;
    }
    else{
        return;
    }
    int x = instance->position.x;
    int y = instance->position.y;
    int xl = -instance->width/2;
    int xr = instance->width/2-1;
    
    int c = MapGetTile(x >>TILE_SHIFT, (y+side+ysp) >>TILE_SHIFT) > EMPTY;
    int l = MapGetTile((x+xl) >>TILE_SHIFT, (y+side+ysp) >>TILE_SHIFT) > EMPTY;
    int r = MapGetTile((x+xr) >>TILE_SHIFT, (y+side+ysp) >>TILE_SHIFT) > EMPTY;
    if(c || l || r){
        if(ysp > 0){
            y = ((y+side+ysp) & ~TILE_ROUND) -1 -side;
        }
        else{
            y = ((y+side+ysp) & ~TILE_ROUND) +TILE_SIZE -side;
        }
        instance->position.y = y;
        instance->velocity.y = 0.0;
        instance->vsp = 0.0;
    }
}

int sign(float x){
    if(x < 0){
        return -1;
    }
    else if(x < 0.0001){
        return 0;
    }
    else{
        return 1;
    }
}






