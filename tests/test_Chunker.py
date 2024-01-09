# This file is part of sphgeom.
#
# Developed for the LSST Data Management System.
# This product includes software developed by the LSST Project
# (http://www.lsst.org).
# See the COPYRIGHT file at the top-level directory of this distribution
# for details of code ownership.
#
# This software is dual licensed under the GNU General Public License and also
# under a 3-clause BSD license. Recipients may choose which of these licenses
# to use; please see the files gpl-3.0.txt and/or bsd_license.txt,
# respectively.  If you choose the GPL option then the following text applies
# (but note that there is still no warranty even if you opt for BSD instead):
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
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import pickle
import unittest

from lsst.sphgeom import Box, Chunker


class ChunkerTestCase(unittest.TestCase):
    """Test Chunker."""

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
        self.assertEqual(c.getSubChunksIntersecting(b), [(9630, [770]), (9631, [759]), (9797, [11])])

    def testString(self):
        chunker = Chunker(85, 12)
        self.assertEqual(str(chunker), "Chunker(85, 12)")
        self.assertEqual(repr(chunker), "Chunker(85, 12)")
        self.assertEqual(chunker, eval(repr(chunker), {"Chunker": Chunker}))

    def testPickle(self):
        a = Chunker(85, 12)
        b = pickle.loads(pickle.dumps(a))
        self.assertEqual(a, b)

    def testChunkBoundingBox(self):
        chunker = Chunker(200, 5)
        chunk_id = 3645
        stripe = chunker.getStripe(chunk_id)
        chunk_in_stripe = chunker.getChunk(chunk_id, stripe)
        bbox = chunker.getChunkBoundingBox(stripe, chunk_in_stripe)
        sbbox = chunker.getSubChunkBoundingBox(0, 0)
        self.assertEqual(stripe, 9)
        self.assertEqual(chunk_in_stripe, 45)
        b = Box.fromRadians(5.048988193233824, -1.4294246573883558, 5.1611879309330035, -1.413716694110407)
        self.assertAlmostEqual(bbox, b)
        sb = Box.fromRadians(0.0, -1.5707963267948966, 6.283185307179586, -1.5676547341363067)
        self.assertAlmostEqual(sbbox, sb)


if __name__ == "__main__":
    unittest.main()
