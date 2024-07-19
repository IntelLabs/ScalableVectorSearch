#
# Copyright (C) 2023-present, Intel Corporation
#
# You can redistribute and/or modify this software under the terms of the
# GNU Affero General Public License version 3.
#
# You should have received a copy of the GNU Affero General Public License
# version 3 along with this software. If not, see
# <https://www.gnu.org/licenses/agpl-3.0.en.html>.
#

import unittest
import svs

class MKLTester(unittest.TestCase):
    def test_mkl(self):
        if svs.have_mkl():
            self.assertTrue(svs.mkl_num_threads() is not None)
        else:
            self.assertTrue(svs.mkl_num_threads() is None)
