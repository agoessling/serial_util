#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Calculate maximum COBS encoded length from decoded length.
#define COBS_MAX_ENCODE_LEN(decode_len) \
  ((decode_len) > 0 ? (decode_len) + ((decode_len) + 253) / 254 + 1 : 2)

// Calculate maximum COBS decoded length from encoded length.
#define COBS_MAX_DECODE_LEN(encode_len) ((encode_len) > 1 ? (encode_len)-2 : 0)

typedef enum {
  kCobsStatusForceSigned = -1,
  kCobsStatusProcessing,
  kCobsStatusFrameAvailable,
  kCobsStatusMalformedFrame,
  kCobsStatusOverflow,
  kCobsStatusIncompleteFrame,
  kNumCobsStatus
} CobsStatus;

typedef struct {
  // Public.
  uint8_t *encoded;  // Encoded output buffer.
  size_t len;  // Length of encoded output buffer.

  // Private.
  uint8_t *_delim_ptr;
  uint8_t *_write_ptr;
  uint8_t _delim_cnt;
} CobsEncodeState;

typedef struct {
  // Public.
  uint8_t *decoded;  // Decoded output buffer.
  size_t len;  // Length of decoded output buffer.

  // Private.
  uint8_t *_write_ptr;
  uint8_t *_end_ptr;
  uint8_t _delim_cnt;
  bool _mandatory_delim;
} CobsDecodeState;

// Initialize state for use with CobsEncodeBlock.
void CobsEncodeStateInit(CobsEncodeState *state, uint8_t *output_buf);

// Sequentially encode block of data into output buffer specified by "state".  Optionally finalizing
// encoded data via "finalize".
void CobsEncodeBlock(CobsEncodeState *state, const uint8_t *input_buf, size_t len, bool finalize);

// Encode single input buffer into output buffer.  Store encoded length in "output_len".
size_t CobsEncodeBuffer(uint8_t *output_buf, const uint8_t *input_buf, size_t input_len);

// Initialize state for use with CobsDecodeByte.
void CobsDecodeStateInit(CobsDecodeState *state, uint8_t *output_buf, size_t len);

// Sequentially decode data per byte into buffer associated with "state".  Resets decoder on success
// or COBS error.  Returns decode status.
CobsStatus CobsDecodeByte(CobsDecodeState *state, uint8_t byte);

// Decodes single input buffer into output buffer, writing "output_len" on success. Returns decode
// status.
CobsStatus CobsDecodeBuffer(uint8_t *output_buf, size_t *output_len, const uint8_t *input_buf,
                            size_t input_len);
