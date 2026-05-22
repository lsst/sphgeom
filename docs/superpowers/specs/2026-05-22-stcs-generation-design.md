# STC-S string generation for sphgeom regions

## Motivation

`lsst.sphgeom.Region` already exposes `to_ivoa_pos()` / `from_ivoa_pos()` for
the IVOA SIAv2 POS string format. We need an analogous one-way export to the
IVOA STC-S string format so that sphgeom regions can be passed to tools that
consume STC-S — notably the `stilts mocshape` command, which builds a MOC from
an STC-S region description.

POS is a tiny SIAv2 syntax limited to `CIRCLE`, `RANGE`, and `POLYGON`. STC-S
is a richer IVOA Note (1.33, 2009) covering `Circle`, `Polygon`, `Ellipse`,
`Box`, plus the `Union`, `Intersection`, `Difference`, and `Not` set
operators, all qualified by a coordinate frame keyword. Reference:
http://www.ivoa.net/Documents/Notes/STC-S/20091030/NOTE-STC-S-1.33-20091030.html

## Scope

In scope:

- One-way generation only: `Region.to_ivoa_stcs(frame: str = "ICRS") -> str`.
- Coverage: `Circle`, `ConvexPolygon`, `Ellipse`, `UnionRegion`,
  `IntersectionRegion`. (`CompoundRegion` is the abstract base.)
- `Box` deliberately not supported (see below).

Out of scope:

- Parsing (no `Region.from_ivoa_stcs`). The user-facing need is export to
  `stilts mocshape`. A parser can be added later if a consumer requires it.
- `Difference` / `Not` operators — sphgeom has no corresponding region
  classes.
- STC-S features beyond the geometric region grammar (e.g. time, spectral,
  redshift, position errors, fillfactor, refpos beyond the frame keyword).

## API

A new method on `Region`, mirroring the placement of `to_ivoa_pos`:

```python
def to_ivoa_stcs(self, frame: str = "ICRS") -> str:
    """Represent the region as an IVOA STC-S string."""
```

The base `Region.to_ivoa_stcs` raises `NotImplementedError`, matching the
pattern used by `to_ivoa_pos`. Concrete subclasses override it.

The `frame` argument is emitted verbatim as the STC-S frame keyword. No
validation is performed; STC-S consumers reject unknown frames. Default is
`"ICRS"` because sphgeom is frame-agnostic and ICRS is the de-facto convention
in LSST data products.

### The frame-once rule and the body helper

STC-S compound regions emit the frame keyword *only at the outermost level*:

```
Union ICRS ( Circle 180 10 20 Polygon 100 0 110 0 110 10 100 10 )
```

To support this, each region class also defines a private helper:

```python
def _ivoa_stcs_body(self) -> str:
```

which returns the STC-S body of that region **without** the frame keyword.
`to_ivoa_stcs(frame)` is then implemented uniformly as inserting `frame` after
the leading shape/operator name in the body. Compound regions call
`_ivoa_stcs_body()` recursively on their operands.

Examples:

- `Circle._ivoa_stcs_body()` → `"Circle 180.0 10.0 2.0"`
- `Circle.to_ivoa_stcs("ICRS")` → `"Circle ICRS 180.0 10.0 2.0"`
- `UnionRegion(...)._ivoa_stcs_body()` → `"Union ( Circle 180.0 10.0 2.0 Circle 190.0 20.0 1.0 )"`
- `UnionRegion(...).to_ivoa_stcs("ICRS")` → `"Union ICRS ( Circle 180.0 10.0 2.0 Circle 190.0 20.0 1.0 )"`

The `_` prefix marks the helper as not part of the public API.

## Per-region implementations

All implementations live in `python/lsst/sphgeom/_continue_class.py` next to
the existing `to_ivoa_pos` overrides.

### Circle

```
Circle <lon_deg> <lat_deg> <radius_deg>
```

Body: from `getCenter()` (→ `LonLat`) and `getOpeningAngle()`. Numbers
formatted with Python's default `f"{x}"`, matching `to_ivoa_pos`.

### ConvexPolygon

```
Polygon <lon1> <lat1> <lon2> <lat2> ...
```

Body: from `getVertices()` mapped through `LonLat`. Vertex order is preserved
as sphgeom returns it; sphgeom maintains a consistent winding.

### Box

`to_ivoa_stcs` raises `NotImplementedError`, with a message of the form:

> "Box cannot be converted to STC-S directly because STC-S has no
> latitude-parallel range region; build a ConvexPolygon from this Box if an
> STC-S representation is required."

Rationale:

