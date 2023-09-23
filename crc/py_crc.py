import ctypes

_lib = ctypes.cdll.LoadLibrary('crc/c_crc.so')
_all_crcs = ctypes.cdll.LoadLibrary('crc/all_crcs.so')


class _Crc8Info(ctypes.Structure):
  _fields_ = [
      ('table', ctypes.POINTER(ctypes.c_uint8)),
      ('initial_crc', ctypes.c_uint8),
      ('final_xor', ctypes.c_uint8),
      ('lsb_first', ctypes.c_bool),
  ]


class _Crc16Info(ctypes.Structure):
  _fields_ = [
      ('table', ctypes.POINTER(ctypes.c_uint16)),
      ('initial_crc', ctypes.c_uint16),
      ('final_xor', ctypes.c_uint16),
      ('lsb_first', ctypes.c_bool),
  ]


class _Crc32Info(ctypes.Structure):
  _fields_ = [
      ('table', ctypes.POINTER(ctypes.c_uint32)),
      ('initial_crc', ctypes.c_uint32),
      ('final_xor', ctypes.c_uint32),
      ('lsb_first', ctypes.c_bool),
  ]


_lib.Crc8Update.argtypes = [
    ctypes.POINTER(_Crc8Info),
    ctypes.c_uint8,
    ctypes.c_uint8,
]
_lib.Crc8Update.restype = ctypes.c_uint8
_lib.Crc8Seq.argtypes = [
    ctypes.POINTER(_Crc8Info),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.c_uint8,
]
_lib.Crc8Seq.restype = ctypes.c_uint8
_lib.Crc8Block.argtypes = [
    ctypes.POINTER(_Crc8Info),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
_lib.Crc8Block.restype = ctypes.c_uint8

_lib.Crc16Update.argtypes = [
    ctypes.POINTER(_Crc16Info),
    ctypes.c_uint16,
    ctypes.c_uint8,
]
_lib.Crc16Update.restype = ctypes.c_uint16
_lib.Crc16Seq.argtypes = [
    ctypes.POINTER(_Crc16Info),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.c_uint16,
]
_lib.Crc16Seq.restype = ctypes.c_uint16
_lib.Crc16Block.argtypes = [
    ctypes.POINTER(_Crc16Info),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
_lib.Crc16Block.restype = ctypes.c_uint16

_lib.Crc32Update.argtypes = [
    ctypes.POINTER(_Crc32Info),
    ctypes.c_uint32,
    ctypes.c_uint8,
]
_lib.Crc32Update.restype = ctypes.c_uint32
_lib.Crc32Seq.argtypes = [
    ctypes.POINTER(_Crc32Info),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.c_uint32,
]
_lib.Crc32Seq.restype = ctypes.c_uint32
_lib.Crc32Block.argtypes = [
    ctypes.POINTER(_Crc32Info),
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
_lib.Crc32Block.restype = ctypes.c_uint32

_CrcMapping = {
    8: (_Crc8Info, _lib.Crc8Update, _lib.Crc8Seq, _lib.Crc8Block),
    16: (_Crc16Info, _lib.Crc16Update, _lib.Crc16Seq, _lib.Crc16Block),
    32: (_Crc32Info, _lib.Crc32Update, _lib.Crc32Seq, _lib.Crc32Block),
}


class Crc:

  def __init__(self, bits: int, name: str):
    self.bits = bits

    if bits not in _CrcMapping:
      raise ValueError(f'bits ({bits}) not supported.')

    if str(bits) not in name:
      raise ValueError(f'bits ({bits}) does not match Crc{bits}Info struct name: {name}')

    self._CrcInfo, self._CrcUpdate, self._CrcSeq, self._CrcBlock = _CrcMapping[bits]

    try:
      self.info = self._CrcInfo.in_dll(_all_crcs, name)
    except ValueError:
      raise AttributeError(f'{name} does not name a valid Crc{bits}Info struct.')

    self.reset()

  def reset(self):
    self.crc = self.info.initial_crc

  def block(self, data: bytes) -> int:
    input = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
    return self._CrcBlock(self.info, input, len(input))

  def update(self, data: bytes | int) -> int:
    if isinstance(data, int):
      self.crc = self._CrcUpdate(self.info, self.crc, data)
      return self.info.final_xor ^ self.crc

    input = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
    self.crc = self._CrcSeq(self.info, input, len(input), self.crc)
    return self.info.final_xor ^ self.crc
