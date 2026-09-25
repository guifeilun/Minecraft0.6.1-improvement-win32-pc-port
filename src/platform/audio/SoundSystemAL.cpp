#include "SoundSystemAL.h"
#include "../../util/Mth.h"
#include "../../world/phys/Vec3.h"
#include "../../client/sound/Sound.h"
#include "../log.h"
#include <math.h>

SoundSystemAL::SoundSystemAL()
    : _dsound(NULL), _playedCnt(0), _rotation(-9999.9f), _available(false)
{
    _available = initDirectSound();
}

SoundSystemAL::~SoundSystemAL()
{
    for (int i = 0; i < MAX_PLAYED; i++) {
        if (_buffers[i].buffer) {
            _buffers[i].buffer->Stop();
            _buffers[i].buffer->Release();
            _buffers[i].buffer = NULL;
        }
        _buffers[i].inUse = false;
    }

    if (_dsound) {
        _dsound->Release();
        _dsound = NULL;
    }
}

bool SoundSystemAL::initDirectSound()
{
    HRESULT hr = DirectSoundCreate8(NULL, &_dsound, NULL);
    if (FAILED(hr)) {
        return false;
    }

    HWND hwnd = GetActiveWindow();
    if (!hwnd) {
        _dsound->Release();
        _dsound = NULL;
        return false;
    }

    hr = _dsound->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
    if (FAILED(hr)) {
        _dsound->Release();
        _dsound = NULL;
        return false;
    }

    return true;
}

void SoundSystemAL::enable(bool status)
{
    _available = status && (_dsound != NULL);
}

void SoundSystemAL::setListenerPos(float x, float y, float z) {}
void SoundSystemAL::setListenerAngle(float deg) { _rotation = deg; }

void SoundSystemAL::removeStoppedSounds()
{
    _playedCnt = 0;
    for (int i = 0; i < MAX_PLAYED; i++) {
        if (!_buffers[i].buffer) continue;
        
        DWORD status;
        HRESULT hr = _buffers[i].buffer->GetStatus(&status);
        if (FAILED(hr)) {
            _buffers[i].buffer->Release();
            _buffers[i].buffer = NULL;
            _buffers[i].inUse = false;
            continue;
        }
        
        if (status & DSBSTATUS_PLAYING) {
            _playedCnt++;
        } else {
            _buffers[i].buffer->Release();
            _buffers[i].buffer = NULL;
            _buffers[i].inUse = false;
        }
    }
}

int SoundSystemAL::getFreeSourceIndex()
{
    removeStoppedSounds();
    
    for (int i = 0; i < MAX_PLAYED; i++) {
        if (!_buffers[i].inUse || !_buffers[i].buffer) {
            _buffers[i].inUse = true;
            return i;
        }
    }
    return -1;
}

bool SoundSystemAL::createBuffer(const SoundDesc& sound, LPDIRECTSOUNDBUFFER* outBuffer)
{
    if (!sound.isValid()) {
        return false;
    }

    WAVEFORMATEX wf;
    ZeroMemory(&wf, sizeof(wf));
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nSamplesPerSec = sound.frameRate;
    wf.wBitsPerSample = sound.byteWidth * 8;
    wf.nChannels = sound.channels;
    wf.nBlockAlign = sound.channels * sound.byteWidth;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    wf.cbSize = 0;

    DSBUFFERDESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
    desc.dwBufferBytes = sound.size;
    desc.lpwfxFormat = &wf;

    LPDIRECTSOUNDBUFFER tmpBuffer = NULL;
    HRESULT hr = _dsound->CreateSoundBuffer(&desc, &tmpBuffer, NULL);
    if (FAILED(hr)) {
        return false;
    }

    *outBuffer = tmpBuffer;

    void* writePtr = NULL;
    DWORD writeSize = 0;
    hr = (*outBuffer)->Lock(0, sound.size, &writePtr, &writeSize, NULL, 0, 0);
    if (FAILED(hr)) {
        (*outBuffer)->Release();
        *outBuffer = NULL;
        return false;
    }

    memcpy(writePtr, sound.frames, sound.size);
    (*outBuffer)->Unlock(writePtr, writeSize, NULL, 0);

    return true;
}

void SoundSystemAL::playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch)
{
    if (!_available || !_dsound) return;
    if (!sound.isValid()) {
        return;
    }

    int index = getFreeSourceIndex();
    if (index < 0) {
        return;
    }

    LPDIRECTSOUNDBUFFER buffer = NULL;
    if (!createBuffer(sound, &buffer)) {
        _buffers[index].inUse = false;
        return;
    }

    _buffers[index].buffer = buffer;

    LONG dsVolume = DSBVOLUME_MIN;
    if (volume > 0.0f) {
        float vol = (volume > 1.0f) ? 1.0f : volume;
        if (vol > 0.0f) {
            dsVolume = (LONG)(2000.0f * log10f(vol));
            if (dsVolume < DSBVOLUME_MIN) dsVolume = DSBVOLUME_MIN;
            if (dsVolume > DSBVOLUME_MAX) dsVolume = DSBVOLUME_MAX;
        }
    }
    buffer->SetVolume(dsVolume);

    HRESULT hr = buffer->Play(0, 0, 0);
    if (FAILED(hr)) {
        buffer->Release();
        _buffers[index].buffer = NULL;
        _buffers[index].inUse = false;
    }
}