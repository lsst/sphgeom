from typing import Iterable, Tuple

from mpl_toolkits import mplot3d
from .. import Box3d, ConvexPolygon, Pixelization, RangeSet


def setAxesLimitsFromBox3d(bbox: Box3d, axis: mplot3d.Axes3D):
    axis.set_xlim(bbox.x().getA(), bbox.x.getB())
    axis.set_ylim(bbox.y().getA(), bbox.y.getB())
    axis.set_zlim(bbox.z().getA(), bbox.z.getB())


def convertConvexPolygons(polygons: Iterable[ConvexPolygon]) -> Tuple[mplot3d.art3d.Poly3DCollection, Box3d]:
    bbox = Box3d()
    vertices = []
    for p in polygons:
        bbox.expandTo(p.getBoundingBox3d())
        vertices.append(p.getVertices())
    return mplot3d.art3d.Poly3DCollection(vertices), bbox


def convertRangeSet(pixelization: Pixelization, ranges: RangeSet
                    ) -> Tuple[mplot3d.art3d.Poly3DCollection, Box3d]:
    def iterPolygons():
        for begin, end in ranges:
            for index in range(begin, end):
                yield pixelization.pixel(index)
    return convertConvexPolygons(iterPolygons())
