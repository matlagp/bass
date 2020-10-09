#include <Arduino.h>
#include "AudioGeneratorRaw.h"

AudioGeneratorRaw::AudioGeneratorRaw() {
    running = false;
}

AudioGeneratorRaw::~AudioGeneratorRaw() {}

bool AudioGeneratorRaw::begin(AudioFileSource *in, AudioOutput *out) {
    if (!in) return false;
    file = in;

    if (!out) return false;
    output = out;

    output->begin();
    running = true;

    return true;
}

bool AudioGeneratorRaw::loop() {
    if (!running) return finishLoop();

    while (file->getSize() >= 4) { // 2 bytes for both channels
        file->read(lastSample, 4);
        if (!output->ConsumeSample(lastSample)) return finishLoop();
        Serial.println("Consumed 4b sample");
    }
}


bool AudioGeneratorRaw::stop() {
    running = false;
    output->stop();
    return file->close();
}

bool AudioGeneratorRaw::isRunning() {
    return running;
}