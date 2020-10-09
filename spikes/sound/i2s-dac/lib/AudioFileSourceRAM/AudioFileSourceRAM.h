#ifndef AudioFileSourceRAM_h
#define AudioFileSourceRAM_h

#include <AudioFileSource.h>
#include "CircularBuffer.h"

class AudioFileSourceRAM : public AudioFileSource {
public:
    AudioFileSourceRAM();
    AudioFileSourceRAM(CircularBuffer *buf);
    virtual ~AudioFileSourceRAM() override;
    
    virtual bool open(const char *buf) override;
    virtual uint32_t read(void *data, uint32_t len) override;
    virtual bool seek(int32_t pos, int dir) override;
    virtual bool close() override;
    virtual bool isOpen() override;
    virtual uint32_t getSize() override;
    virtual uint32_t getPos() override;

private:
    CircularBuffer *buffer;
    int pos;
};

#endif