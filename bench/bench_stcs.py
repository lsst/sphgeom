"""Throughput benchmark for ``ConvexPolygon.to_ivoa_stcs``.

Run with: ``python bench/bench_stcs.py``.
"""

from __future__ import annotations

import math
import time

from lsst.sphgeom import Angle, Circle, ConvexPolygon, LonLat, UnionRegion, UnitVector3d


def make_polygon(n_vertices: int, radius_deg: float = 1.0) -> ConvexPolygon:
    """Build a regular n-gon centred near (0, 0)."""
    r = math.radians(radius_deg)
    vertices = []
    for i in range(n_vertices):
        theta = 2.0 * math.pi * i / n_vertices
        lon = math.degrees(r * math.cos(theta))
        lat = math.degrees(r * math.sin(theta))
        vertices.append(UnitVector3d(LonLat.fromDegrees(lon, lat)))
    return ConvexPolygon(vertices)


def time_calls(fn, target_seconds: float = 2.0, warmup: int = 1000) -> tuple[int, float]:
    """Run ``fn`` for ~``target_seconds`` and return (calls, elapsed)."""
    for _ in range(warmup):
        fn()
    n = 0
    t0 = time.perf_counter()
    deadline = t0 + target_seconds
    # Process in batches of 1000 to amortise the clock check.
    while time.perf_counter() < deadline:
        for _ in range(1000):
            fn()
        n += 1000
    elapsed = time.perf_counter() - t0
    return n, elapsed


def main() -> None:
    """Run the STC-S throughput benchmark and print a results table."""
    print(f"{'workload':<40} {'ops/sec':>14} {'us/op':>10}")
    print("-" * 66)

    for n in (4, 8, 16, 64, 256):
        poly = make_polygon(n)
        ops, elapsed = time_calls(poly.to_ivoa_stcs)
        rate = ops / elapsed
        print(f"ConvexPolygon[{n:>3}].to_ivoa_stcs()           {rate:>14,.0f} {1e6 / rate:>10.2f}")

    # Body helper alone (no frame insertion) — shows frame overhead.
    poly = make_polygon(64)
    ops, elapsed = time_calls(poly._ivoa_stcs_body)
    rate = ops / elapsed
    print(f"ConvexPolygon[ 64]._ivoa_stcs_body()         {rate:>14,.0f} {1e6 / rate:>10.2f}")

    # Circle for scale.
    circle = Circle(UnitVector3d(LonLat.fromDegrees(0.0, 0.0)), Angle.fromDegrees(1.0))
    ops, elapsed = time_calls(circle.to_ivoa_stcs)
    rate = ops / elapsed
    print(f"Circle.to_ivoa_stcs()                        {rate:>14,.0f} {1e6 / rate:>10.2f}")

    # Union of two 64-gon polygons (recursive body call path).
    u = UnionRegion(make_polygon(64), make_polygon(64))
    ops, elapsed = time_calls(u.to_ivoa_stcs)
    rate = ops / elapsed
    print(f"UnionRegion(Poly64, Poly64).to_ivoa_stcs()   {rate:>14,.0f} {1e6 / rate:>10.2f}")


if __name__ == "__main__":
    main()
