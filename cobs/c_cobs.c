#include "cobs/c_cobs.h"

#include <stdbool.h>
#include <stdint.h>

static const uint8_t kCobsDelimiter = 0x00;

void CobsEncodeStateInit(CobsEncodeState *state, uint8_t *output_buf) {
  state->encoded = output_buf;
  state->len = 0;
  state->_delim_ptr = output_buf;
  state->_write_ptr = output_buf + 1;
  state->_delim_cnt = 1;
}

void CobsEncodeBlock(CobsEncodeState *state, const uint8_t *input_buf, size_t len, bool finalize) {
  for (size_t i = 0; i < len; ++i) {
    // Mandatory delimiter required.
    if (state->_delim_cnt == 0xFF) {
      *state->_delim_ptr = state->_delim_cnt;
      state->_delim_ptr = state->_write_ptr++;
      state->_delim_cnt = 1;
    }

    if (*input_buf == kCobsDelimiter) {
      *state->_delim_ptr = state->_delim_cnt;
      state->_delim_ptr = state->_write_ptr++;
      state->_delim_cnt = 1;
    } else {
      *state->_write_ptr++ = *input_buf;
      state->_delim_cnt++;
    }

    input_buf++;
  }

  if (finalize) {
    *state->_delim_ptr = state->_delim_cnt;
    *state->_write_ptr++ = kCobsDelimiter;
    state->len = (size_t)(state->_write_ptr - state->encoded);
  }
}

size_t CobsEncodeBuffer(uint8_t *output_buf, const uint8_t *input_buf, size_t input_len) {
  CobsEncodeState state;
  CobsEncodeStateInit(&state, output_buf);
  CobsEncodeBlock(&state, input_buf, input_len, true);
  return state.len;
}

CobsStatus CobsDecodeBuffer(uint8_t *output_buf, size_t *output_len, const uint8_t *input_buf,
                            size_t input_len) {
  CobsDecodeState state;
  CobsDecodeStateInit(&state, output_buf, *output_len);
  *output_len = 0;

  for (size_t i = 0; i < input_len; ++i) {
    CobsStatus status = CobsDecodeByte(&state, *input_buf++);

    if (status == kCobsStatusFrameAvailable) {
      *output_len = state.len;
      return status;
    }

    // Report errors.
    if (status != kCobsStatusProcessing) {
      return status;
    }
  }

  return kCobsStatusIncompleteFrame;
}

static inline void CobsDecodeStateReset(CobsDecodeState *state) {
  state->_write_ptr = state->decoded;
  state->_delim_cnt = 0;
  state->_mandatory_delim = true;
}

void CobsDecodeStateInit(CobsDecodeState *state, uint8_t *output_buf, size_t len) {
  state->decoded = output_buf;
  state->len = 0;
  state->_end_ptr = output_buf + len;

  CobsDecodeStateReset(state);
}

CobsStatus CobsDecodeByte(CobsDecodeState *state, uint8_t byte) {
  // This byte is a delimiter.
  if (state->_delim_cnt == 0) {
    // End of frame.
    if (byte == kCobsDelimiter) {
      state->len = (size_t)(state->_write_ptr - state->decoded);
      CobsDecodeStateReset(state);
      return kCobsStatusFrameAvailable;
    }

    // If there isn't a mandatory delimiter it means the data was the reserved byte.
    if (!state->_mandatory_delim) {
      if (state->_write_ptr >= state->_end_ptr) {
        CobsDecodeStateReset(state);
        return kCobsStatusOverflow;
      }
      *state->_write_ptr++ = kCobsDelimiter;
    }

    state->_delim_cnt = byte;
    state->_mandatory_delim = state->_delim_cnt == 0xFF;

    // This should not be a delimiter.
  } else {
    if (byte == kCobsDelimiter) {
      CobsDecodeStateReset(state);
      return kCobsStatusMalformedFrame;
    }

    if (state->_write_ptr >= state->_end_ptr) {
      CobsDecodeStateReset(state);
      return kCobsStatusOverflow;
    }

    *state->_write_ptr++ = byte;
  }

  state->_delim_cnt--;

  return kCobsStatusProcessing;
}
