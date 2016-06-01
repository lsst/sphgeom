HTM Indexing
============

Overview                        {#htm-overview}
========

The Hierarchical Triangular Mesh (HTM) is described in the work of A. Szalay,
T. Budavari, G. Fekete at The Johns Hopkins University, and Jim Gray,
Microsoft Research. See in particular the following paper:

> "Indexing the Sphere with the Hierarchical Triangular Mesh"
> Szalay, Alexander S.; Gray, Jim; Fekete, George; Kunszt, Peter Z.;
> Kukol, Peter; Thakar, Ani
> 2007, Arxiv e-prints
>
> http://arxiv.org/abs/cs/0701164
> http://adsabs.harvard.edu/abs/2007cs........1164S

To summarize very briefly: HTM partitions the unit sphere into 8 root triangles
by splitting it with the planes x = 0, y = 0, and z = 0. A triangle is subdivided
into 4 children by connecting the midpoints of its edges with geodesics, and each
root triangle is recursively subdivided to a fixed subdivision level to obtain a
pixelization of the sphere. The root triangles are assigned indexes 8-15, and the
children of a triangle with index I are assigned indexes [4*I, 4*I + 4).

Further References              {#htm-references}
==================

> Budavári, Tamás; Szalay, Alexander S.; Fekete, György
> "Searchable Sky Coverage of Astronomical Observations:
> Footprints and Exposures"
> Publications of the Astronomical Society of Pacific,
> Volume 122, Issue 897, pp. 1375-1388 (2010).
>
> http://adsabs.harvard.edu/abs/2010PASP..122.1375B

> http://voservices.net/spherical/
