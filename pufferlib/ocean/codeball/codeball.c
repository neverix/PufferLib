#include "codeball.h"
#include "puffernet.h"
#include "raymath.h"
#include "rlgl.h"
#include <sys/time.h>

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else  // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

void allocate(CodeBall* env) {
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
    Camera3D camera;
    Shader shader;
    RenderTexture2D render_target;
    Model sphere;
};

Client* make_client() {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->width = 800;   // Example width
    client->height = 600;  // Example height
    client->robot_color[0] = RED;
    client->robot_color[1] = BLUE;
    client->ball_color = WHITE;
    client->nitro_color = GREEN;
    client->camera =
        // (Camera3D){(Vector3){0.0f, 40.0f, -60.0f}, (Vector3){0.0f, 0.0f,
        // 0.0f},
        (Camera3D){(Vector3){0.0f, 60.0f, -90.0f}, (Vector3){0.0f, 0.0f, 0.0f},
                   (Vector3){0.0f, 1.0f, 0.0f}, 45.0f, 0};

    InitWindow(client->width, client->height, "CodeBall");

    client->sphere = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));
    client->shader = LoadShader("base.vs", "fragment.fs");
    client->render_target =
        LoadRenderTexture(client->width, client->height);

    SetTargetFPS(60);

    return client;
}

void close_client(Client* client) {
    UnloadRenderTexture(client->render_target);
    CloseWindow();
    free(client);
}

Vector3 cvt(Vec3D vec) {
    return (Vector3){vec.x, vec.y, vec.z};
}

void MyDrawSphere(Client *client, Vector3 center, float radius,
               Color color) {
                DrawModel(client->sphere, center, radius, color);
}

// Custom DrawPlane function for arbitrary orientation
void MyDrawPlane(Vector3 center, Vector3 normal, float width, float height,
               Color color) {
    Vector3 u, v;

    if (fabsf(normal.y) < 0.999f) {
        u = (Vector3){normal.z, 0, -normal.x};
    } else {
        u = (Vector3){1, 0, 0};
    }
    v = Vector3CrossProduct(normal, u);

    u = Vector3Normalize(u);
    v = Vector3Normalize(v);

    Vector3 p1 =
        Vector3Subtract(Vector3Subtract(center, Vector3Scale(u, width / 2)),
                        Vector3Scale(v, height / 2));
    Vector3 p2 = Vector3Add(Vector3Subtract(center, Vector3Scale(u, width / 2)),
                            Vector3Scale(v, height / 2));
    Vector3 p3 = Vector3Add(Vector3Add(center, Vector3Scale(u, width / 2)),
                            Vector3Scale(v, height / 2));
    Vector3 p4 = Vector3Subtract(Vector3Add(center, Vector3Scale(u, width / 2)),
                                 Vector3Scale(v, height / 2));

    DrawTriangleStrip3D((Vector3[]){p1, p2, p4, p3}, 4, color);
}

