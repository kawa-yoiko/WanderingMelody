#ifndef __WanderingMelody__SoundbankSoundFont_h__
#define __WanderingMelody__SoundbankSoundFont_h__

#include "audio/Soundbank.h"

#include <cstdint>

class SoundbankSoundFont : public Soundbank {
public:
    SoundbankSoundFont(const char *s);

    virtual void sendNote(const MusicNote &note) override;

protected:
    void render(float *output, uint32_t nframe);
    struct {
        MusicNote musicNote;
        float angVel;
        int phase;
    } _lastNote, _curNote;
    int _lastNoteOffTime;
    bool _isCurNoteOn;
};

#endif