- STC-S has no `RANGE` equivalent. STC-S `Box` is *center + x-size + y-size
  with great-circle sides*, semantically different from `sphgeom.Box` (lat
  parallels).
- A `Box` could be emitted as a densified `Polygon`, but the densification
  step is arbitrary, the result has lots of vertices the caller didn't ask
  for, and edge cases (full-longitude boxes, pole-containing boxes) make the
  output potentially misleading.
- Failing fast makes the limitation explicit and forces the caller to make
  the conversion decision.

### Ellipse

```
Ellipse <lon> <lat> <alpha_deg> <beta_deg> <position_angle_deg>
```

Where:

- `<lon>`, `<lat>`: `LonLat(self.getCenter())`.
- `<alpha_deg>`: `self.getAlpha().asDegrees()` (first semi-axis).
- `<beta_deg>`: `self.getBeta().asDegrees()` (second semi-axis).
- `<position_angle_deg>`: position angle of the first axis, measured east of
  north at the center, in degrees.

sphgeom does not expose the position angle directly. It can be computed from
`getTransformMatrix()`: row 2 is the center unit vector, row 0 is the first
local axis direction. The PA is the angle between local north (the projection
of `(0, 0, 1)` onto the tangent plane at the center) and that axis,
measured east-of-north. The implementation will use the matrix rows directly
to avoid recomputing tangent-plane bases.

Degenerate cases:

- `isEmpty()`, `isFull()`, `isGreatCircle()` → raise `ValueError`. None has a
  meaningful STC-S `Ellipse` representation.

### UnionRegion

Body:

```
Union ( <body_of_operand_0> <body_of_operand_1> )
```

Implementation calls `_ivoa_stcs_body()` on `cloneOperand(0)` and
`cloneOperand(1)`. STC-S permits ≥ 2 operands; sphgeom `CompoundRegion` is
strictly binary, so we always emit exactly two. Deeper unions are expressed
through nesting — that is preserved naturally because nested compound regions
re-emit the operator keyword without a frame.

If any operand's `_ivoa_stcs_body()` raises (e.g. it contains a `Box`), the
exception propagates unchanged. Same failure mode as today's `to_ivoa_pos`.

### IntersectionRegion

Identical structure with the `Intersection` keyword.

## Number formatting

Use `f"{x}"` to match `to_ivoa_pos` exactly. Round-tripping is not a goal
(generation only). Tests compare numbers with `assertAlmostEqual` against
expected values, the same approach used in `test_ivoa.py`.

## Testing

A new file `tests/test_stcs.py` parallel to `tests/test_ivoa.py`:

- `test_circle`: round-trip a known circle, compare with float-tolerance.
- `test_polygon`: round-trip a four-vertex polygon.
- `test_ellipse`: build an `Ellipse` with known center, alpha, beta,
  orientation; verify the emitted PA matches the input orientation. Cover the
  PA=0, 45, 90 cases. Verify `ValueError` is raised for empty / full /
  great-circle ellipses.
- `test_box_not_supported`: assert `NotImplementedError` raised by
  `Box.to_ivoa_stcs()`.
- `test_union`: union of two circles, check exact text. Nested union of a
  union and a polygon, check the inner `Union` re-emits without a frame.
- `test_intersection`: intersection of a circle and a polygon, check exact
  text.
- `test_compound_with_unsupported_operand`: a `UnionRegion` containing a
  `Box` raises `NotImplementedError`.
- `test_frame_argument`: passing `frame="GALACTIC"` produces the expected
  keyword in the output, including for nested compound regions (frame
  appears once at the outermost level only).

## Documentation

- Method docstring on `Region.to_ivoa_stcs` includes a pointer to the STC-S
  Note URL and the supported subset.
- A changelog fragment under `doc/changes/` (the project uses towncrier; the
  recent commits show this pattern). The fragment notes the new
  `to_ivoa_stcs` method, the supported region types, and that `Box` is
  intentionally not supported.

## Alternatives considered

- **Densify `Box` to a `Polygon` with a fixed step**: rejected. Picking a
  step size is arbitrary; caller has no control; pole/wraparound edge cases
  are subtle.
- **Add `from_ivoa_stcs` parser in the same change**: deferred. The user need
  is one-way export to `stilts mocshape`. An STC-S parser is non-trivial
  (full grammar covers temporal/spectral metadata even if we ignore them);
  scoping that as a separate ticket keeps this change small.
- **Hardcode `frame="ICRS"`**: rejected in favour of a parameter. Cost is
  negligible (one optional kwarg) and it lets callers who know their data is
  in another frame produce correct STC-S without post-processing.
