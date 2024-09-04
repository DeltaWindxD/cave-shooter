#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define MAX_BULLETS 12
#define INVINCIBILITY_DURATION 1.0f
#define GUN_COOLDOWN 0.5f

int PLAYER_LIFE = 3;
bool isGameOver = false;
int SCORE = 0;

typedef enum
{
    IDLE,
    HURT
} PlayerState;

typedef struct
{
    Vector2 position;
    float velocity;
} bullet_t;

typedef struct
{
    Vector2 position;
    float velocity;
    PlayerState state;
    float invincibility_timer; // Tracks the invincibility duration
} player_t;

player_t player = {
    .position.x = 400,
    .position.y = 550,
    .velocity = 400,
    .state = IDLE,
    .invincibility_timer = 0};

bullet_t bullets[MAX_BULLETS];
int bullet_count = 0;

void shoot_bullets()
{

    bullets[bullet_count].position = (Vector2){player.position.x, player.position.y - 20}; // Slightly above the player
    bullets[bullet_count].velocity = -600.0f;                                              // Bullets go upwards

    bullet_count++;
}

void bullets_spawn()
{
    if (bullet_count < MAX_BULLETS)
    {
        float random_pos_x = (float)(rand() % 800);             // Random x-position between 0 and 800
        float random_velocity = 200.0f + (float)(rand() % 100); // Random velocity between 200 and 300

        bullets[bullet_count].position = (Vector2){random_pos_x, 5.0f}; // Start at the top of the screen
        bullets[bullet_count].velocity = random_velocity;

        bullet_count++;
    }
}

void update_bullets(float delta_time)
{
    for (int i = 0; i < bullet_count; i++)
    {
        // Movement for bullet
        bullets[i].position.y += bullets[i].velocity * delta_time;

        // Reset bullet if it goes off screen
        if (bullets[i].position.y < 0 || bullets[i].position.y > 600) // If off screen, remove bullet
        {
            bullets[i] = bullets[bullet_count - 1]; // Move the last bullet to the current position
            bullet_count--;
            i--;
            continue;
        }

        // Check collision between player bullets and falling bullets
        if (bullets[i].velocity < 0) // Only check for upward bullets
        {
            for (int j = 0; j < bullet_count; j++)
            {
                if (bullets[j].velocity > 0) // Only check for downward bullets
                {
                    float collision = CheckCollisionCircles(bullets[i].position, 5, bullets[j].position, 5);

                    if (collision)
                    {
                        // Add points for destroyed bullet
                        SCORE += 10;

                        // Remove both bullets
                        bullets[i] = bullets[bullet_count - 1];
                        bullet_count--;
                        i--;

                        bullets[j] = bullets[bullet_count - 1];
                        bullet_count--;
                        j--;

                        break;
                    }
                }
            }
        }

        // Check collision with player
        if (bullets[i].velocity > 0) // Only check collision for downward bullets
        {
            float collision_player = CheckCollisionCircles(bullets[i].position, 5, player.position, 15);

            if (player.state == IDLE && collision_player) // Player radius (15) + bullet radius (5)
            {
                player.state = HURT;
                player.invincibility_timer = INVINCIBILITY_DURATION;
                PLAYER_LIFE--;
            }

            if (PLAYER_LIFE <= 0)
            {
                isGameOver = true; // Trigger game over when lives reach 0
            }
        }
    }
}

void update_player(float delta_time)
{
    // Update invincibility timer
    if (player.state == HURT)
    {
        player.invincibility_timer -= delta_time;
        if (player.invincibility_timer <= 0)
        {
            player.state = IDLE;
            player.invincibility_timer = 0;
        }
    }

    Vector2 movement = {0.0f, 0.0f};

    // Input
    if (IsKeyDown(KEY_RIGHT))
    {
        movement.x += 1.0f;
    }

    if (IsKeyDown(KEY_LEFT))
    {
        movement.x -= 1.0f;
    }

    if (IsKeyDown(KEY_UP))
    {
        movement.y -= 1.0f;
    }

    if (IsKeyDown(KEY_DOWN))
    {
        movement.y += 1.0f;
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        shoot_bullets();
    }

    // Normalize the movement vector if moving diagonally
    if (movement.x != 0.0f && movement.y != 0.0f)
    {
        movement.x *= 0.7071f;
        movement.y *= 0.7071f;
    }

    // Apply movement
    player.position.x += movement.x * player.velocity * delta_time;
    player.position.y += movement.y * player.velocity * delta_time;
}

int main()
{
    InitWindow(800, 600, "Demo");

    srand(time(NULL));

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float delta_time = GetFrameTime();

        if (!isGameOver)
        {
            // Update the player state and position
            update_player(delta_time);

            // Spawn a new bullet every frame
            bullets_spawn();

            // Update the bullets
            update_bullets(delta_time);
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        if (isGameOver)
        {
            DrawText("GAME OVER", 250, 250, 50, RED);
            DrawText(TextFormat("Your score was: %d", SCORE), 300, 300, 20, WHITE);
            DrawText("Press [ENTER] to Restart", 200, 350, 30, WHITE);

            // Allows the user to restart the game
            if (IsKeyPressed(KEY_ENTER))
            {
                // Reset the game state
                PLAYER_LIFE = 3;
                bullet_count = 0;
                player.position = (Vector2){400, 550};
                player.state = IDLE;
                isGameOver = false;
                SCORE = 0;
            }
        }
        else
        {
            // Draw the player
            Color player_color = (player.state == HURT && (int)(player.invincibility_timer * 10) % 2 == 0) ? YELLOW : WHITE;
            DrawCircleV(player.position, 15, player_color);

            // Draw bullets
            for (int i = 0; i < bullet_count; i++)
            {
                if (bullets[i].velocity < 0)
                {
                    Vector2 tip = {bullets[i].position.x, bullets[i].position.y - 10};
                    Vector2 left = {bullets[i].position.x - 5, bullets[i].position.y};
                    Vector2 right = {bullets[i].position.x + 5, bullets[i].position.y};

                    DrawTriangle(tip, left, right, YELLOW);
                }
                else
                {
                    DrawCircleV(bullets[i].position, 9, RED);
                }
            }

            DrawText(TextFormat("Health: %d", PLAYER_LIFE), 8, 15, 30, WHITE);
            DrawText(TextFormat("Score: %d", SCORE), 620, 15, 30, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
