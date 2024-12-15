from libc.stdlib cimport calloc, free, rand, srand
import numpy as np

cdef extern from "codeball.h":
    # Constants
    double ROBOT_MIN_RADIUS
    double ROBOT_MAX_RADIUS
    double ROBOT_MAX_JUMP_SPEED
    double ROBOT_ACCELERATION
    double ROBOT_NITRO_ACCELERATION
    double ROBOT_MAX_GROUND_SPEED
    double ROBOT_ARENA_E
    double ROBOT_MASS
    int TICKS_PER_SECOND
    int MICROTICKS_PER_TICK
    int RESET_TICKS
    double BALL_ARENA_E
    double BALL_RADIUS
    double BALL_MASS
    double MIN_HIT_E
    double MAX_HIT_E
    double MAX_ENTITY_SPEED
    double MAX_NITRO_AMOUNT
    double START_NITRO_AMOUNT
    double NITRO_POINT_VELOCITY_CHANGE
    double NITRO_PACK_X
    double NITRO_PACK_Y
    double NITRO_PACK_Z
    double NITRO_PACK_RADIUS
    int NITRO_PACK_AMOUNT
    int NITRO_PACK_RESPAWN_TICKS
    double GRAVITY

    # Structs
    ctypedef struct CodeBallArena:
        double width
        double height
        double depth
        double bottom_radius
        double top_radius
        double corner_radius
        double goal_top_radius
        double goal_width
        double goal_depth
        double goal_height
        double goal_side_radius

    ctypedef struct Vec3D:
        double x
        double y
        double z

    ctypedef struct Action:
        Vec3D target_velocity
        double jump_speed
        bint use_nitro

    ctypedef struct Entity:
        Vec3D position
        Vec3D velocity
        double radius
        double radius_change_speed
        double mass
        double arena_e
        bint touch
        Vec3D touch_normal
        double nitro
        Action action
        bint side

    ctypedef struct NitroPack:
        Vec3D position
        bint alive
        int respawn_ticks
        double radius

    ctypedef struct CodeBall:
        Entity ball
        int n_robots
        Entity* robots
        int n_nitros
        NitroPack* nitro_packs
        int tick
        double* actions
        double* rewards

    # Functions
    void reset(CodeBall* env)
    void step(CodeBall* env)

cpdef vec3d_dtype():
    cdef Vec3D v
    return np.asarray(<Vec3D[:1]>&v).dtype

cpdef entity_dtype():
    cdef Entity e
    return np.asarray(<Entity[:1]>&e).dtype

cpdef nitro_pack_dtype():
    cdef NitroPack np_
    return np.asarray(<NitroPack[:1]>&np_).dtype

cdef class CyCodeBall:
    cdef CodeBall* envs
    cdef int num_envs
    cdef double* action_buffer
    cdef double* reward_buffer

    def __init__(self, int num_envs, int n_robots, int n_nitros):
        self.num_envs = num_envs
        self.envs = <CodeBall*>calloc(num_envs, sizeof(CodeBall))
        self.action_buffer = <double*>calloc(num_envs * n_robots * 4, sizeof(double))
        self.reward_buffer = <double*>calloc(num_envs * n_robots, sizeof(double))

        cdef int i
        for i in range(num_envs):
            self.envs[i].n_robots = n_robots
            self.envs[i].robots = <Entity*>calloc(n_robots, sizeof(Entity))
            self.envs[i].n_nitros = n_nitros
            self.envs[i].nitro_packs = <NitroPack*>calloc(n_nitros, sizeof(NitroPack))
            self.envs[i].actions = &self.action_buffer[i * n_robots * 4]
            self.envs[i].rewards = &self.reward_buffer[i * n_robots]
            # Initialize Nitro Packs
            for j in range(n_nitros):
                self.envs[i].nitro_packs[j].position.x = NITRO_PACK_X;
                self.envs[i].nitro_packs[j].position.y = NITRO_PACK_Y;
                self.envs[i].nitro_packs[j].position.z = NITRO_PACK_Z * (j+1);
                self.envs[i].nitro_packs[j].alive = True;
                self.envs[i].nitro_packs[j].radius = NITRO_PACK_RADIUS;

    def reset(self, int seed):
        srand(seed)
        cdef int i
        for i in range(self.num_envs):
            reset(&self.envs[i])

    def step(self, np.ndarray[np.float64_t, ndim=2] actions):
        cdef int i, j
        if actions.shape != (self.num_envs, self.envs[0].n_robots * 4):
            raise ValueError("Actions array has incorrect shape.")

        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots * 4):
                self.envs[i].actions[j] = actions[i, j]
        for i in range(self.num_envs):
            step(&self.envs[i])

    def get_entities(self):
        cdef np.ndarray[entity_dtype(), ndim=2] entities = np.empty((self.num_envs, self.envs[0].n_robots + 1), dtype=entity_dtype())
        cdef int i, j
        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots):
                entities[i, j] = self.envs[i].robots[j]
            entities[i, self.envs[0].n_robots] = self.envs[i].ball
        return entities

    def get_nitro_packs(self):
        cdef np.ndarray[nitro_pack_dtype(), ndim=2] nitro_packs = np.empty((self.num_envs, self.envs[0].n_nitros), dtype=nitro_pack_dtype())
        cdef int i, j
        for i in range(self.num_envs):
            for j in range(self.envs[0].n_nitros):
                nitro_packs[i, j] = self.envs[i].nitro_packs[j]
        return nitro_packs

    def get_rewards(self):
        cdef np.ndarray[np.float64_t, ndim=2] rewards = np.empty((self.num_envs, self.envs[0].n_robots), dtype=np.float64)
        cdef int i, j
        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots):
                rewards[i, j] = self.envs[i].rewards[j]
        return rewards

    def close(self):
        cdef int i
        for i in range(self.num_envs):
            free(self.envs[i].robots)
            free(self.envs[i].nitro_packs)
        free(self.envs)
        free(self.action_buffer)
        free(self.reward_buffer)
