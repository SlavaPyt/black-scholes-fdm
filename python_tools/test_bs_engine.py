import sys
import mathplotlib.pyplot as plt
from pathlib import Path

project_root = Path(__file__).resolve().parent.parent
build_dir = project_root / "build"
sys.path.insert(0, str(build_dir))

import bs_engine

p = bs_engine.BSParams()
p.S = 100.0
p.K = 100.0
p.r = 0.05
p.sigma = 0.2
p.T = 1.0

q = bs_engine.call_quote(p)

print("price =", q.price)
print("delta =", q.delta)
print("gamma =", q.gamma)
print("vega  =", q.vega)
print("theta =", q.theta)