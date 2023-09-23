import argparse
import textwrap


def _reflect(byte: int, bits: int) -> int:
  if bits < 1:
    raise ValueError('bits must be greater than 0')

  if byte < 0 or byte > 1 << bits:
    raise ValueError('byte must be between 0 and 1 << bits - 1')

  output = 0
  for i in range(bits):
    output |= ((byte >> i) & 0x01) << (bits - 1 - i)
  return output


def _hex_fmt(val: int, bits: int) -> str:
  width = (bits - 1) // 4 + 1
  return f'{{:#0{width + 2}x}}'.format(val)


def _data_type(bits: int) -> str:
  if bits < 0 or bits > 64:
    raise ValueError('Invalid number of bits.')

  num_bytes = (bits - 1) // 8 + 1

  if num_bytes > 4:
    return 'uint64_t'
  elif num_bytes > 2:
    return 'uint32_t'
  elif num_bytes > 1:
    return 'uint16_t'

  return 'uint8_t'


def crc_table(bits: int, poly: int, lsb_first: bool) -> list[int]:
  if bits < 8 or bits > 32:
    raise ValueError(f'bits must be between 8 and 32.')

  if poly < 0 or poly >= 1 << bits:
    raise ValueError(f'poly must be between 0 and 1 << bits - 1.')

  mask = (1 << bits) - 1

  table = []
  for byte in range(256):
    if lsb_first:
      crc = byte
    else:
      crc = byte << (bits - 8)
    for _ in range(8):
      if lsb_first:
        if crc & 0x01:
          crc = (crc >> 1) ^ _reflect(poly, bits)
        else:
          crc = (crc >> 1)
      else:
        if crc & (1 << (bits - 1)):
          crc = (crc << 1) ^ poly
        else:
          crc = (crc << 1)
    table.append(crc & mask)

  return table


def table_str(table: list[int], bits: int) -> str:
  lines = []
  for i in range(0, 256, 8):
    lines.append(', '.join([_hex_fmt(x, bits) for x in table[i:i + 8]]) + ',')
  return '\n'.join(lines)


def get_source(table_name: str, table: list[int], bits: int) -> str:
  return f'''\
#include <stdint.h>

const {_data_type(bits)} {table_name}[{len(table)}] = {{
{textwrap.indent(table_str(table, bits), '    ')}
}};
'''


def get_header(table_name: str, table: list[int], bits: int) -> str:
  return f'''\
#pragma once

#include <stdint.h>

extern const {_data_type(bits)} {table_name}[{len(table)}];
'''


def main():
  parser = argparse.ArgumentParser(description='Generate 256 element CRC lookup table.')
  parser.add_argument('-b', '--bits', type=int, required=True, help='CRC bit length.')
  parser.add_argument('-p', '--polynomial', required=True, help='Generator polynomial.')
  parser.add_argument('--lsb_first', action='store_true', help='LSB first or reflected table.')
  parser.add_argument('--header', help='Header file name to write.')
  parser.add_argument('--source', help='Source file name to write.')
  parser.add_argument('--name', help='Name for C array.')
  parser.add_argument('--print', action='store_true', help='Print table values.')

  args = parser.parse_args()

  if args.polynomial.lower().startswith('0x'):
    poly = int(args.polynomial, 16)
  else:
    poly = int(args.polynomial)

  table = crc_table(args.bits, poly, args.lsb_first)

  write_output = any(x is not None for x in [args.header, args.source, args.name])
  if write_output:
    if not all(x is not None for x in [args.header, args.source, args.name]):
      parser.error('--header, --source, and --name must be used together.')

  if write_output:
    with open(args.header, 'w') as f:
      f.write(get_header(args.name, table, args.bits))

    with open(args.source, 'w') as f:
      f.write(get_source(args.name, table, args.bits))

  if args.print:
    print('CRC Table')
    print(f'Bits: {args.bits}')
    print(f'Polynomial: {_hex_fmt(poly, args.bits)}')
    print(f'LSB First (reflected): {args.lsb_first}')
    print('Values:')
    print(table_str(table, args.bits))


if __name__ == '__main__':
  main()
