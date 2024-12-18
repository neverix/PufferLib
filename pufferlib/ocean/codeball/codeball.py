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
    def __init__(self, num_envs=2, n_robots=6, n_nitros=0, max_steps=2000,
                #  reward_mul=20.0,
                 reward_mul=1.0,
                 frame_skip=5, buf=None, render_mode='human'):
        self.num_envs = num_envs
        self.num_agents = n_robots * num_envs
        self.n_robots = n_robots
        self.n_nitros = n_nitros
        self.max_steps = max_steps
        self.render_mode = render_mode

        # Define observation and action spaces
        self.single_observation_space = gym.spaces.Box(
            low=-1, high=1,
            shape=((self.n_robots + 2), 9,)
            , dtype=np.float32)
        # self.single_action_space = gym.spaces.MultiDiscrete([8])
        self.single_action_space = gym.spaces.Discrete(8)
        # self.single_action_space = gym.spaces.MultiDiscrete([8, 2])

        super().__init__(buf=buf)

        self.c_envs = CyCodeBall(
            self.num_envs, self.n_robots, self.n_nitros, frame_skip, reward_mul, max_steps,
            # np.reshape(self.observations, (-1, self.n_robots + 2, 6)),
            self.observations,
            self.actions, self.rewards, self.terminals, self.truncations,
        )

    def reset(self, seed=None):
        self.c_envs.reset()
        return self.observations, []
    
    def render(self):
        return self.c_envs.render()

    def step(self, actions):
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

    # actions = np.array([[0, 0]])
    # actions = np.array([[0]])
    actions = np.array([0])
    # actions = np.tile(actions, (env.num_envs * env.n_robots, 1))
    actions = np.tile(actions, (env.num_envs * env.n_robots,))
    # actions[:, :] = 3
    # actions[:, :] = 0
    actions[:] = 0
    all_obs = []
    all_rewards = []
    terminals = []
    truncations = []
    for _ in trange(10_000):
        # actions[:, 0] = np.random.randint(0, 8, size=actions.shape[0])
        actions[:] = np.random.randint(0, 8, size=actions.shape[0])
        # actions[::2, 0] = 6 + np.random.randint(0, 3, size=actions.shape[0] // 2)
        # actions[1::2, 0] = 2 + np.random.randint(0, 3, size=actions.shape[0] // 2)
        obs, rewards, terminated, truncated, info = env.step(actions)
        all_obs.append(obs.copy())
        all_rewards.append(rewards.copy())
        terminals.append(terminated.copy())
        truncations.append(truncated.copy())
    obs = np.stack(all_obs)
    if obs.ndim == 3:
        obs = obs.reshape(obs.shape[0], env.num_agents, -1, 6)
    rewards = np.stack(all_rewards)
    terminals = np.stack(terminals)
    truncations = np.stack(truncations)
    from matplotlib import pyplot as plt
    plt.plot(obs[:, 0, 0, 0], label="Robot 0 X")
    plt.plot(obs[:, 0, 0, 1], label="Robot 0 Y")
    plt.plot(obs[:, 0, 0, 2], label="Robot 0 Z")
    plt.plot(obs[:, 0, -1, -6], label="Ball X")
    plt.plot(obs[:, 0, -1, -5], label="Ball Y")
    plt.plot(obs[:, 0, -1, -4], label="Ball Z")
    plt.plot(obs[:, 0, -1, -3], label="Ball vX")
    plt.plot(obs[:, 0, -1, -2], label="Ball vY")
    plt.plot(obs[:, 0, -1, -1], label="Ball vZ")
    plt.plot(rewards[:, 0], label="Rewards")
    plt.plot(terminals[:, 0], label="Terminals")
    plt.plot(truncations[:, 0], label="Truncations")
    plt.legend()
    plt.show()
    print("Rewards:", rewards.sum(0))
    print("Terminated:", terminated)
    print("Truncated:", truncated)
    print("Observations shape:", obs.shape)

    env.close()
