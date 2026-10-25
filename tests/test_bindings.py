"""Tests for the bs_engine Python module.

Build the project first, then run from the repository root:

    python -m unittest discover -s tests -v

Set BS_ENGINE_DIR if the module was built somewhere other than ./build.
"""

import math
import os
import sys
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = Path(os.environ.get("BS_ENGINE_DIR", PROJECT_ROOT / "build"))
for candidate in (BUILD_DIR, BUILD_DIR / "Release", BUILD_DIR / "Debug"):
    sys.path.insert(0, str(candidate))

import bs_engine  # noqa: E402  (needs the sys.path setup above)

ATM = dict(S=100.0, K=100.0, r=0.05, sigma=0.2, T=1.0)


class BindingsTest(unittest.TestCase):
    def test_params_keyword_constructor(self):
        p = bs_engine.BSParams(**ATM)
        self.assertEqual((p.S, p.K, p.r, p.sigma, p.T), (100.0, 100.0, 0.05, 0.2, 1.0))
        self.assertIn("sigma=0.2", repr(p))

    def test_params_attribute_style(self):
        p = bs_engine.BSParams()
        p.S, p.K, p.r, p.sigma, p.T = 100.0, 100.0, 0.05, 0.2, 1.0
        self.assertAlmostEqual(bs_engine.call_quote(p).price, 10.450583572186, places=9)

    def test_call_and_put_quotes(self):
        p = bs_engine.BSParams(**ATM)
        call = bs_engine.quote("call", p)
        put = bs_engine.quote("put", p)
        self.assertAlmostEqual(call.price, 10.450583572186, places=9)
        self.assertAlmostEqual(put.price, 5.573526022257, places=9)
        self.assertAlmostEqual(call.theta, -6.414027546438, places=9)
        self.assertAlmostEqual(put.theta, -1.657880423935, places=9)
        self.assertAlmostEqual(call.delta - put.delta, 1.0, places=12)
        self.assertEqual(call.gamma, put.gamma)
        self.assertEqual(call.vega, put.vega)

    def test_shortcuts_match_quote(self):
        p = bs_engine.BSParams(**ATM)
        self.assertEqual(bs_engine.call_quote(p).price, bs_engine.quote("call", p).price)
        self.assertEqual(bs_engine.put_quote(p).price, bs_engine.quote("put", p).price)

    def test_model_class(self):
        model = bs_engine.BlackScholes(bs_engine.BSParams(**ATM))
        parity = model.call_price() - model.put_price()
        self.assertAlmostEqual(parity, 100.0 - 100.0 * math.exp(-0.05), places=10)
        self.assertEqual(model.params.K, 100.0)

    def test_invalid_input_raises_value_error(self):
        with self.assertRaises(ValueError):
            bs_engine.call_quote(bs_engine.BSParams(S=100.0, K=100.0, r=0.05, sigma=0.0, T=1.0))
        with self.assertRaises(ValueError):
            bs_engine.quote("straddle", bs_engine.BSParams(**ATM))


if __name__ == "__main__":
    unittest.main()