void render(Client* client, CodeBall* env) {
    Entity* robots = env->robots;
    Entity ball = env->ball;
    NitroPack* nitro_packs = env->nitro_packs;

    BeginTextureMode(client->render_target);

    ClearBackground(DARKGRAY);
    BeginMode3D(client->camera);  // Begin 3D mode

    BeginShaderMode(client->shader);

    // Arena dimensions
    Vector3 arena_size = {arena.width, arena.height,
                          arena.depth};

    // Draw arena (box)
    // DrawCube((Vector3){0, arena.height / 2, 0}, arena_size.x, arena_size.y,
    //          arena_size.z, LIGHTGRAY);
    // DrawCube((Vector3){0, -arena.height / 2, 0}, arena_size.x, arena_size.y,
    //          arena_size.z, LIGHTGRAY);

    MyDrawPlane((Vector3){0, 0, 0}, (Vector3){0, -1, 0}, arena_size.x,
              arena_size.z, LIGHTGRAY);  // Bottom (CORRECTED NORMAL)
    MyDrawPlane((Vector3){arena_size.x / 2, arena_size.y / 2, 0},
              (Vector3){1, 0, 0}, arena_size.z, arena_size.y,
              LIGHTGRAY);  // Right
    MyDrawPlane((Vector3){-arena_size.x / 2, arena_size.y / 2, 0},
              (Vector3){-1, 0, 0}, arena_size.z, arena_size.y,
              LIGHTGRAY);  // Left
    
    for (int fb = 0; fb < 2; fb++) {
        int sign = fb == 0 ? 1 : -1;
        Vector3 normal = {0, 0, sign};
        MyDrawPlane(
            (Vector3){0, arena.goal_height + (arena_size.y - arena.goal_height) / 2,
                    arena_size.z / 2 * sign},
            normal, arena.goal_width,
            (arena_size.y - arena.goal_height),
            LIGHTGRAY);  // Back middle
        double remaining = (arena_size.x - arena.goal_width) / 2;
        MyDrawPlane((Vector3){-arena.goal_width / 2 - remaining / 2, arena_size.y / 2,
                            arena_size.z / 2 * sign},
                    normal, remaining, arena_size.y,
                    LIGHTGRAY);  // Back right
        MyDrawPlane((Vector3){arena.goal_width / 2 + remaining / 2,
                              arena_size.y / 2, arena_size.z / 2 * sign},
                    normal, remaining, arena_size.y,
                    LIGHTGRAY);  // Back left
    }

    MyDrawPlane((Vector3){0, arena_size.y / 2, -arena_size.z / 2},
                (Vector3){0, 0, -1}, arena_size.x, arena_size.y,
                LIGHTGRAY);  // Front

    // Draw Goals (as boxes)
    Vector3 goal_size = {arena.goal_width, arena.goal_height, arena.goal_depth};

    Color goal_colors[2] = {GREEN, YELLOW};
    for (int gi = 0; gi < 2; gi++) {
        Color goal_color = goal_colors[gi];
        int sign = gi == 0 ? 1 : -1;
        MyDrawPlane(
            (Vector3){0, goal_size.y / 2, sign * (arena.depth / 2 + arena.goal_depth)},
            (Vector3){0, 0, sign}, goal_size.x, goal_size.y, goal_color);  // Back
        MyDrawPlane((Vector3){-goal_size.x / 2, goal_size.y / 2,
                              sign * (arena.depth / 2 + goal_size.z / 2)},
                    (Vector3){-1, 0, 0}, goal_size.z, goal_size.y,
                    goal_color);  // Left
        MyDrawPlane((Vector3){goal_size.x / 2, goal_size.y / 2,
                              sign * (arena.depth / 2 + goal_size.z / 2)},
                    (Vector3){1, 0, 0}, goal_size.z, goal_size.y,
                    goal_color);  // Right
        MyDrawPlane(
            (Vector3){0, goal_size.y, sign * (arena.depth / 2 + goal_size.z / 2)},
            (Vector3){0, 1, 0}, goal_size.x, goal_size.z, goal_color);  // Top

        MyDrawPlane((Vector3){0, 0, sign * (arena.depth / 2 + goal_size.z / 2)},
                    (Vector3){0, -1, 0}, goal_size.x, goal_size.z, goal_color);  // Bottom
    }

    // Draw robots (colored by side)
    for (int i = 0; i < env->n_robots; i++) {
        Color robot_color =
            robots[i].side ? client->robot_color[1] : client->robot_color[0];
        // DrawCube(cvt(robots[i].position), robots[i].radius * 2, robots[i].radius * 2,
        //          robots[i].radius * 2, robot_color);
        MyDrawSphere(client, cvt(robots[i].position), robots[i].radius, robot_color);
    }

    // Draw ball (as a box)
    // DrawCube(cvt(ball.position), ball.radius * 2, ball.radius * 2, ball.radius * 2,
    //          client->ball_color);
    MyDrawSphere(client, cvt(ball.position), ball.radius, client->ball_color);

    // Draw nitro packs (as boxes)
    for (int i = 0; i < env->n_nitros; i++) {
        if (nitro_packs[i].alive) {
            MyDrawSphere(client, cvt(nitro_packs[i].position), nitro_packs[i].radius * 2,
                     client->nitro_color);
        }
    }

    EndShaderMode();

    EndMode3D();  // End 3D mode
    EndTextureMode();


    BeginDrawing();

    ClearBackground(RAYWHITE);
    BeginShaderMode(client->shader);
    // NOTE: Render texture must be y-flipped due to default OpenGL coordinates
    // (left-bottom)
    DrawTextureRec(client->render_target.texture,
                   (Rectangle){0, 0, (float)client->render_target.texture.width,
                               (float)-client->render_target.texture.height},
                   (Vector2){0, 0}, WHITE);
    EndShaderMode();

    DrawFPS(10, 10);

    EndDrawing();


}

// void render(Client* client, CodeBall* env) {
//     BeginDrawing();
//     ClearBackground(DARKGRAY);

//     Entity* robots = env->robots;
//     Entity ball = env->ball;
//     NitroPack* nitro_packs = env->nitro_packs;

//     // Arena dimensions for scaling
//     float arena_width_scaled = arena.width * client->width / arena.width;
//     float arena_depth_scaled = (arena.depth + 2 * arena.goal_depth) * client->height / (arena.depth + 2 * arena.goal_depth);
//     float arena_x_offset = client->width / 2.0f;
//     float arena_z_offset = client->height / 2.0f;

//     // Draw background
//     DrawRectangle(0, 0, client->width, client->height, DARKGRAY);

