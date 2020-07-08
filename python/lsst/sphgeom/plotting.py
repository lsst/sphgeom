from __future__ import annotations

from typing import Iterable, Optional

from mpl_toolkits import mplot3d
from matplotlib import pyplot
from matplotlib.colors import to_rgba
from matplotlib.figure import Figure
from . import Box3d, ConvexPolygon, Pixelization, RangeSet


def setAxesLimitsFromBox3d(bbox: Box3d, axis: mplot3d.Axes3D):
    axis.set_xlim(bbox.x().getA(), bbox.x().getB())
    axis.set_ylim(bbox.y().getA(), bbox.y().getB())
    axis.set_zlim(bbox.z().getA(), bbox.z().getB())


def convertConvexPolygons(polygons: Iterable[ConvexPolygon], *, bbox: Optional[Box3d] = None, **kwargs
                          ) -> mplot3d.art3d.Poly3DCollection:
    vertices = []
    for p in polygons:
        if bbox is not None:
            bbox.expandTo(p.getBoundingBox3d())
        vertices.append(p.getVertices())
    return mplot3d.art3d.Poly3DCollection(vertices, **kwargs)


def convertRangeSet(pixelization: Pixelization, ranges: RangeSet, *, bbox: Optional[Box3d] = None, **kwargs
                    ) -> mplot3d.art3d.Poly3DCollection:
    def iterPolygons():
        for begin, end in ranges:
            for index in range(begin, end):
                yield pixelization.pixel(index)
    return convertConvexPolygons(iterPolygons(), bbox=bbox, **kwargs)


class PlotContext:

    def __init__(self, figure: Optional[Figure] = None, axes: Optional[mplot3d.Axes3D] = None):
        if figure is None:
            figure = pyplot.figure()
        if axes is None:
            axes = figure.add_subplot(1, 1, 1, projection="3d")
        self.figure = figure
        self.axes = axes
        self.bbox = None

    def __enter__(self) -> PlotContext:
        self.bbox = Box3d()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        setAxesLimitsFromBox3d(self.bbox, self.axes)
        self.bbox = None

    def _fixCollectionColors(self, collection, *, color=None, edgecolor=None, facecolor=None, alpha=None):
        if color is not None:
            color = to_rgba(color, alpha=alpha)
            collection.set_color(color)
        if edgecolor is not None:
            edgecolor = to_rgba(edgecolor, alpha=alpha)
            collection.set_edgecolor(edgecolor)
        if facecolor is not None:
            facecolor = to_rgba(facecolor, alpha=alpha)
            collection.set_facecolor(facecolor)

    def plotPolygons(self, *polygons: ConvexPolygon,
                     color=None, edgecolor=None, facecolor=None, alpha=None,
                     **kwargs):
        collection = convertConvexPolygons(polygons, bbox=self.bbox)
        self._fixCollectionColors(collection, color=color, edgecolor=edgecolor, facecolor=facecolor,
                                  alpha=alpha)
        self.axes.add_collection(collection)

    def plotRangeSet(self, pixelization: Pixelization, ranges: RangeSet, *,
                     color=None, edgecolor=None, facecolor=None, alpha=None,
                     **kwargs):
        collection = convertRangeSet(pixelization, ranges, bbox=self.bbox, **kwargs)
        self._fixCollectionColors(collection, color=color, edgecolor=edgecolor, facecolor=facecolor,
                                  alpha=alpha)
        self.axes.add_collection(collection)
