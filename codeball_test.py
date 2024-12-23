from pufferlib.ocean.torch import MLPPolicy, Recurrent
from pufferlib.ocean.codeball.codeball import CodeBall
from tqdm import trange
import numpy as np
import torch


env = CodeBall(num_envs=1, n_robots=8, is_single=True)
obs, _ = env.reset()


# pol = MLPPolicy(env)
# rnn = Recurrent(env, pol)
wp = None
if wp is None:
    from glob import glob
    wp = sorted(glob("experiments/**/model_*.pt", recursive=True))[-1]
rnn = torch.load(wp, map_location='cpu')
rnn_state = None
torch.set_grad_enabled(False)
for _ in (bar := trange(10_000)):
    obs = torch.from_numpy(obs).float()
    actions, logprob, entropy, value, rnn_state = rnn(obs, rnn_state)
    # logits, value, rnn_state = rnn.policy(obs, rnn_state)
    # actions = logits
    actions = actions.numpy()
    obs, rewards, terminated, truncated, info = env.step(actions)
    bar.set_postfix(val=(value.reshape(-1, 2).mean(0).tolist()))
    if (terminated | truncated).any():
        rnn_state = None
    env.render()
env.close()
