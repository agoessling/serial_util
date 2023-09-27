import ctypes

_lib = ctypes.cdll.LoadLibrary('crc/c_crc.so')
print(ctypes.cdll.LoadLibrary('crc/all_crcs.so').__dict__)


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

_CrcInfo = {
    8: _Crc8Info,
    16: _Crc16Info,
    32: _Crc32Info,
}


class Crc:

  def __init__(self, bits: int, name: str):
    if bits not in _CrcInfo:
      raise ValueError(f'bits ({bits}) not supported.')

    self.bits = bits
    self.info = getattr(_lib, name)
