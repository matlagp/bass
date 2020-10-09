#include "CircularBuffer.h"

#include <Arduino.h>

CircularBuffer::CircularBuffer(size_t capacity) {
    buffer = new byte[capacity];
    end = &buffer[capacity-1];
    head = buffer;
    tail = buffer;
    count = 0;
    this->capacity = capacity;
}

CircularBuffer::~CircularBuffer() {
    delete[] buffer;
}

void CircularBuffer::clear() {
    memset(buffer, '\0', capacity);
}

size_t CircularBuffer::available() {
    return capacity - count;
}

size_t CircularBuffer::taken() {
    return count;
}

bool CircularBuffer::isEmpty() {
    return count == 0;
}

bool CircularBuffer::isFull() {
    return count == capacity;
}

size_t CircularBuffer::push(byte *data, size_t sz) {
    size_t curr_available = available();
    size_t curr_tspace = tailSpace();

    if (sz <= curr_available) { // We're able to save everything
        writeBuffer(data, min(sz, curr_tspace), max(0, (int) sz - (int) curr_tspace));
        return sz;
    } else { // We're able to save part
        writeBuffer(data, curr_tspace, curr_available - curr_tspace);
        return curr_available;
    }
}

size_t CircularBuffer::tailSpace() {
    if (tail < head) {
        return head - tail;
    } else {
        return end - tail + 1;
    }
}

void CircularBuffer::writeBuffer(byte *data, size_t firstPart, size_t secondPart) {
    memcpy(tail, data, firstPart);
    if (secondPart != 0) {
        memcpy(buffer, data + firstPart, secondPart);
        tail = buffer - firstPart;
    }

    count += firstPart + secondPart;
    tail += firstPart + secondPart;
}

size_t CircularBuffer::shift(byte *target, size_t sz) {
    size_t curr_taken = taken();
    size_t curr_hspace = headSpace();

    if (sz <= curr_taken) {
        readBuffer(target, min(sz, curr_hspace), max(0, (int) sz - (int) curr_hspace));
        return sz;
    } else {
        readBuffer(target, curr_hspace, curr_taken - curr_hspace);
        return curr_taken;
    }
}

size_t CircularBuffer::headSpace() {
    if (head <= tail) {
        return tail - head;
    } else {
        return end - head + 1;
    }
}

void CircularBuffer::readBuffer(byte *target, size_t firstPart, size_t secondPart) {
    memcpy(target, head, firstPart);
    if (secondPart != 0) {
        memcpy(target + firstPart, buffer, secondPart);
        head = buffer - firstPart;
    }

    count -= (firstPart + secondPart);
    head += firstPart + secondPart;
}

void CircularBuffer::debug() {
    Serial.printf("Current count: %d\n", count);
    Serial.printf("Available/taken: %d/%d\n", available(), taken());
	for (int i = 0; i < capacity; i++) {
		int hex = (int)buffer + i;
		Serial.print("[");
		Serial.print(hex, HEX);
		Serial.print("] ");
		Serial.print(*(buffer + i));
		if (head == buffer + i) {
			Serial.print("<-head");
		} 
		if (tail == buffer + i) {
			Serial.print("<-tail");
		}
		Serial.println();
	}
}