uint8_t* download(WiFiClient *s, int32_t len) {
  uint8_t *buffer = (uint8_t *)ps_malloc(len);
  uint8_t *buff = buffer;
  while (len > 0) {
    if (!s->connected()) {
      free(buffer);
      return NULL;
    }
    size_t size = s->available();
    if (size) {
      int c = s->readBytes(buff, ((size > len) ? len : size));
      if (len > 0) {
        len -= c;
      }
      buff += c;
    }
  }
  return buffer;
}
