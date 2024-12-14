#include "codeball.h"
#include "puffernet.h"
#include <sys/time.h>

void allocate(CodeBall* env) {
    if (env->n_robots < 2 || env->n_robots > 6) {
        printf("Invalid number of robots\n");
        exit(1);
    }
    if (env->n_nitros != 4 && env->n_nitros != 0) {
        printf("Invalid number of nitro packs\n");
        exit(1);
    }
    env->robots = (Entity*)calloc(env->n_robots, sizeof(Entity));
    env->nitro_packs = (NitroPack*)calloc(env->n_nitros, sizeof(NitroPack));
    env->actions = (double*)calloc(env->n_robots * 4, sizeof(double));
    env->rewards = (double*)calloc(env->n_robots, sizeof(double));
}

void free_allocated(CodeBall* env) {
    free(env->robots);
    free(env->nitro_packs);
    free(env->actions);
    free(env->rewards);
}

typedef struct Client Client;
struct Client {
    float width;
    float height;
    Color robot_color[2];
    Color ball_color;
    Color nitro_color;
};

Client* make_client() {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->width = 800;   // Example width
    client->height = 600;  // Example height
    client->robot_color[0] = RED;
    client->robot_color[1] = BLUE;
    client->ball_color = WHITE;
    client->nitro_color = GREEN;

    InitWindow(client->width, client->height, "CodeBall");
    SetTargetFPS(60);

    return client;
}

void close_client(Client* client) {
    CloseWindow();
    free(client);
}

void render(Client* client, CodeBall* env) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    Entity* robots = env->robots;
    Entity ball = env->ball;
    NitroPack* nitro_packs = env->nitro_packs;

    // Arena dimensions for scaling
    float arena_width_scaled = arena.width * client->width / arena.width;
    float arena_depth_scaled = (arena.depth + 2 * arena.goal_depth) * client->height / (arena.depth + 2 * arena.goal_depth);
    float arena_x_offset = client->width / 2.0f;
    float arena_z_offset = client->height / 2.0f;

    // Draw background
    DrawRectangle(0, 0, client->width, client->height, DARKGRAY);

    // Draw arena (now a separate colored rectangle)
    DrawRectangle(arena_x_offset - arena_width_scaled / 2, arena_z_offset - arena_depth_scaled / 2, arena_width_scaled, arena_depth_scaled, LIGHTGRAY);

    // Draw goal areas (Corrected positioning and size)
    float goal_width_scaled = arena.goal_width * client->width / arena.width;
    float goal_depth_scaled = arena.goal_depth * client->height / (arena.depth + 2 * arena.goal_depth);
    float goal_height_scaled = arena.goal_height * client->height / (arena.depth + 2 * arena.goal_depth);

    DrawRectangle(arena_x_offset - goal_width_scaled / 2, arena_z_offset + arena_depth_scaled / 2 - goal_depth_scaled, goal_width_scaled, goal_height_scaled, GREEN); // Blue goal
    DrawRectangle(arena_x_offset - goal_width_scaled / 2, arena_z_offset - arena_depth_scaled / 2, goal_width_scaled, goal_height_scaled, YELLOW); // Red goal

    // Draw robots (Colored by side)
    for (int i = 0; i < env->n_robots; i++) {
        Color robot_color = robots[i].side ? client->robot_color[1] : client->robot_color[0]; // Right is blue, left is red
        Vector2 robot_pos = {
            arena_x_offset + robots[i].position.x * arena_width_scaled / arena.width,
            arena_z_offset - robots[i].position.z * arena_depth_scaled / (arena.depth + 2 * arena.goal_depth)
        };
        DrawCircleV(robot_pos, robots[i].radius * arena_width_scaled / arena.width, robot_color);
    }

    // Draw ball
    Vector2 ball_pos = {
        arena_x_offset + ball.position.x * arena_width_scaled / arena.width,
        arena_z_offset - ball.position.z * arena_depth_scaled / (arena.depth + 2 * arena.goal_depth)
    };
    DrawCircleV(ball_pos, ball.radius * arena_width_scaled / arena.width, client->ball_color);

    // Draw nitro packs
    for (int i = 0; i < env->n_nitros; i++) {
        if (nitro_packs[i].alive) {
            Vector2 nitro_pos = {
                arena_x_offset + nitro_packs[i].position.x * arena_width_scaled / arena.width,
                arena_z_offset - nitro_packs[i].position.z * arena_depth_scaled / (arena.depth + 2 * arena.goal_depth)
            };
            DrawCircleV(nitro_pos, nitro_packs[i].radius * arena_width_scaled / arena.width, client->nitro_color);
        }
    }

    DrawFPS(10, 10);
    EndDrawing();
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    Client* client = make_client();

    CodeBall env;
    env.n_robots = 6;
    env.n_nitros = 4;
    allocate(&env);
    reset(&env);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int initial_steps = 10;

    for (int i = 0; i < 10000; i++) {
        for (int j = 0; j < env.n_robots; j++)
        {
            Vec3D tgt =
                vec3d_subtract(env.ball.position, env.robots[j].position);
            for (int k = 0; k < env.n_robots; k++) {
                if (k != j) {
                    Vec3D diff = vec3d_subtract(env.robots[k].position, env.robots[j].position);
                    double diff_len = vec3d_length(diff);
                    if (diff_len < 2.5) {
                        tgt = vec3d_multiply(diff, -1.0);
                    }
                }
            }
            tgt = vec3d_multiply(tgt, 4.0);
            env.actions[j * 4] = tgt.x;
            env.actions[j * 4 + 1] = tgt.y;
            env.actions[j * 4 + 2] = tgt.z;
        }
        step(&env);
        if (i == initial_steps) {
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
                            (end.tv_usec - start.tv_usec) / 1000000.0;
            printf("%d steps took %f seconds\n", initial_steps, elapsed);
            printf("SPS: \t%f\n", ((double)initial_steps) / elapsed);
        }
        if (i > initial_steps) {
            render(client, &env);
        }
    }

    close_client(client);

    return 0;
}
