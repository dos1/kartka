/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <rom/miniz.h>

uint8_t* deflate_decompress(const uint8_t *src_buf, size_t src_len, size_t *out_len) {
  tinfl_decompressor *decomp = (tinfl_decompressor*) malloc(sizeof(tinfl_decompressor));
  tinfl_status status;
  uint8_t *buf = NULL;
  size_t pos = 0, capacity = 0;
  size_t incr = 524288;
  if (*out_len) {
    incr = *out_len;
  }
  *out_len = 0;

  tinfl_init(decomp);
  do {
    capacity += incr;
    void *new_buf = ps_realloc(buf, capacity);
    if (!new_buf) {
      free(buf);
      free(decomp);
      return NULL;
    }
    buf = (uint8_t*)new_buf;

    size_t src_size = src_len - pos;
    size_t dst_size = capacity - *out_len;

    status = tinfl_decompress(decomp, src_buf + pos, &src_size, buf, buf + *out_len, &dst_size, TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF | TINFL_FLAG_PARSE_ZLIB_HEADER);

    if ((status < 0) || (status == TINFL_STATUS_NEEDS_MORE_INPUT)) {
      free(buf);
      free(decomp);
      return NULL;
    }

    pos += src_size;
    *out_len += dst_size;
  } while(status != TINFL_STATUS_DONE);

  free(decomp);
  return buf;
}
