"""Plot the option price as a function of the spot price using the C++ engine.

Build the project first (cmake -S . -B build && cmake --build build), then run:

    python python_tools/plot_from_engine.py
    python python_tools/plot_from_engine.py --style put --sigma 0.3 --save put.png
"""

import argparse
import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt

PROJECT_ROOT = Path(__file__).resolve().parent.parent


def import_engine():
    """Import bs_engine from BS_ENGINE_DIR or the default CMake build directory."""
    build_dir = Path(os.environ.get("BS_ENGINE_DIR", PROJECT_ROOT / "build"))
    for candidate in (build_dir, build_dir / "Release", build_dir / "Debug"):
        sys.path.insert(0, str(candidate))
    try:
        import bs_engine
    except ImportError:
        sys.exit(
            f"Could not import bs_engine from {build_dir}.\n"
            "Build the project first: cmake -S . -B build && cmake --build build"
        )
    return bs_engine


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--style", choices=["call", "put"], default="call")
    parser.add_argument("--K", type=float, default=100.0, help="strike price")
    parser.add_argument("--r", type=float, default=0.05, help="risk-free rate")
    parser.add_argument("--sigma", type=float, default=0.2, help="volatility")
    parser.add_argument("--T", type=float, default=1.0, help="time to maturity in years")
    parser.add_argument("--s-min", type=float, default=50.0, help="lowest spot price")
    parser.add_argument("--s-max", type=float, default=150.0, help="highest spot price")
    parser.add_argument("--points", type=int, default=201, help="number of spot prices")
    parser.add_argument("--save", metavar="FILE", help="save the plot instead of showing it")
    return parser.parse_args()


def main():
    args = parse_args()
    if args.points < 2 or not 0 < args.s_min < args.s_max:
        sys.exit("Need --points >= 2 and 0 < --s-min < --s-max")

    bs_engine = import_engine()
    params = bs_engine.BSParams(S=args.s_min, K=args.K, r=args.r, sigma=args.sigma, T=args.T)

    # Computation happens in C++, Python only collects the results and plots them.
    step = (args.s_max - args.s_min) / (args.points - 1)
    spots = [args.s_min + i * step for i in range(args.points)]
    prices = []
    for s in spots:
        params.S = s
        prices.append(bs_engine.quote(args.style, params).price)

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(spots, prices, label=f"{args.style} price")
    ax.axvline(args.K, color="gray", linestyle="--", linewidth=1, label=f"strike K={args.K:g}")
    ax.set_xlabel("Spot price S")
    ax.set_ylabel(f"{args.style.capitalize()} price")
    ax.set_title(
        f"European {args.style} price vs S  (r={args.r:g}, sigma={args.sigma:g}, T={args.T:g})"
    )
    ax.grid(True)
    ax.legend()
    fig.tight_layout()

    if args.save:
        fig.savefig(args.save, dpi=150)
        print(f"Saved plot to {args.save}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
