sphgeom v30.0.8 (2026-06-08)
============================

New Features
------------

- Added ``Region.to_ivoa_stcs(frame="ICRS")`` for serialising regions as IVOA STC-S strings.
  Supports ``Circle``, ``ConvexPolygon``, ``Ellipse``, ``UnionRegion``, and ``IntersectionRegion``.
  ``Box`` raises `NotImplementedError` because STC-S has no latitude-parallel range region. (`DM-53569 <https://rubinobs.atlassian.net/browse/DM-53569>`_)


sphgeom v30.0.6 (2026-04-07)
============================

Bug Fixes
---------

- Fixed crash that could occur when a nearly degenerate polygon was used with healpix pixelization. (`DM-53933 <https://rubinobs.atlassian.net/browse/DM-53933>`_)
- Fixed empty envelope that could occur when a small region was used with healpix pixelization. (`DM-54274 <https://rubinobs.atlassian.net/browse/DM-54274>`_)
