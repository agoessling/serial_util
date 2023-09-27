import unittest

from crc import py_crc


class TestBlock(unittest.TestCase):

  def test_pass(self):
    print(py_crc._lib.__dict__)
    crc = py_crc.Crc(16, 'kCrc16KermitInfo')
    print(crc.info)


if __name__ == '__main__':
  unittest.main()
