import ctypes
import enum


class Status(enum.IntEnum):
  '''Decode status'''
  Processing = 0
  FrameAvailable = 1
  MalformedFrame = 2
  Overflow = 3
  IncompleteFrame = 4


class _EncodeState(ctypes.Structure):
  _fields_ = [
      ('encoded', ctypes.POINTER(ctypes.c_uint8)),
      ('len', ctypes.c_size_t),
      ('_delim_ptr', ctypes.POINTER(ctypes.c_uint8)),
      ('_write_ptr', ctypes.POINTER(ctypes.c_uint8)),
      ('_delim_cnt', ctypes.c_uint8),
  ]


class _DecodeState(ctypes.Structure):
  _fields_ = [
      ('decoded', ctypes.POINTER(ctypes.c_uint8)),
      ('len', ctypes.c_size_t),
      ('_write_ptr', ctypes.POINTER(ctypes.c_uint8)),
      ('_end_ptr', ctypes.POINTER(ctypes.c_uint8)),
      ('_delim_cnt', ctypes.c_uint8),
      ('_mandatory_delim', ctypes.c_bool),
  ]


class _CobsStatus(ctypes.c_int):

  def enum(self) -> Status:
    return Status(self.value)


_lib = ctypes.cdll.LoadLibrary('cobs/c_cobs.so')

_lib.CobsEncodeStateInit.argtypes = [ctypes.POINTER(_EncodeState), ctypes.POINTER(ctypes.c_uint8)]
_lib.CobsEncodeStateInit.restype = None

_lib.CobsEncodeBlock.argtypes = [
    ctypes.POINTER(_EncodeState),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.c_bool,
]
_lib.CobsEncodeBlock.restype = None

_lib.CobsEncodeBuffer.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
_lib.CobsEncodeBuffer.restype = ctypes.c_size_t

_lib.CobsDecodeStateInit.argtypes = [
    ctypes.POINTER(_DecodeState),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
_lib.CobsDecodeStateInit.restype = None

_lib.CobsDecodeByte.argtypes = [
    ctypes.POINTER(_DecodeState),
    ctypes.c_uint8,
]
_lib.CobsDecodeByte.restype = _CobsStatus

_lib.CobsDecodeBuffer.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_size_t),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
_lib.CobsDecodeBuffer.restype = _CobsStatus


def max_encode_len(decoded_len: int) -> int:
  '''Maximum encoded length given a decoded length.'''
  if decoded_len > 0:
    return decoded_len + (decoded_len + 253) // 254 + 1
  return 2


def max_decode_len(encoded_len: int) -> int:
  '''Maximum decoded length given an encoded length.'''
  if encoded_len > 1:
    return encoded_len - 2
  return 0


def encode(buf: bytes) -> bytes:
  '''COBS encode bytes'''
  output = (ctypes.c_uint8 * max_encode_len(len(buf)))()
  input = (ctypes.c_uint8 * len(buf)).from_buffer_copy(buf)
  output_len = _lib.CobsEncodeBuffer(output, input, len(input))
  return bytes(output[:output_len])


def decode(buf: bytes) -> tuple[Status, bytes | None]:
  '''COBS decode bytes.  Returns status and optionally successfully decoded bytes.'''
  # Create buffer same size as input instead of max_decode_len(len(buf)) so that incomplete frames
  # are flagged correctly instead of as overflows.
  output = (ctypes.c_uint8 * len(buf))()
  output_len = ctypes.c_size_t(len(output))
  input = (ctypes.c_uint8 * len(buf)).from_buffer_copy(buf)
  status = _lib.CobsDecodeBuffer(output, output_len, input, len(input)).enum()

  if status != Status.FrameAvailable:
    return status, None

  return status, bytes(output[:output_len.value])


class Encoder:
  '''Incremental COBS Encoder.'''

  def __init__(self, max_encoded_len: int):
    '''Create encoder capable of encoding "max_encoded_len" output buffer.'''
    self._state = _EncodeState()
    self._buf = (ctypes.c_uint8 * max_encoded_len)()
    self.reset()

  def reset(self):
    '''Reset encoder.'''
    self._bytes_encoded = 0
    _lib.CobsEncodeStateInit(self._state, self._buf)

  def encode(self, buf: bytes):
    '''Incrementally COBS encode bytes.'''
    self._bytes_encoded += len(buf)
    if max_encode_len(self._bytes_encoded) > len(self._buf):
      raise RuntimeError('Buffer overflow.  Increase "max_encoded_len".')

    input = (ctypes.c_uint8 * len(buf)).from_buffer_copy(buf)
    _lib.CobsEncodeBlock(self._state, input, len(input), False)

  def get(self) -> bytes:
    '''Get encoded bytes and reset encoder.'''
    _lib.CobsEncodeBlock(self._state, None, 0, True)
    output = bytes(self._state.encoded[:self._state.len])
    self.reset()
    return output


class Decoder:
  '''Incremental COBS Decoder.'''

  def __init__(self, max_decoded_len: int):
    '''Create decoder capable of decoding "max_decoded_len" output buffer.'''
    self._state = _DecodeState()
    self._buf = (ctypes.c_uint8 * max_decoded_len)()
    self.reset()

  def reset(self):
    '''Reset decoder.'''
    _lib.CobsDecodeStateInit(self._state, self._buf, len(self._buf))

  def decode(self, byte: int) -> tuple[Status, bytes | None]:
    '''Incrementally COBS decode byte.  Returns status and optionally successfully decoded bytes.'''
    status = _lib.CobsDecodeByte(self._state, byte).enum()

    if status != Status.FrameAvailable:
      return status, None

    return status, bytes(self._state.decoded[:self._state.len])
