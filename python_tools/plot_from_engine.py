import sys
import matplotlib.pyplot as plt
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

S_values =[ ]
price_values = [ ]

for s in range(50,151):
    p.S = float(s)#because fractional number
    q = bs_engine.call_quote(p)
    S_values.append(s)
    price_values.append(q.price)

plt.plot(S_values, price_values)
plt.xlabel("S")
plt.ylabel("Call price")
plt.title("Call price vs S")
plt.grid(True)
plt.show()