//     // Draw arena (now a separate colored rectangle)
//     DrawRectangle(arena_x_offset - arena_width_scaled / 2, arena_z_offset - arena_depth_scaled / 2, arena_width_scaled, arena_depth_scaled, LIGHTGRAY);

//     // Draw goal areas (Corrected positioning and size)
//     float goal_width_scaled = arena.goal_width * client->width / arena.width;
//     float goal_depth_scaled = arena.goal_depth * client->height / (arena.depth + 2 * arena.goal_depth);
//     float goal_height_scaled = arena.goal_height * client->height / (arena.depth + 2 * arena.goal_depth);

//     DrawRectangle(arena_x_offset - goal_width_scaled / 2, arena_z_offset + arena_depth_scaled / 2 - goal_depth_scaled, goal_width_scaled, goal_height_scaled, GREEN); // Blue goal
//     DrawRectangle(arena_x_offset - goal_width_scaled / 2, arena_z_offset - arena_depth_scaled / 2, goal_width_scaled, goal_height_scaled, YELLOW); // Red goal

//     // Draw robots (Colored by side)
//     for (int i = 0; i < env->n_robots; i++) {
//         Color robot_color = robots[i].side ? client->robot_color[1] : client->robot_color[0]; // Right is blue, left is red
//         Vector2 robot_pos = {
//             arena_x_offset + robots[i].position.x * arena_width_scaled / arena.width,
//             arena_z_offset - robots[i].position.z * arena_depth_scaled / (arena.depth + 2 * arena.goal_depth)
//         };
//         DrawCircleV(robot_pos, robots[i].radius * arena_width_scaled / arena.width, robot_color);
//     }

//     // Draw ball
//     Vector2 ball_pos = {
//         arena_x_offset + ball.position.x * arena_width_scaled / arena.width,
//         arena_z_offset - ball.position.z * arena_depth_scaled / (arena.depth + 2 * arena.goal_depth)
//     };
//     DrawCircleV(ball_pos, ball.radius * arena_width_scaled / arena.width, client->ball_color);

//     // Draw nitro packs
//     for (int i = 0; i < env->n_nitros; i++) {
//         if (nitro_packs[i].alive) {
//             Vector2 nitro_pos = {
//                 arena_x_offset + nitro_packs[i].position.x * arena_width_scaled / arena.width,
//                 arena_z_offset - nitro_packs[i].position.z * arena_depth_scaled / (arena.depth + 2 * arena.goal_depth)
//             };
//             DrawCircleV(nitro_pos, nitro_packs[i].radius * arena_width_scaled / arena.width, client->nitro_color);
//         }
//     }

//     DrawFPS(10, 10);
//     EndDrawing();
// }

int main() {
    srand(time(NULL)); // Seed the random number generator
    Client* client = make_client();

    CodeBall env;
    env.n_robots = 50;
    env.n_nitros = 4;
    allocate(&env);
    reset(&env);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int initial_steps = 1;

    for (int i = 0; i < 10000; i++) {
        if (WindowShouldClose()) break;

        // Camera controls
        Vector3 addition = {0, 0, 0};
        Vector3 camera_forward = Vector3Normalize(Vector3Subtract(client->camera.target, client->camera.position));
        Vector3 camera_right = Vector3Normalize(Vector3CrossProduct(camera_forward, (Vector3){0, 1, 0}));
        Vector3 camera_up = Vector3Normalize(Vector3CrossProduct(camera_right, camera_forward));

        if (IsKeyDown(KEY_W)) addition = Vector3Add(addition, camera_forward);
        if (IsKeyDown(KEY_S)) addition = Vector3Subtract(addition, camera_forward);
        if (IsKeyDown(KEY_D)) addition = Vector3Add(addition, camera_right);
        if (IsKeyDown(KEY_A)) addition = Vector3Subtract(addition, camera_right);
        if (IsKeyDown(KEY_E)) addition = Vector3Add(addition, camera_up);
        if (IsKeyDown(KEY_Q)) addition = Vector3Subtract(addition, camera_up);
        addition = Vector3Scale(Vector3Normalize(addition), 0.5f);
        client->camera.position = Vector3Add(client->camera.position, addition);

        // if (IsKeyDown(KEY_D)) client->camera.position.x -= 0.5f;
        // if (IsKeyDown(KEY_A)) client->camera.position.x += 0.5f;
        // if (IsKeyDown(KEY_E)) client->camera.position.y += 0.5f;
        // if (IsKeyDown(KEY_Q)) client->camera.position.y -= 0.5f;
        // if (IsKeyDown(KEY_S)) client->camera.position.z -= 0.5f;
        // if (IsKeyDown(KEY_W)) client->camera.position.z += 0.5f;

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
