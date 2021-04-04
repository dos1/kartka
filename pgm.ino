#include <stdio.h>
#include <stdint.h>

static uint8_t* pgm_skip_whitespace(uint8_t *buf) {
  while (isspace(*buf)) {
    buf++;
  }
  return buf;
}

static uint8_t* pgm_skip_to_newline(uint8_t *buf) {
  while (*buf && *buf != '\n' && *buf != '\r') {
    buf++;
  }
  return buf++;
}

static uint8_t* pgm_skip_comments(uint8_t *buf) {
  buf = pgm_skip_whitespace(buf);
  if (*buf != '#') {
      return buf;
  }
  buf = pgm_skip_to_newline(buf);
  return pgm_skip_comments(buf);
}

int pgm_depth(int max) {
  if (max > 255) {
    return 2;
  }
  return 1;
}

uint16_t pgm_pixel_at(uint8_t* buf, int pos, int depth) {
  if (depth == 2) {
    pos *= 2;
    return ((uint16_t)buf[pos] << 8) + buf[pos+1];
  } else {
    return buf[pos];
  }
}

uint8_t* pgm_parse(uint8_t* buf, uint32_t size, int* width, int* height, int* max) {
  uint8_t* orig = buf;
  int pos;
  if (size < 3) {
    Serial.println("Insufficient size to hold the magic");
    return NULL;
  }
  if (strncmp((char*)buf, "P5", 2)) {
    Serial.println("Invalid magic");
    return NULL;
  }
  buf = pgm_skip_comments(buf + 2);
  if (!*buf || sscanf((char*)buf, "%d%n", width, &pos) != 1) {
    Serial.println("Could not receive the width");
    return NULL;
  }
  buf = pgm_skip_comments(buf + pos);
  if (!*buf || sscanf((char*)buf, "%d%n", height, &pos) != 1) {
    Serial.println("Could not receive the height");
    return NULL;
  }
  buf = pgm_skip_comments(buf + pos);
  if (!*buf || sscanf((char*)buf, "%d%n", max, &pos) != 1) {
    Serial.println("Could not receive the maximum value");
    return NULL;
  }
  buf += pos;
  if (!isspace(*buf)) {
    Serial.println("No whitespace before data");
    return NULL;
  }
  buf++;
  if (size < buf - orig + pgm_depth(*max) * *width * *height) {
    Serial.println("Insufficient size to hold the image");
    return NULL;
  }
  return buf;
}
