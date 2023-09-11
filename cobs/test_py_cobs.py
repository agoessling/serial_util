import unittest

from cobs import py_cobs


class TestLengths(unittest.TestCase):

  def test_encoded_len(self):
    self.assertEqual(2, py_cobs.max_encode_len(-1))
    self.assertEqual(2, py_cobs.max_encode_len(0))
    self.assertEqual(3, py_cobs.max_encode_len(1))
    self.assertEqual(256, py_cobs.max_encode_len(254))
    self.assertEqual(258, py_cobs.max_encode_len(255))
    self.assertEqual(511, py_cobs.max_encode_len(508))
    self.assertEqual(513, py_cobs.max_encode_len(509))

  def test_decoded_len(self):
    self.assertEqual(0, py_cobs.max_decode_len(-1))
    self.assertEqual(0, py_cobs.max_decode_len(0))
    self.assertEqual(0, py_cobs.max_decode_len(1))
    self.assertEqual(0, py_cobs.max_decode_len(2))
    self.assertEqual(1, py_cobs.max_decode_len(3))
    self.assertEqual(1000, py_cobs.max_decode_len(1002))


class TestEncodeDecode(unittest.TestCase):

  def setUp(self):
    self.vectors = [
        ([], [0x01, 0x00]),
        ([0x00], [0x01, 0x01, 0x00]),
        ([0x00, 0x00], [0x01, 0x01, 0x01, 0x00]),
        ([0x11, 0x22, 0x00, 0x33], [0x03, 0x11, 0x22, 0x02, 0x33, 0x00]),
        ([0x11, 0x22, 0x33, 0x44], [0x05, 0x11, 0x22, 0x33, 0x44, 0x00]),
    ]

    input = [(i + 1) % 256 for i in range(254)]
    output = [i % 256 for i in range(256)]
    output[0] = 0xFF
    output[255] = 0x00
    self.vectors.append((input, output))

    input = [i % 256 for i in range(255)]
    output = [(i - 1) % 256 for i in range(257)]
    output[0] = 0x01
    output[1] = 0xFF
    output[256] = 0x00
    self.vectors.append((input, output))

    input = [(i + 1) % 256 for i in range(255)]
    output = [i % 256 for i in range(258)]
    output[0] = 0xFF
    output[255] = 0x02
    output[256] = 0xFF
    output[257] = 0x00
    self.vectors.append((input, output))

    input = [(i + 2) % 256 for i in range(255)]
    output = [(i + 1) % 256 for i in range(258)]
    output[0] = 0xFF
    output[255] = 0x01
    output[256] = 0x01
    output[257] = 0x00
    self.vectors.append((input, output))

    input = [(i + 3) % 256 for i in range(255)]
    output = [(i + 2) % 256 for i in range(257)]
    output[0] = 0xFE
    output[254] = 0x02
    output[255] = 0x01
    output[256] = 0x00
    self.vectors.append((input, output))

  def test_encode(self):
    for i, (decoded, encoded) in enumerate(self.vectors):
      with self.subTest(vector=i):
        self.assertSequenceEqual(bytes(encoded), py_cobs.encode(bytes(decoded)))

  def test_encoder(self):
    encoder = py_cobs.Encoder(py_cobs.max_encode_len(1000))
    self.assertRaises(RuntimeError, encoder.encode, bytes(1001))

    encoder = py_cobs.Encoder(py_cobs.max_encode_len(1000))
    encoder.encode(bytes(1000))
    encoder.reset()

    for i, (decoded, encoded) in enumerate(self.vectors):
      with self.subTest(vector=i):
        BLOCK_SIZE = 25
        j = 0
        while j < len(decoded):
          encoder.encode(bytes(decoded[j:j + BLOCK_SIZE]))
          j += BLOCK_SIZE
        self.assertSequenceEqual(bytes(encoded), encoder.get())

  def test_decode(self):
    for i, (decoded, encoded) in enumerate(self.vectors):
      with self.subTest(vector=i):
        status, actual = py_cobs.decode(bytes(encoded))
        self.assertEqual(py_cobs.Status.FrameAvailable, status)
        self.assertIsNotNone(actual)
        self.assertSequenceEqual(bytes(decoded), actual)  # type:ignore

  def test_decode_incomplete_frame(self):
    encoded = bytes([0x01, 0x01])
    status, actual = py_cobs.decode(encoded)
    self.assertEqual(py_cobs.Status.IncompleteFrame, status)
    self.assertIsNone(actual)

  def test_decoder_overflow(self):
    LEN = 3
    decoder = py_cobs.Decoder(LEN)

    for _ in range(LEN + 1):
      status, buf = decoder.decode(0xFF)
      self.assertEqual(py_cobs.Status.Processing, status)
      self.assertIsNone(buf)

    status, buf = decoder.decode(0xFF)
    self.assertEqual(py_cobs.Status.Overflow, status)
    self.assertIsNone(buf)

  def test_decoder_malformed_frame(self):
    decoder = py_cobs.Decoder(32)
    encoded = bytes([0xAA, 0xFF, 0xFF, 0xFF, 0x00])

    for b in encoded[:-1]:
      status, buf = decoder.decode(b)
      self.assertEqual(py_cobs.Status.Processing, status)
      self.assertIsNone(buf)

    status, buf = decoder.decode(encoded[-1])
    self.assertEqual(py_cobs.Status.MalformedFrame, status)
    self.assertIsNone(buf)

  def test_decoder_success(self):
    decoder = py_cobs.Decoder(1024)

    for i, (decoded, encoded) in enumerate(self.vectors):
      with self.subTest(vector=i):
        for b in encoded[:-1]:
          status, buf = decoder.decode(b)
          self.assertEqual(py_cobs.Status.Processing, status)
          self.assertIsNone(buf)

        status, buf = decoder.decode(encoded[-1])
        self.assertEqual(py_cobs.Status.FrameAvailable, status)
        self.assertIsNotNone(buf)
        self.assertSequenceEqual(bytes(decoded), buf)  # type: ignore


if __name__ == '__main__':
  unittest.main()
