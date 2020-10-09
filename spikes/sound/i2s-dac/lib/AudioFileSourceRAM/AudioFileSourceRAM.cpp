#include <Arduino.h>
#include "AudioFileSourceRAM.h"
#include "CircularBuffer.h"

AudioFileSourceRAM::AudioFileSourceRAM() {
    pos = 0;
    buffer = NULL;
}

AudioFileSourceRAM::AudioFileSourceRAM(CircularBuffer *buf) {
    pos = 0;
    buffer = buf;
}

AudioFileSourceRAM::~AudioFileSourceRAM() {}

bool AudioFileSourceRAM::open(const char *buf) {
    buffer = (CircularBuffer*) buf;
    return true;
}

uint32_t AudioFileSourceRAM::read(void *data, uint32_t len) {
    if (data == NULL) {
        audioLogger->printf_P(PSTR("ERROR! AudioFileSourceRAM::read passed NULL data\n"));
        return 0;
    }

    size_t bytesRead = buffer->shift((byte*) data, len);
    pos += bytesRead;

    return bytesRead;
}

bool AudioFileSourceRAM::seek(int32_t pos, int dir) {
    audioLogger->printf_P(PSTR("ERROR! AudioFileSourceRAM::seek not implemented!"));
    (void) pos;
    (void) dir;
    return false;
}

bool AudioFileSourceRAM::close() {
    return true;
}

bool AudioFileSourceRAM::isOpen() {
    return buffer != NULL;
}

uint32_t AudioFileSourceRAM::getSize() {
    return buffer->taken();
}

uint32_t AudioFileSourceRAM::getPos() {
    return pos;
}