#ifndef AudioBuffer_h
#define AudioBuffer_h

#include <Arduino.h>

class AudioBuffer {
public:
    AudioBuffer(size_t capacity);
    ~AudioBuffer();

    void clear();

    size_t available();
    size_t taken();

    bool isEmpty();
    bool isFull();

    size_t push(byte *data, size_t sz);
    size_t shift(byte *target, size_t sz);

    void debug();

private:
    byte *buffer;
    byte *end;
    byte *head;
    byte *tail;
    size_t count;
    size_t capacity;

    /**
     * Tells how much data is used behind head, either until tail, or end
     * This is data that can be read in one go
     */
    size_t headSpace();

    /**
     * Tells how much space is left behind tail, either until end, or head
     * This is space that can be used in one write
     */
    size_t tailSpace();

    /**
     * Read from internal buffer with no bounds check, potentially in two memcpy calls
     */
    void readBuffer(byte *target, size_t firstPart, size_t secondPart = 0);

    /**
     * Write internal buffer with no bounds check, potentially in two memcpy calls
     */
    void writeBuffer(byte *data, size_t firstPart, size_t secondPart = 0);
};

#endif