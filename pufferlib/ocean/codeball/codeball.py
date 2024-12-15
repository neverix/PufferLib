import numpy as np
from tqdm.auto import trange

import pufferlib

import cy_codeball
CyCodeBall = cy_codeball.CyCodeBall

import gymnasium as gym
import numpy as np
import pufferlib


class CodeBall(pufferlib.PufferEnv):
    def __init__(self, num_envs=2, n_robots=6, n_nitros=2, max_steps=1000, buf=None):
        self.num_envs = num_envs
        self.num_agents = n_robots * num_envs
        self.n_robots = n_robots
        self.n_nitros = n_nitros
        self.max_steps = max_steps

        # Define observation and action spaces
        self.single_observation_space = gym.spaces.Box(low=-np.inf, high=np.inf, shape=(12 * (self.n_robots + 1),), dtype=np.float32)  # Adjusted shape
        self.single_action_space = gym.spaces.MultiDiscrete([9, 2] * self.n_robots) # 9 actions for velocity, 2 for jump

        super().__init__(buf=buf)

        self.c_envs = CyCodeBall(self.num_envs, self.n_robots, self.n_nitros)

    def _get_observations(self):
        return self.c_envs.get_observations()

    def reset(self, seed=None):
        if seed is not None:
            self.c_envs.reset(seed)
        else:
            self.c_envs.reset(0) # provide default seed
        return self._get_observations(), []

    def step(self, actions):
        if actions.shape != (self.num_envs, self.n_robots * 2):
            raise ValueError(f"Actions shape incorrect. Expected {(self.num_envs, self.n_robots * 2)}, got {actions.shape}")

        vel_actions = actions[:, ::2]
        jump_actions = actions[:, 1::2]
        vels = np.array([[0.0, 0.0], [-1.0, 0.0], [-1.0, -1.0], [0.0, -1.0], [1.0, -1.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [-1.0, 1.0]])
        vel = vels[vel_actions]
        jump = jump_actions.astype(np.bool_) * 15.0
        actions = np.concatenate([vel, jump[..., None], jump[..., None] * 0], axis=-1)
        reshaped_actions = actions.reshape(self.num_envs, self.n_robots, 4)
        self.c_envs.step(reshaped_actions)

        truncated = np.array([self.c_envs.get_tick() >= self.max_steps for i in range(self.num_envs)])
        terminated = np.any(self.c_envs.get_terminals() != 0, axis=1)

        return (
            self._get_observations(),
            self.c_envs.get_rewards(),
            terminated,
            truncated,
            [{}] * self.num_envs  # Empty infos
        )

    def close(self):
        self.c_envs.close()

if __name__ == '__main__':
    env = CodeBall(num_envs=2)
    obs, _ = env.reset(seed=42)
    print("Observation Space:", env.single_observation_space)
    print("Action Space:", env.single_action_space)
    print("Initial Observations Shape:", obs.shape)

    actions = np.array([[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]])
    actions[0, ::2] = 1
    all_obs = []
    all_rewards = []
    for _ in trange(10000):
        obs, rewards, terminated, truncated, info = env.step(actions)
        all_obs.append(obs)
        all_rewards.append(rewards)
    obs = np.stack(all_obs)
    rewards = np.stack(all_rewards)
    from matplotlib import pyplot as plt
    plt.plot(obs[:, 0, 0, 0], label="Robot 0 X")
    plt.plot(obs[:, 0, 0, 1], label="Robot 0 Y")
    plt.plot(obs[:, 0, 0, 2], label="Robot 0 Z")
    plt.plot(obs[:, 0, -1, 0], label="Ball X")
    plt.plot(obs[:, 0, -1, 1], label="Ball Y")
    plt.plot(obs[:, 0, -1, 2], label="Ball Z")
    plt.plot(rewards[:, 0, 0] * 1000, label="Rewards")
    plt.legend()
    plt.pause(5)
    plt.close()
    print("Rewards:", rewards)
    print("Terminated:", terminated)
    print("Truncated:", truncated)
    print("Observations shape:", obs.shape)

    env.close()

# python setup.py build_ext --inplace && python codeball.py