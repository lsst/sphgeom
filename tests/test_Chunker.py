#
# LSST Data Management System
# See COPYRIGHT file at the top of the source tree.
#
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the LSST License Statement and
# the GNU General Public License along with this program.  If not,
# see <https://www.lsstcorp.org/LegalNotices/>.
#
from __future__ import absolute_import, division, print_function

import pickle
import unittest

from lsst.sphgeom import Box, Chunker


class ChunkerTestCase(unittest.TestCase):

    def testConstruction(self):
        chunker = Chunker(85, 12)
        self.assertEqual(chunker.numStripes, 85)
        self.assertEqual(chunker.numSubStripesPerStripe, 12)

    def testComparisonOperators(self):
        c = Chunker(85, 12)
        self.assertEqual(c, c)
        self.assertEqual(c, Chunker(85, 12))
        self.assertNotEqual(c, Chunker(85, 10))

    def testIntersecting(self):
        b = Box.fromDegrees(273.6, 30.7, 273.7180105379097, 30.722546655347717)
        c = Chunker(85, 12)
        self.assertEqual(c.getChunksIntersecting(b), [9630, 9631, 9797])
        self.assertEqual(c.getSubChunksIntersecting(b),
                         [(9630, [770]), (9631, [759]), (9797, [11])])

    def testString(self):
        chunker = Chunker(85, 12)
        self.assertEqual(str(chunker), 'Chunker(85, 12)')
        self.assertEqual(repr(chunker), 'Chunker(85, 12)')
        self.assertEqual(chunker, eval(repr(chunker), dict(Chunker=Chunker)))

    def testPickle(self):
        a = Chunker(85, 12)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)


if __name__ == '__main__':
    unittest.main()
