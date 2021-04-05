/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

int pgm_depth(int max);
uint16_t pgm_pixel_at(uint8_t* buf, int pos, int depth);

// buf: add \0 at the end to guard from cut off data
uint8_t* pgm_parse(uint8_t* buf, uint32_t size, int* width, int* height, int* max);
