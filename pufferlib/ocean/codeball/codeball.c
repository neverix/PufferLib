#include <stdio.h>
#include "codeball.h"
#include "puffernet.h"
#include "renderer.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else  // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

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

    int initial_steps = 2000;

    for (int i = 0; i < 10000; i++) {
        if (WindowShouldClose()) break;

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
