Q3C Indexing
============

Overview          				{#q3c-overview}
========

Q3C is described by the following paper:

> Koposov, S., Bartunov, O., Jul. 2006. Q3C, Quad Tree Cube –
> The new sky-indexing concept for huge astronomical catalogues and
> its realization for main astronomical queries (cone search and xmatch)
> in Open Source Database PostgreSQL. In: Gabriel, C., Arviset, C.,
> Ponz, D., Enrique, S. (Eds.), Astronomical Data Analysis Software
> and Systems XV. Vol. 351 of Astronomical Society of the Pacific
> Conference Series. p. 735.

available online [here](http://adsabs.harvard.edu/abs/2006ASPC..351..735K).
The authors also provide [an implementation](https://github.com/segasai/q3c).

Two kinds of Q3C indexes can be produced by this library. The first kind,
"original" Q3C indexes, are compatible with those produced by the Q3C
PostgreSQL extension, and are provided by the
[Q3cPixelization](\ref lsst::sphgeom::Q3cPixelization) class.

The second kind, "modified" Q3C indexes, are produced according to a
similar scheme described below. They are provided by the
[Mq3cPixelization](\ref lsst::sphgeom::Mq3cPixelization) class.

To obtain the index of unit vector P, both the original and modified
schemes start by centrally projecting P onto the faces of the cube [-1,1]³.
Each of the 6 cube faces is assigned a distinct face number, and a local
coordinate system (u, v). A grid of pixels is overlaid on each cube face,
and pixels are labeled according to a space filling curve. The index is
obtained by combining the label of the pixel containing the face coordinates
with the face number.

Original Indexes         		{#q3c-original}
================

In the original Q3C scheme, the cube faces are numbered 0-5, where:

- Face 0 corresponds to the plane z =  1, (u, v) = ( y, -x)
- Face 1 corresponds to the plane x =  1, (u, v) = ( y,  z)
- Face 2 corresponds to the plane y =  1, (u, v) = (-x,  z)
- Face 3 corresponds to the plane x = -1, (u, v) = (-y,  z)
- Face 4 corresponds to the plane y = -1, (u, v) = ( x,  z)
- Face 5 corresponds to the plane z = -1, (u, v) = ( y,  x)

Face coordinates (u, v) are converted to grid coordinates (s, t) using a
linear transformation. Grid coordinates s and t are both in [0, n), where
n = 2ᵐ is the Q3C grid resolution. The Q3C index is formed by
concatenating the 3 bit face number and the 2m bit Morton index of (s,t).

Modified Indexes				{#q3c-modified}
================

In the modified scheme, cube faces are numbered 10-15, where:

- Face 10 corresponds to the plane z = -1, (u, v) = ( x,  y)
- Face 11 corresponds to the plane x =  1, (u, v) = ( y,  z)
- Face 12 corresponds to the plane y =  1, (u, v) = ( z, -x)
- Face 13 corresponds to the plane z =  1, (u, v) = (-x, -y)
- Face 14 corresponds to the plane x = -1, (u, v) = (-y, -z)
- Face 15 corresponds to the plane y = -1, (u, v) = (-z,  x)

The transformation to grid coordinates (s, t), where s and t are
both in [0, n) and n = 2ᵐ is the grid resolution, is non-linear.

The reason is that if each cube face is overlaid with a uniform grid
(as in the original Q3C scheme), then the area of grid pixels (projected
back onto the sphere) varies by a factor of about 5.2. The largest pixels
are in the face centers (where the face is tangent to the unit sphere),
and the smallest are in the face corners.

To get more uniform pixel area, we use an idea from the Google S2
library. In particular, we apply a separable transformation to face
coordinates before the linear transformation to grid coordinates. This
allows pixel area to be adjusted by changing edge spacing. Edges remain
geodesic, so that pixels are convex spherical polygons as in the original
scheme.

One suitable transformation is obtained by noticing that the central
projection of (z, x) = (cos θ, sin θ), where -π/4 ≤ θ ≤ π/4, onto the
line z = 1 is (1, tan θ) (taking the origin as the center of projection).
So, applying f(x) = 4 arctan x / π to each face coordinate will produce
pixel edges that subtend more uniform angles. This reduces pixel area
variation down to a factor of about 1.414.

However, computing arc-tangents is relatively expensive. Instead, the
modified Q3C scheme applies f(x) = x(4 - |x|)/3, a decent quadratic
approximation to 4 arctan x / π, to both u and v before applying a linear
transformation to grid coordinates (s, t). This cuts area variation down
to a factor of about 1.56. f(x) was arrived at by picking a quadratic
polynomial satisfying f(0) = 0 and f(1) = 1 with coefficients close to
the best minimax rational approximation of degree 2 for 4 arctan x / π
on the interval [0, 1]:

    -0.00312224879 + (1.357648680 - 0.3514041823 x) x

(computed with Maple using the Remez algorithm). The constraint
f(-x) = -f(x) (following the symmetry of arctan) is used to extend the
domain to [-1, 1]. Though better approximations likely exist, this one
is already noticeably better than the one used by the S2 library, which
results in a pixel area variation factor of about 2.1. Note that S2
employs a quadratic approximation for tan rather than arctan, which means
that grid to face coordinate conversion is faster since it does not
involve any square roots - this may have been more important to the S2
authors than pixel area variation.

Finally, the modified Q3C index I of a point P is formed by concatenating
the 4 bit face number of P with the 2m bit Hilbert index of its grid
coordinates (s, t). The Hilbert curve is preferred over the Morton curve
used in the original scheme because it has better spatial locality - see:

> Analysis of the Clustering Properties of the Hilbert Space-Filling Curve
> Moon, B., Jagadish, H. V., Faloutsos, C., & Saltz, J. H.
> IEEE Trans. on Knowledge and Data Engineering,
> vol. 13, no. 1, pp. 124-141, 2001

for an analysis. Because the MSB of the face number is always 1, the
grid resolution is derivable from the index of the MSB of I.
