cdef extern from "codeball.h":

    ctypedef double sim_dtype

    ctypedef struct CodeBallArena:
        sim_dtype width
        sim_dtype height
        sim_dtype depth
        sim_dtype bottom_radius
        sim_dtype top_radius
        sim_dtype corner_radius
        sim_dtype goal_top_radius
        sim_dtype goal_width
        sim_dtype goal_depth
        sim_dtype goal_height
        sim_dtype goal_side_radius

    CodeBallArena arena

    ctypedef struct Vec3D:
        sim_dtype x
        sim_dtype y
        sim_dtype z

    ctypedef struct Action:
        Vec3D target_velocity
        sim_dtype jump_speed
        bool use_nitro

    ctypedef struct Entity:
        Vec3D position
        Vec3D velocity
        sim_dtype radius
        sim_dtype radius_change_speed
        sim_dtype mass
        sim_dtype arena_e
        bool touch
        Vec3D touch_normal
        sim_dtype nitro
        Action action
        bool side

    ctypedef struct NitroPack:
        Vec3D position
        bool alive
        int respawn_ticks
        sim_dtype radius

    sim_dtype vec3d_length(Vec3D v)

    Vec3D vec3d_normalize(Vec3D v)

    sim_dtype vec3d_dot(Vec3D a, Vec3D b)

    Vec3D vec3d_subtract(Vec3D a, Vec3D b)

    Vec3D vec3d_add(Vec3D a, Vec3D b)

    Vec3D vec3d_multiply(Vec3D v, sim_dtype s)

    sim_dtype clamp(sim_dtype val, sim_dtype min, sim_dtype max)

    Vec3D vec3d_clamp(Vec3D v, sim_dtype max_length)

    ctypedef struct DistanceAndNormal:
        sim_dtype distance
        Vec3D normal

    DistanceAndNormal dan_to_plane(Vec3D point, Vec3D point_on_plane, Vec3D plane_normal)

    DistanceAndNormal dan_to_sphere_inner(Vec3D point, Vec3D sphere_center, sim_dtype sphere_radius)

    DistanceAndNormal dan_to_sphere_outer(Vec3D point, Vec3D sphere_center, sim_dtype sphere_radius)

    DistanceAndNormal dan_to_arena_quarter(Vec3D point)

    DistanceAndNormal dan_to_arena(Vec3D point)

    void collide_entities(Entity* a, Entity* b)

    Vec3D collide_with_arena(Entity* e)

    void move(Entity* e, sim_dtype delta_time)

    cdef struct CodeBall:
        Entity ball
        int n_robots
        Entity* robots
        int n_nitros
        NitroPack* nitro_packs
        int tick
        int* scores
        double* actions
        double* rewards
        double* terminals
        int rounds
        int frame_skip

    void allocate(CodeBall* env)

    void free_allocated(CodeBall* env)

    void reset_positions(CodeBall* env)

    void goal_scored(CodeBall* env, bool side)

    void reset(CodeBall* env)

    void update(sim_dtype delta_time, CodeBall* env)

    sim_dtype goal_potential(Vec3D position, CodeBallArena* arena, bool side)

    void step(CodeBall* env)

    sim_dtype goal_potential(Vec3D position, CodeBallArena* arena, bool side)
