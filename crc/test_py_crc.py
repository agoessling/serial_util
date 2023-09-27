import unittest

from crc import py_crc


class TestCrc(unittest.TestCase):
  def setUp(self):
    self.test_input = b'123456789'
    self.crcs = [
        (py_crc.Crc(8, 'kCrc8DarcInfo'), 0x15),
        (py_crc.Crc(8, 'kCrc8ICodeInfo'), 0x7E),
        (py_crc.Crc(16, 'kCrc16KermitInfo'), 0x2189),
        (py_crc.Crc(16, 'kCrc16CcittFalseInfo'), 0x29B1),
        (py_crc.Crc(32, 'kCrc32Info'), 0xCBF43926),
        (py_crc.Crc(32, 'kCrc32Mpeg2Info'), 0x0376E6E7),
    ]

  def test_block(self):
    for crc, value in self.crcs:
      with self.subTest(crc=crc):
        self.assertEqual(crc.block(self.test_input), value)

  def test_byte(self):
    for crc, value in self.crcs:
      with self.subTest(crc=crc):
        for i in range(4):
          crc.update(i)

        crc.reset()

        for b in self.test_input[:-1]:
          self.assertNotEqual(crc.update(b), value)

        self.assertEqual(crc.update(self.test_input[-1]), value)

  def test_seq(self):
    for crc, value in self.crcs:
      with self.subTest(crc=crc):
        for i in range(4):
          crc.update(i)

        crc.reset()

        self.assertNotEqual(crc.update(self.test_input[:4]), value)
        self.assertEqual(crc.update(self.test_input[4:]), value)


if __name__ == '__main__':
  unittest.main()
