import numpy as np
from tqdm.auto import trange

import pufferlib

try:
    from .cy_codeball import CyCodeBall, robot_max_jump_speed, robot_max_ground_speed
except ImportError:
    from cy_codeball import CyCodeBall, robot_max_jump_speed, robot_max_ground_speed

import gymnasium as gym
import numpy as np
import pufferlib


class CodeBall(pufferlib.PufferEnv):
    def __init__(self, num_envs=2, n_robots=6, n_nitros=2, max_steps=1000,
                 reward_mul=1.0,
                 frame_skip=4, buf=None):
        self.num_envs = num_envs
        self.num_agents = n_robots * num_envs
        self.n_robots = n_robots
        self.n_nitros = n_nitros
        self.max_steps = max_steps

        # Define observation and action spaces
        self.single_observation_space = gym.spaces.Box(low=-1, high=1, shape=(self.n_robots + 2, 3), dtype=np.float32)
        self.single_action_space = gym.spaces.MultiDiscrete([8, 2])

        super().__init__(buf=buf)

        self.c_envs = CyCodeBall(
            self.num_envs, self.n_robots, self.n_nitros, frame_skip, reward_mul, max_steps,
            self.observations, self.actions, self.rewards, self.terminals, self.truncations,
        )

    def reset(self, seed=None):
        self.c_envs.reset()
        return self.observations, []

    def step(self, actions):
        if actions.shape != (self.num_envs * self.n_robots, 2):
            raise ValueError(f"Actions shape incorrect. Expected {(self.num_envs, self.n_robots * 2)}, got {actions.shape}")

        self.actions[:] = actions
        self.c_envs.step()

        return (
            self.observations,
            self.rewards,
            self.terminals,
            self.truncations,
            [self.c_envs.log()],
        )

    def close(self):
        self.c_envs.close()

if __name__ == '__main__':
    env = CodeBall(num_envs=128)
    obs, _ = env.reset(seed=42)
    print("Observation Space:", env.single_observation_space)
    print("Action Space:", env.single_action_space)
    print("Initial Observations Shape:", obs.shape)

    actions = np.array([[0, 0]])
    actions = np.tile(actions, (env.num_envs * env.n_robots, 1))
    # actions[:, :] = 3
    actions[:, :] = 0
    all_obs = []
    all_rewards = []
    for _ in trange(100_000):
        actions[:, 0] = np.random.randint(0, 8, size=actions.shape[0])
        # actions[::2, 0] = 6 + np.random.randint(0, 3, size=actions.shape[0] // 2)
        # actions[1::2, 0] = 2 + np.random.randint(0, 3, size=actions.shape[0] // 2)
        obs, rewards, terminated, truncated, info = env.step(actions)
        all_obs.append(obs.copy())
        all_rewards.append(rewards.copy())
    obs = np.stack(all_obs)
    rewards = np.stack(all_rewards)
    from matplotlib import pyplot as plt
    plt.plot(obs[:, 0, 0, 0], label="Robot 0 X")
    plt.plot(obs[:, 0, 0, 1], label="Robot 0 Y")
    plt.plot(obs[:, 0, 0, 2], label="Robot 0 Z")
    plt.plot(obs[:, 0, -1, 0], label="Ball X")
    plt.plot(obs[:, 0, -1, 1], label="Ball Y")
    plt.plot(obs[:, 0, -1, 2], label="Ball Z")
    plt.plot(rewards[:, 0] * 100, label="Rewards")
    plt.legend()
    plt.pause(5)
    plt.close()
    print("Rewards:", rewards.sum(0))
    print("Terminated:", terminated)
    print("Truncated:", truncated)
    print("Observations shape:", obs.shape)

    env.close()

# python setup.py build_ext --inplace && python codeball.py