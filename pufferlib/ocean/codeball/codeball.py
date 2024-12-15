from pdb import set_trace as T  # Optional for debugging

import numpy as np
import os

import pettingzoo
import gymnasium

# from pufferlib.ocean.codeball.cy_codeball import CyCodeBall

import pyximport
pyximport.install()

# from .cy_codeball import CyCodeBall
import cy_codeball
CyCodeBall = cy_codeball.CyCodeBall


class CodeBall(pettingzoo.env.ParallelEnv):
    metadata = {
        'render.modes': ['human'],
        'name': 'codeball_v0'
    }

    def __init__(self, num_envs=2, n_robots=3, n_nitros=2, max_steps=1000):
        self.num_envs = num_envs
        self.n_robots = n_robots
        self.n_nitros = n_nitros
        self.max_steps = max_steps

        self.observation_spaces = [gymnasium.spaces.Box(low=-10.0, high=10.0, shape=(12 * (self.n_robots + 1), 7), dtype=np.float32) for _ in range(self.num_envs)]
        self.action_spaces = [gymnasium.spaces.Discrete(4) for _ in range(self.num_envs)]

        self.c_envs = CyCodeBall(self.num_envs, self.n_robots, self.n_nitros)

        self.dones = [False] * self.num_envs
        self.env_rewards = [0.0] * self.num_envs
        self.infos = [{}] * self.num_envs
        self.ts = 0

    def reset(self, seed=None):
        if seed is not None:
            self.c_envs.reset(seed)
        else:
            self.c_envs.reset()
        self.dones = [False] * self.num_envs
        self.env_rewards = [0.0] * self.num_envs
        self.infos = [{}] * self.num_envs
        self.ts = 0

        return self._get_observations()

    def step(self, all_actions):
        actions = np.array(all_actions)
        if actions.shape != (self.num_envs,):
            raise ValueError("Actions must have shape (num_envs,)")

        for i in range(self.num_envs):
            if self.dones[i]:
                continue
            self.env_rewards[i] = self.c_envs.get_rewards()[i][0]  # Assuming single agent rewards

        self.c_envs.step(actions.reshape(self.num_envs, 1))
        self.ts += 1

        self.dones = [ts >= self.max_steps for ts in self.c_envs.get_rewards().sum(axis=1)]
        all_obs = self._get_observations()

        return all_obs, self.env_rewards, self.dones, self.infos

    def render(self, mode='human'):
        if mode == 'human':
            self.c_envs.render(self.ts)

    def close(self):
        self.c_envs.close()

    def _get_observations(self):
        entities = self.c_envs.get_entities()
        nitro_packs = self.c_envs.get_nitro_packs()

        observations = []
        for i in range(self.num_envs):
            entity_data = np.concatenate([entities[i, :self.n_robots], entities[i, self.n_robots:self.n_robots+1]], axis=1)
            nitro_data = nitro_packs[i]
            obs = np.concatenate([entity_data, nitro_data], axis=1)
            observations.append(obs.flatten())

        return observations


if __name__ == '__main__':
    env = CodeBall(num_envs=2)

    # Run an episode
    observation = env.reset()
    for _ in range(100):
        action = [np.random.choice(env.action_spaces[i].n) for i in range(env.num_envs)]
        observation, reward, done, info = env.step(action)