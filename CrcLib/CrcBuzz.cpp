#include "CrcBuzz.h"

namespace CrcUtility {
void CrcBuzz::Initialize(unsigned char pin)
{
    _buzzPin = pin;
    pinMode(_buzzPin, OUTPUT);
}

void CrcBuzz::StartTune(Tune* tune)
{
    _currentTune = tune;

    Update(0);
}

void CrcBuzz::Update(unsigned int delta)
{
    if (_currentTune == NO_TUNE)
        return;

    if (_currentTune->Update(
            delta,
            [](pitch_t note, void* self) { ((CrcBuzz*)self)->PlayTone(note); },
            this))
        _currentTune = NO_TUNE;
}

void CrcBuzz::PlayTone(unsigned int pitch) const
{
    if (pitch == NOTE_SILENCE)
        noTone(_buzzPin);
    else
        tone(_buzzPin, pitch);
}
}
