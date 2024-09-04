#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define MAX_ENEMY_BULLETS 12
#define MAX_PLAYER_BULLETS 512
#define INVINCIBILITY_DURATION 1.0f

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
} player_bullet_t;

typedef struct
{
    Vector2 position;
    float velocity;
} enemy_bullet_t;

typedef struct
{
    Vector2 position;
    float velocity;
    PlayerState state;
    float invincibility_timer; // Tracks the invincibility duration
    float shoot_cooldown;
    float shoot_cooldown_duration;
} player_t;

player_t player = {
    .position.x = 400,
    .position.y = 550,
    .velocity = 430,
    .state = IDLE,
    .invincibility_timer = 0,
    .shoot_cooldown = 0,
    .shoot_cooldown_duration = 0.08f};

player_bullet_t player_bullets[MAX_PLAYER_BULLETS];
enemy_bullet_t enemy_bullets[MAX_ENEMY_BULLETS];

int player_bullet_count = 0;
int enemy_bullet_count = 0;

void shoot_bullets()
{
    player_bullets[player_bullet_count].position = (Vector2){player.position.x, player.position.y - 20}; // Slightly above the player
    player_bullets[player_bullet_count].velocity = -600.0f;
    player_bullet_count++;
}

void bullets_spawn()
{
    if (enemy_bullet_count < MAX_ENEMY_BULLETS)
    {
        float random_pos_x = (float)(rand() % 800);             // Random x-position between 0 and 800
        float random_velocity = 200.0f + (float)(rand() % 100); // Random velocity between 200 and 300

        enemy_bullets[enemy_bullet_count].position = (Vector2){random_pos_x, 5.0f}; // Start at the top of the screen
        enemy_bullets[enemy_bullet_count].velocity = random_velocity;

        enemy_bullet_count++;
    }
}

void update_bullets(float delta_time)
{
    // Update player bullets
    for (int i = 0; i < player_bullet_count; i++)
    {
        player_bullets[i].position.y += player_bullets[i].velocity * delta_time;

        // Reset bullet if it goes off screen
        if (player_bullets[i].position.y < 0)
        {
            player_bullets[i] = player_bullets[player_bullet_count - 1];
            player_bullet_count--;
            i--;
            continue;
        }

        // Check collision between player bullets and falling bullets
        for (int j = 0; j < enemy_bullet_count; j++)
        {
            float collision = CheckCollisionCircles(player_bullets[i].position, 5, enemy_bullets[j].position, 5);

            if (collision)
            {
                // Add points for destroyed bullet
                SCORE += 10;

                // Remove both bullets
                player_bullets[i] = player_bullets[player_bullet_count - 1];
                player_bullet_count--;

                enemy_bullets[j] = enemy_bullets[enemy_bullet_count - 1];
                enemy_bullet_count--;

                i--;
                break;
            }
        }
    }

    // Update falling bullets
    for (int i = 0; i < enemy_bullet_count; i++)
    {
        enemy_bullets[i].position.y += enemy_bullets[i].velocity * delta_time;

        // Reset bullet if it goes off screen
        if (enemy_bullets[i].position.y > 600)
        {
            enemy_bullets[i] = enemy_bullets[enemy_bullet_count - 1];
            enemy_bullet_count--;
            i--;
            continue;
        }

        // Check collision with player
        float collision_player = CheckCollisionCircles(enemy_bullets[i].position, 5, player.position, 15);

        if (player.state == IDLE && collision_player) // Player radius (15) + bullet radius (5)
        {
            player.state = HURT;
            player.invincibility_timer = INVINCIBILITY_DURATION;
            PLAYER_LIFE--;

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

    if (player.shoot_cooldown > 0)
    {
        player.shoot_cooldown -= delta_time;

        if (player.shoot_cooldown < 0)
        {
            player.shoot_cooldown = 0;
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

    if (IsKeyDown(KEY_SPACE) && player.shoot_cooldown == 0)
    {
        shoot_bullets();
        player.shoot_cooldown = player.shoot_cooldown_duration;
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
                player_bullet_count = 0;
                enemy_bullet_count = 0;
                player.position = (Vector2){400, 550};
                player.state = IDLE;
                player.shoot_cooldown = 0;
                isGameOver = false;
                SCORE = 0;
            }
        }
        else
        {
            // Draw the player
            Color player_color = (player.state == HURT && (int)(player.invincibility_timer * 10) % 2 == 0) ? YELLOW : WHITE;
            DrawCircleV(player.position, 15, player_color);

            // Draw Player bullets
            for (int i = 0; i < player_bullet_count; i++)
            {
                Vector2 tip = {player_bullets[i].position.x, player_bullets[i].position.y - 10};
                Vector2 left = {player_bullets[i].position.x - 5, player_bullets[i].position.y};
                Vector2 right = {player_bullets[i].position.x + 5, player_bullets[i].position.y};

                DrawTriangle(tip, left, right, YELLOW);
            }

            for (int i = 0; i < enemy_bullet_count; i++)
            {
                DrawCircleV(enemy_bullets[i].position, 9, RED);
            }

            DrawText(TextFormat("Health: %d", PLAYER_LIFE), 8, 15, 30, WHITE);
            DrawText(TextFormat("Score: %d", SCORE), 620, 15, 30, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
