#include "AudioBuffer.h"

#include <Arduino.h>

AudioBuffer::AudioBuffer(size_t capacity) {
    buffer = new byte[capacity];
    end = &buffer[capacity-1];
    head = buffer;
    tail = buffer;
    count = 0;
    this->capacity = capacity;
}

AudioBuffer::~AudioBuffer() {
    delete[] buffer;
}

void AudioBuffer::clear() {
    memset(buffer, '\0', capacity);
}

size_t AudioBuffer::available() {
    return capacity - count;
}

size_t AudioBuffer::taken() {
    return count;
}

bool AudioBuffer::isEmpty() {
    return count == 0;
}

bool AudioBuffer::isFull() {
    return count == capacity;
}

size_t AudioBuffer::push(byte *data, size_t sz) {
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

size_t AudioBuffer::tailSpace() {
    if (tail < head) {
        return head - tail;
    } else {
        return end - tail + 1;
    }
}

void AudioBuffer::writeBuffer(byte *data, size_t firstPart, size_t secondPart) {
    memcpy(tail, data, firstPart);
    if (secondPart != 0) {
        memcpy(buffer, data + firstPart, secondPart);
        tail = buffer - firstPart;
    }

    count += firstPart + secondPart;
    tail += firstPart + secondPart;
}

size_t AudioBuffer::shift(byte *target, size_t sz) {
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

size_t AudioBuffer::headSpace() {
    if (head <= tail) {
        return tail - head;
    } else {
        return end - head + 1;
    }
}

void AudioBuffer::readBuffer(byte *target, size_t firstPart, size_t secondPart) {
    memcpy(target, head, firstPart);
    if (secondPart != 0) {
        memcpy(target + firstPart, buffer, secondPart);
        head = buffer - firstPart;
    }

    count -= (firstPart + secondPart);
    head += firstPart + secondPart;
}

void AudioBuffer::debug() {
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