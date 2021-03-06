#ifndef __WanderingMelody__AudioOutput_h__
#define __WanderingMelody__AudioOutput_h__

#include "audio/miniaudio.h"

#include <functional>
#include <utility>
#include <vector>
#include <cstdint>

class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();

    typedef std::function<void (float *, uint32_t)> callback_t;

    void startPlayback();
    int registerCallback(callback_t callback);
    void removeCallback(int id);

    static inline void createInstance() { if (!_instance) _instance = new AudioOutput(); }
    static inline void destroyInstance() { if (_instance) { delete _instance; _instance = nullptr; } }
    static inline AudioOutput *getInstance() { return _instance; }
    static inline const char *getError() { return getInstance()->_errMsg; }

private:
    const char *_errMsg;

    ma_device _device;
    std::vector<std::pair<bool, callback_t>> _callbacks;
    std::vector<size_t> _emptyPos;

    void render(float *outbuf, uint32_t nframes);

    friend void audioOutputCallback(
        ma_device *device, float *outbuf, float *inbuf, ma_uint32 nframes);

    static AudioOutput *_instance;
};

#endif
