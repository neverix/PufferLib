from libc.stdlib cimport calloc, free, rand, srand
import numpy as np
cimport numpy as cnp

cdef extern from "stdbool.h":
    ctypedef bint bool

include "codeball.pxd"

cpdef vec3d_dtype():
    cdef Vec3D v
    return np.asarray(<Vec3D[:1]>&v).dtype

cpdef entity_dtype():
    cdef Entity e
    return np.asarray(<Entity[:1]>&e).dtype

cpdef nitro_pack_dtype():
    cdef NitroPack np_
    return np.asarray(<NitroPack[:1]>&np_).dtype

cpdef ent_array(Entity e):
    return np.array([
        e.position.x, e.position.y, e.position.z,
        e.velocity.x, e.velocity.y, e.velocity.z,],
        dtype=np.float64)

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
            self.envs[i].n_nitros = n_nitros
            allocate(&self.envs[i]) # allocate memory for each env

    def get_observations(self):
        cdef cnp.ndarray[cnp.float64_t, ndim=3] obs = \
            np.empty((self.num_envs, self.envs[0].n_robots + 1, 6), dtype=np.float64)
        cdef int i, j
        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots):
                obs[i, j] = ent_array(self.envs[i].robots[j])
            obs[i, self.envs[0].n_robots] = ent_array(self.envs[i].ball)
        return obs

    def get_terminals(self):
        cdef cnp.ndarray[cnp.float64_t, ndim=2] terminals = \
            np.empty((self.num_envs, self.envs[0].n_robots), dtype=np.float64)
        cdef int i, j
        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots):
                terminals[i, j] = self.envs[i].terminals[j]
        return terminals
    
    def get_rewards(self):
        cdef cnp.ndarray[cnp.float64_t, ndim=2] rewards = \
            np.empty((self.num_envs, self.envs[0].n_robots), dtype=np.float64)
        cdef int i, j
        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots):
                rewards[i, j] = self.envs[i].rewards[j]
        return rewards

    def get_tick(self):
        return self.envs[0].tick

    def reset(self, int seed):
        srand(seed)
        cdef int i
        for i in range(self.num_envs):
            reset(&self.envs[i])

    def step(self, actions):
    # cpdef step(self, np.ndarray[np.float64_t, ndim=3] actions):
        cdef int i, j
        if actions.shape != (self.num_envs, self.envs[0].n_robots, 4):
            raise ValueError("Actions array has incorrect shape.")

        for i in range(self.num_envs):
            for j in range(self.envs[0].n_robots):
                for k in range(4):
                    self.envs[i].actions[j * 4 + k] = actions[i, j, k]
        for i in range(self.num_envs):
            step(&self.envs[i])

    def close(self):
        cdef int i
        for i in range(self.num_envs):
            free_allocated(&self.envs[i])
        free(self.envs)
