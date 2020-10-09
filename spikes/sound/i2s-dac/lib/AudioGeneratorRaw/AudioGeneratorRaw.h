#ifndef AudioGeneratorRaw_h
#define AudioGeneratorRaw_h

#include <Arduino.h>
#include <AudioGenerator.h>

class AudioGeneratorRaw : public AudioGenerator {
public:
    AudioGeneratorRaw();
    virtual ~AudioGeneratorRaw() override;
    virtual bool begin(AudioFileSource *source, AudioOutput *output) override;
    virtual bool loop() override;
    virtual bool stop() override;
    virtual bool isRunning() override;

private:
    inline bool finishLoop() {
        file->loop();
        output->loop();
        return running;
    } 
};

#endif
