#include "gig/Musician.h"
#include "gig/Gig.h"

#include <algorithm>

void Musician::startPlay()
{
    _curTime = _curTick = 0;
    _isPlaying = true;
    _barlinePtr = 0;
    _isInBreak = false;

    _beater = Beater(8, 1);
}

void Musician::stopPlay()
{
    _isPlaying = false;
    _isInBreak = false;
}

void Musician::resumePlay()
{
    _isPlaying = true;
}

double Musician::tickToTime(double tick)
{
    auto it = std::upper_bound(_tempoChanges.begin(), _tempoChanges.end(),
        tick,
        [] (const double a, const tempoChange &b) { return a < b.ticks; }
    );
    it--;
    return it->time + (double)(tick - it->ticks) / it->tempo;
}

double Musician::timeToTick(double time)
{
    auto it = std::upper_bound(_tempoChanges.begin(), _tempoChanges.end(),
        time,
        [] (const double a, const tempoChange &b) { return a < b.time; }
    );
    it--;
    return it->ticks + (double)(time - it->time) * it->tempo;
}

void Musician::tick(double dt, double lcap, double hcap)
{
    if (!_isPlaying) return;
    int32_t lastTick = (int32_t)_curTick;

    _curTime += dt;
    _rawTick = timeToTick(_beater.getY(_curTime));
    _curTick = std::min(std::max(_rawTick, lcap), hcap);

    for (auto &mt : _musicTracks) {
        MusicTrack::flags_t f =
            mt.triggerAutoNotes(lastTick, (int32_t)_curTick, _isAutoplay);
        this->processFlags(f);
    }
}

void Musician::jump(double tick)
{
    _curTime = tickToTime(tick);
    _rawTick = _curTick = tick;
    _beater = Beater(8, 1);
}

void Musician::clearTriggered()
{
    for (KeyNote &n : _keyNotes) n.triggered = -1;
}

void Musician::addHit(double time, int32_t noteTick, bool propagateUp)
{
    if (propagateUp && (_isAutoscroll || _isAutoplay)) return;
    if (!propagateUp || !_isCooperative)
        _beater.update({time, tickToTime(noteTick)}, _beater.getK(_curTime));
    if (propagateUp && _gig) _gig->dispatchHit(_tag, _curTime, noteTick);
}

void Musician::processFlags(MusicTrack::flags_t flags)
{
    if (flags & MusicTrack::FLAGS_BREAK)
        _isInBreak ^= 1;
}

Musician::~Musician()
{
    for (auto &mt : _musicTracks) mt.releaseSoundbank();
}
