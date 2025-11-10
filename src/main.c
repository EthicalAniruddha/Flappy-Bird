#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define MAX_PIPES 5
#define GRAVITY 1000.0f
#define JUMP_FORCE -375.0f

Vector2 centerText(const char *text, int fontSize, int screenWidth, int screenHeight) {
    Vector2 position;
    position.x = screenWidth/2 - MeasureText(text, fontSize)/2;
    position.y = screenHeight/2 - fontSize/2;
    return position;
}

int main(void) {

    const int screenHeight = 720, screenWidth = 1400;

    // Create a window
    InitWindow(screenWidth,screenHeight,"FLAPPY-BIRD");

    // Load textures    
    Texture2D birdTexture = LoadTexture("assets/sprites/bird.png");
    Texture2D pipeTexture = LoadTexture("assets/sprites/pipe.png");

    Texture2D background = LoadTexture("assets/parallax/moon_back.png");
    Texture2D midground = LoadTexture("assets/parallax/moon_mid.png");
    Texture2D foreground = LoadTexture("assets/parallax/moon_front.png");

    // Load sounds
    InitAudioDevice();

    Music bgMusic = LoadMusicStream("assets/sound/bgSound/bg.mp3");
    bgMusic.looping = true;
    
    SetMusicVolume(bgMusic, 0.05f);
    PlayMusicStream(bgMusic);

    // jump sounds
    Sound jumpSounds[6];
    jumpSounds[0] = LoadSound("assets/sound/jumpSounds/bamba.wav");
    jumpSounds[1] = LoadSound("assets/sound/jumpSounds/bumba.wav");
    jumpSounds[2] = LoadSound("assets/sound/jumpSounds/dumba.wav");
    jumpSounds[3] = LoadSound("assets/sound/jumpSounds/humba.wav");
    jumpSounds[4] = LoadSound("assets/sound/jumpSounds/kamba.wav");
    jumpSounds[5] = LoadSound("assets/sound/jumpSounds/ramba.wav");

    for(int i = 0; i < 5; ++i) {
        SetSoundVolume(jumpSounds[i], 1.0f);
    }

    float scrollingBack= 0.0f;
    float scrollingMid = 0.0f;
    float scrollingFore = 0.0f;

    SetTargetFPS(60);

    // game started
    bool gameStarted = false;
    bool gameOver = false;

    // for the start menu
    Vector2 txtPos = centerText("Press ENTER to START", 40, screenWidth, screenHeight);

    // For our bird
    Vector2 bird = {screenWidth/2.0, screenHeight/2.0-100};
    float birdVel = 0.0f; // Y velocity of bird

    // For pipes
    float pipeWidth = 120.0f;
    float gapSize = 250.0f;
    float pipeSpeed = 200.0f;
    float pipeSpacing = 320.0f;

    float pipeX[MAX_PIPES]; // Pipe X positions
    float gapY[MAX_PIPES]; // Gap Y positions

    // pipe initialization
    for(int i = 0; i < MAX_PIPES; ++i) {
        pipeX[i] = screenWidth+i*pipeSpacing;
        gapY[i] = GetRandomValue(gapSize, screenHeight-gapSize);
    }

    // Scores
    int score = 0;
    bool scored[MAX_PIPES] = {false};

    // Color for bird
    float colorTimer = 0.0f;

    // Calulate the scale to fit 
    float bgScale = (float)screenHeight / background.height;
    float mgScale = (float)screenHeight / midground.height;
    float fgScale = (float)screenHeight / foreground.height;

    // Game loop
    while(!WindowShouldClose()) {

        UpdateMusicStream(bgMusic);

        // run if gameStarted == true
        if(gameStarted && !gameOver) {
            // for bird gavity
            birdVel += GRAVITY*GetFrameTime();
            bird.y += birdVel*GetFrameTime();

            // for pipe
            for(int i = 0; i < MAX_PIPES; ++i) {
                pipeX[i] -= pipeSpeed*GetFrameTime();

                if(pipeX[i] + pipeWidth <= 0) {
                    pipeX[i] = screenWidth;
                    gapY[i] = GetRandomValue(gapSize/2 + 30, screenHeight-gapSize/2 - 30);
                    scored[i] = false;
                }
            }

            // Check collision within the pipe
            float shrink = 0.3f;
            Rectangle birdColRect = {
                bird.x - (birdTexture.width*shrink)/2, 
                bird.y - (birdTexture.height*shrink)/2, 
                birdTexture.width*shrink, 
                birdTexture.height*shrink
            };

            for(int i = 0; i < MAX_PIPES; ++i) {
                Rectangle topPipe = {pipeX[i], 0, pipeWidth, gapY[i] - gapSize/2};
                Rectangle bottomPipe = {pipeX[i], gapY[i] + gapSize/2, pipeWidth, screenHeight - (gapY[i] + gapSize/2)};

                if(CheckCollisionRecs(birdColRect,topPipe) || CheckCollisionRecs(birdColRect, bottomPipe)) gameOver = true;
            }

            // score logic
            for(int i = 0; i < MAX_PIPES; ++i) {
                if(bird.x > pipeX[i] + pipeWidth && !scored[i]) {
                    ++score;
                    scored[i] = true;
                }
            }

            // collision of top and bottom of our screen
            if(bird.y + birdTexture.height/2 >= screenHeight || bird.y - birdTexture.height/2 <= 0) gameOver = true;

            // parallax?
            float dt = GetFrameTime();
            scrollingBack -= 20.0f*dt;
            scrollingMid -= 100.0f*dt;
            scrollingFore -= 200.0f*dt;

            if(scrollingBack <= -background.width*bgScale) scrollingBack = 0;
            if(scrollingMid <= -midground.width*mgScale) scrollingMid = 0;
            if(scrollingFore <= -foreground.width*fgScale) scrollingFore = 0;
        }

        // bird alien colour
        colorTimer += GetFrameTime() * 5.0f;
        Color birdAlien = {
            (unsigned char) (127 + 127*sinf(colorTimer)),
            (unsigned char) (127 + 127*sinf(colorTimer + 2.0f)),
            (unsigned char) (127 + 127*sinf(colorTimer + 4.0f)),
            255
        };
        
        // space -> jump
        if(IsKeyPressed(KEY_SPACE) && gameStarted && !gameOver) {
            birdVel = JUMP_FORCE;

            // pick random index
            int randIdx = GetRandomValue(0, 5);
            PlaySound(jumpSounds[randIdx]);
        }

        // enter -> game start
        if(IsKeyPressed(KEY_ENTER)) gameStarted = true;

        // r -> restart
        if(gameOver && IsKeyPressed(KEY_R)) {
            gameOver = false;
            gameStarted = false;
            bird.y =  screenHeight/2-100;
            birdVel = 0;
            score = 0;
            scrollingBack = 0.0f;
            scrollingMid = 0.0f;
            scrollingFore = 0.0f;

            // reset pipes
            for(int i = 0; i < MAX_PIPES; ++i) {
                pipeX[i] = screenWidth+i*pipeSpacing;
                gapY[i] = GetRandomValue(gapSize, screenHeight-gapSize);
                scored[i] = false;
            }
        }

        /* Draw */
        BeginDrawing();
            ClearBackground(GetColor(0x052c46ff));
            // Background
            DrawTextureEx(background, (Vector2){scrollingBack, 0}, 0.0f, bgScale, WHITE);
            DrawTextureEx(background, (Vector2){scrollingBack + background.width*bgScale, 0}, 0.0f, bgScale, WHITE);

            // Midground  
            DrawTextureEx(midground, (Vector2){scrollingMid, 0}, 0.0f, mgScale, WHITE);
            DrawTextureEx(midground, (Vector2){scrollingMid + midground.width*mgScale, 0}, 0.0f, mgScale, WHITE);

            // Foreground
            DrawTextureEx(foreground, (Vector2){scrollingFore, 0}, 0.0f, fgScale, WHITE);
            DrawTextureEx(foreground, (Vector2){scrollingFore + foreground.width*fgScale, 0}, 0.0f, fgScale, WHITE);
            DrawTexture(birdTexture, bird.x - birdTexture.width/2, bird.y - birdTexture.height/2, birdAlien);
            if(gameStarted) {
                for(int i = 0; i < MAX_PIPES; ++i) {
                    // Top pipe
                    DrawTexturePro(
                        pipeTexture,
                        (Rectangle){0,0,pipeTexture.width, pipeTexture.height},
                        (Rectangle){pipeX[i], 0, pipeWidth, gapY[i] - gapSize/2},
                        (Vector2){0,0},
                        0,
                        WHITE
                    );
                    // Bottom pipe
                    DrawTexturePro(
                        pipeTexture,
                        (Rectangle){0,0,pipeTexture.width, pipeTexture.height},
                        (Rectangle){pipeX[i], gapY[i] + gapSize/2, pipeWidth, screenHeight - (gapY[i] + gapSize/2)},
                        (Vector2){0,0},
                        0,
                        WHITE
                    );
                }

                if(gameOver) {
                    Vector2 gameOverTxtPos = centerText("GAME OVER! Press R to Restart", 40, screenWidth, screenHeight);
                    DrawText("GAME OVER! Press R to Restart", gameOverTxtPos.x, gameOverTxtPos.y, 40, RED);
                }
            } else {
                DrawText("Press ENTER to START", txtPos.x, txtPos.y, 40, RAYWHITE);
            }
            if(gameStarted) {
                char scoreTxt[20];
                sprintf(scoreTxt, "%d", score);
                int scoreWidth = MeasureText(scoreTxt, 60);
                DrawText(scoreTxt, screenWidth/2-scoreWidth/2, 50, 60, birdAlien);
            }
        EndDrawing();
    }

    UnloadMusicStream(bgMusic);
    for(int i = 0; i < 6; ++i) UnloadSound(jumpSounds[i]);
    CloseAudioDevice();
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadTexture(birdTexture);
    UnloadTexture(pipeTexture);
    CloseWindow(); // close window

    return 0;
}