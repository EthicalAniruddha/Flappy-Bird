#include "raymob.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define GRAVITY 1000.0f
#define JUMP_FORCE (-385.0f)
#define SOUND_INSTANCES 4

// for centering text
Vector2 centerText(const char *text, int fontSize, int screenWidth, int screenHeight) {
    Vector2 position;
    position.x = screenWidth/2 - MeasureText(text, fontSize)/2;
    position.y = screenHeight/2 - fontSize/2;
    return position;
}

int main(void) {

    // Create window duh
    InitWindow(0,0,"FLAPPY-BIRD");
    const int screenHeight = GetScreenHeight(), screenWidth = GetScreenWidth();

    // Load textures tbh
    Texture2D birdTexture = LoadTexture("sprites/bird.png"); // funny pic
    Texture2D pipeTexture = LoadTexture("sprites/pipe.png"); // funny pic

    Texture2D background = LoadTexture("parallax/moon_back.png");
    Texture2D midground = LoadTexture("parallax/moon_mid.png");
    Texture2D foreground = LoadTexture("parallax/moon_front.png");

    Texture2D gameOverImg = LoadTexture("sprites/gameOver.png");

    // Load sounds
    InitAudioDevice();

    Music bgMusic = LoadMusicStream("sound/bgSound/bg.mp3");
    bgMusic.looping = true;

    SetMusicVolume(bgMusic, 0.1f);
    PlayMusicStream(bgMusic);

    // jump sounds
    Sound jumpSounds[6][SOUND_INSTANCES];
    for(int i = 0; i < 6; ++i) {
        const char *soundFilePath[6] = {
                "sound/jumpSounds/bamba.wav",
                "sound/jumpSounds/bumba.wav",
                "sound/jumpSounds/dumba.wav",
                "sound/jumpSounds/humba.wav",
                "sound/jumpSounds/kamba.wav",
                "sound/jumpSounds/ramba.wav"
        };

        for(int j = 0; j < SOUND_INSTANCES; ++j) {
            jumpSounds[i][j] = LoadSound(soundFilePath[i]);
            SetSoundVolume(jumpSounds[i][j], 1.0f);
        }
    }

    int currentSound = 0;

    // Game over sound
    Sound gameOverSound = LoadSound("sound/gameOver/gameOver.wav");
    SetSoundVolume(gameOverSound, 1.0f);

    float scrollingBack= 0.0f;
    float scrollingMid = 0.0f;
    float scrollingFore = 0.0f;

    SetTargetFPS(60);

    // game started
    bool gameStarted = false;
    bool gameOver = false;
    bool gameOverSoundPlayed = false;

    // for the start menu
    Vector2 txtPos = centerText("Tap to START", 40, screenWidth, screenHeight);

    // Bird size
    float birdScale = (float)screenWidth / 1080.0f;
    if(birdScale < 0.5f) birdScale = 0.5f; // minimum size
    if(birdScale > 1.2f) birdScale = 1.2f; // maximum size

    // Bird position
    Vector2 bird = {(float)screenWidth * 0.25f, (float)screenHeight * 0.5f};
    float birdVel = 0.0f;

    // Pipe dimensions - scale with screen width
    float pipeWidth = (float)screenWidth * 0.30f;  // 30% of screen width
    if(pipeWidth < 100.0f) pipeWidth = 100.0f;     // Minimum width
    if(pipeWidth > 200.0f) pipeWidth = 200.0f;     // Maximum width

    // Gap size - scale with screen height
    float gapSize = (float)screenHeight * 0.35f;   // 35% of screen height

    // Pipe spacing - based on screen width
    float pipeSpacing = (float)screenWidth * 0.20f; // 20% of screen width apart

    // Pipe speed - proportional to screen
    float pipeSpeed = (float)screenWidth * 0.15f;  // Adjusted speed

    // Calculate how many pipes we need to cover screen + some extra off-screen
    int maxPipes = (int)((screenWidth * 2.0f) / pipeSpacing) + 2;
    if(maxPipes < 3) maxPipes = 3; // Minimum 3 pipes
    if(maxPipes > 8) maxPipes = 8; // Maximum 8 pipes for performance

    // Dynamically allocate arrays based on calculated pipe count
    float *pipeX = (float*)malloc(maxPipes * sizeof(float));
    float *gapY = (float*)malloc(maxPipes * sizeof(float));
    bool *scored = (bool*)malloc(maxPipes * sizeof(bool));

    // Initialize pipes with screen-based positioning
    for(int i = 0; i < maxPipes; ++i) {
        pipeX[i] = screenWidth + i * pipeSpacing;
        // Gap centers should avoid top and bottom edges
        float minGapY = gapSize * 0.7f;
        float maxGapY = screenHeight - gapSize * 0.7f;
        gapY[i] = GetRandomValue((int)minGapY, (int)maxGapY);
        scored[i] = false;
    }

    // Score
    int score = 0;

    // Color for bird
    float colorTimer = 0.0f;

    // Calculate the scale to fit
    float bgScale = (float)screenHeight / background.height;
    float mgScale = (float)screenHeight / midground.height;
    float fgScale = (float)screenHeight / foreground.height;

    // Restart button - scale with screen
    float btnWidth = screenWidth * 0.4f;
    float btnHeight = screenHeight * 0.08f;
    Rectangle restartBtn = {
            (float)screenWidth/2 - btnWidth/2,
            (float)screenHeight * 0.65f,
            btnWidth,
            btnHeight
    };

    // Game loop
    while(!WindowShouldClose()) {

        UpdateMusicStream(bgMusic);
        Vector2 touchPos = GetTouchPosition(0);

        if(gameStarted && !gameOver) {
            birdVel += GRAVITY*GetFrameTime();
            bird.y += birdVel*GetFrameTime();

            // Update pipes
            for(int i = 0; i < maxPipes; ++i) {
                pipeX[i] -= pipeSpeed*GetFrameTime();

                // When pipe goes off screen, reset it
                if(pipeX[i] + pipeWidth <= 0) {
                    // Find the rightmost pipe
                    float maxX = pipeX[0];
                    for(int j = 1; j < maxPipes; ++j) {
                        if(pipeX[j] > maxX) maxX = pipeX[j];
                    }

                    // Place this pipe after the rightmost one
                    pipeX[i] = maxX + pipeSpacing;

                    // Random gap position
                    float minGapY = gapSize * 0.7f;
                    float maxGapY = screenHeight - gapSize * 0.7f;
                    gapY[i] = GetRandomValue((int)minGapY, (int)maxGapY);
                    scored[i] = false;
                }
            }

            // Bird collision box (slightly smaller than visual for fair gameplay)
            float birdHitboxScale = 0.65f;
            float birdWidth = birdTexture.width * birdScale;
            float birdHeight = birdTexture.height * birdScale;

            Rectangle birdColRect = {
                    bird.x - (birdWidth * birdHitboxScale)/2,
                    bird.y - (birdHeight * birdHitboxScale)/2,
                    birdWidth * birdHitboxScale,
                    birdHeight * birdHitboxScale
            };

            // Check collision with pipes
            for(int i = 0; i < maxPipes; ++i) {
                Rectangle topPipe = {
                        pipeX[i],
                        0,
                        pipeWidth,
                        gapY[i] - gapSize/2
                };

                Rectangle bottomPipe = {
                        pipeX[i],
                        gapY[i] + gapSize/2,
                        pipeWidth,
                        (float)screenHeight - (gapY[i] + gapSize/2)
                };

                if(CheckCollisionRecs(birdColRect, topPipe) || CheckCollisionRecs(birdColRect, bottomPipe)) {
                    gameOver = true;
                    if(!gameOverSoundPlayed) {
                        PlaySound(gameOverSound);
                        gameOverSoundPlayed = true;
                    }
                }
            }

            // Score counting
            for(int i = 0; i < maxPipes; ++i) {
                if(bird.x > pipeX[i] + pipeWidth && !scored[i]) {
                    ++score;
                    scored[i] = true;
                }
            }

            // Check if bird hits floor or ceiling
            if(bird.y + birdHeight/2 >= screenHeight || bird.y - birdHeight/2 <= 0) {
                gameOver = true;
                if(!gameOverSoundPlayed) {
                    PlaySound(gameOverSound);
                    gameOverSoundPlayed = true;
                }
            }

            // Parallax scrolling - proportional to game speed
            float dt = GetFrameTime();
            float scrollFactor = pipeSpeed / (float)screenWidth;
            scrollingBack -= 20.0f * scrollFactor * dt;
            scrollingMid -= 100.0f * scrollFactor * dt;
            scrollingFore -= 200.0f * scrollFactor * dt;

            if(scrollingBack <= -background.width*bgScale) scrollingBack = 0;
            if(scrollingMid <= -midground.width*mgScale) scrollingMid = 0;
            if(scrollingFore <= -foreground.width*fgScale) scrollingFore = 0;
        }

        // Bird color animation
        colorTimer += GetFrameTime() * 5.0f;
        Color birdAlien = {
                (unsigned char) (127 + 127*sinf(colorTimer)),
                (unsigned char) (127 + 127*sinf(colorTimer + 2.0f)),
                (unsigned char) (127 + 127*sinf(colorTimer + 4.0f)),
                255
        };

        // Jump input
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !gameOver) {
            if (!gameStarted) gameStarted = true;
            birdVel = JUMP_FORCE;

            int randIdx = GetRandomValue(0, 5);
            PlaySound(jumpSounds[randIdx][currentSound]);
            currentSound = (currentSound+1)%SOUND_INSTANCES;
        }

        // Restart game
        if(gameOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(touchPos, restartBtn)) {

            if(IsSoundPlaying(gameOverSound)) {
                StopSound(gameOverSound);
            }

            gameOver = false;
            gameStarted = false;
            gameOverSoundPlayed = false;
            bird.y = (float)screenHeight * 0.5f;
            bird.x = (float)screenWidth * 0.25f;
            birdVel = 0;
            score = 0;
            scrollingBack = 0.0f;
            scrollingMid = 0.0f;
            scrollingFore = 0.0f;

            // Reset pipes
            for(int i = 0; i < maxPipes; ++i) {
                pipeX[i] = screenWidth + i * pipeSpacing;
                float minGapY = gapSize * 0.7f;
                float maxGapY = screenHeight - gapSize * 0.7f;
                gapY[i] = GetRandomValue((int)minGapY, (int)maxGapY);
                scored[i] = false;
            }
        }

        BeginDrawing();
        ClearBackground(GetColor(0x052c46ff));

        // Draw parallax backgrounds
        DrawTextureEx(background, (Vector2){scrollingBack, 0}, 0.0f, bgScale, WHITE);
        DrawTextureEx(background, (Vector2){scrollingBack + background.width*bgScale, 0}, 0.0f, bgScale, WHITE);

        DrawTextureEx(midground, (Vector2){scrollingMid, 0}, 0.0f, mgScale, WHITE);
        DrawTextureEx(midground, (Vector2){scrollingMid + midground.width*mgScale, 0}, 0.0f, mgScale, WHITE);

        DrawTextureEx(foreground, (Vector2){scrollingFore, 0}, 0.0f, fgScale, WHITE);
        DrawTextureEx(foreground, (Vector2){scrollingFore + foreground.width*fgScale, 0}, 0.0f, fgScale, WHITE);
        DrawTextureEx(
                birdTexture,
                (Vector2){
                        bird.x - (birdTexture.width * birdScale)/2,
                        bird.y - (birdTexture.height * birdScale)/2
                },
                0,
                birdScale,
                birdAlien
        );

        if(gameStarted) {
            for(int i = 0; i < maxPipes; ++i) {
                // Top pipe
                DrawTexturePro(
                        pipeTexture,
                        (Rectangle){0, 0, (float)pipeTexture.width, (float)pipeTexture.height},
                        (Rectangle){pipeX[i], 0, pipeWidth, gapY[i] - gapSize/2},
                        (Vector2){0, 0},
                        0,
                        WHITE
                );

                // Bottom pipe
                DrawTexturePro(
                        pipeTexture,
                        (Rectangle){0, 0, (float)pipeTexture.width, (float)pipeTexture.height},
                        (Rectangle){pipeX[i], gapY[i] + gapSize/2, pipeWidth, (float)screenHeight - (gapY[i] + gapSize/2)},
                        (Vector2){0, 0},
                        0,
                        WHITE
                );
            }

            if(gameOver) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 180});

                // Scale game over image
                float goScale = (float)screenWidth / 1080.0f * 0.8f;
                if(goScale < 0.5f) goScale = 0.5f;

                int imgW = gameOverImg.width * goScale;
                int imgH = gameOverImg.height * goScale;
                int imgX = screenWidth/2 - imgW/2;
                int imgY = screenHeight/2 - imgH - 100;

                DrawTextureEx(gameOverImg, (Vector2){imgX, imgY}, 0, goScale, birdAlien);

                // Score text - scale with screen
                int scoreFontSize = screenHeight * 0.05f;
                if(scoreFontSize < 30) scoreFontSize = 30;

                char scoreTxt[50];
                sprintf(scoreTxt, "Score: %d", score);
                int scoreWidth = MeasureText(scoreTxt, scoreFontSize);
                DrawText(scoreTxt, screenWidth/2 - scoreWidth/2, imgY + imgH + 20, scoreFontSize, birdAlien);

                // Restart button
                Color btnColor = CheckCollisionPointRec(touchPos, restartBtn) ? DARKGREEN : GREEN;
                DrawRectangleRec(restartBtn, btnColor);
                DrawRectangleLinesEx(restartBtn, 3, WHITE);

                const char *buttonTxt = "RESTART";
                int btnFontSize = btnHeight * 0.5f;
                int btnTextWidth = MeasureText(buttonTxt, btnFontSize);
                DrawText(
                        buttonTxt,
                        restartBtn.x + restartBtn.width/2 - btnTextWidth/2,
                        restartBtn.y + restartBtn.height/2 - btnFontSize/2,
                        btnFontSize,
                        WHITE
                );

            } else {
                // Score during gameplay - bigger and more visible
                char scoreTxt[20];
                sprintf(scoreTxt, "%d", score);
                int scoreFontSize = screenHeight * 0.08f;
                if(scoreFontSize < 40) scoreFontSize = 40;
                int scoreWidth = MeasureText(scoreTxt, scoreFontSize);
                DrawText(scoreTxt, screenWidth/2 - scoreWidth/2, 50, scoreFontSize, birdAlien);
            }
        } else {
            // Start text - scale with screen
            int startFontSize = screenHeight * 0.05f;
            if(startFontSize < 30) startFontSize = 30;
            Vector2 startPos = centerText("Tap to START", startFontSize, screenWidth, screenHeight);
            DrawText("Tap to START", startPos.x, startPos.y, startFontSize, RAYWHITE);
        }
        EndDrawing();
    }

    // Cleanup
    free(pipeX);
    free(gapY);
    free(scored);

    UnloadMusicStream(bgMusic);
    for(int i = 0; i < 6; ++i) {
        for(int j = 0; j < SOUND_INSTANCES; ++j) {
            UnloadSound(jumpSounds[i][j]);
        }
    }
    UnloadSound(gameOverSound);
    CloseAudioDevice();
    UnloadTexture(gameOverImg);
    UnloadTexture(background);
    UnloadTexture(midground);
    UnloadTexture(foreground);
    UnloadTexture(birdTexture);
    UnloadTexture(pipeTexture);
    CloseWindow();

    return 0;
}