#ifndef SoundSystemAL_H__
#define SoundSystemAL_H__

#include "SoundSystem.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

class SoundSystemAL : public SoundSystem
{
public:
    SoundSystemAL();
    virtual ~SoundSystemAL();

    virtual bool isAvailable() { return _available; }
    virtual void enable(bool status);
    
    virtual void setListenerPos(float x, float y, float z);
    virtual void setListenerAngle(float deg);

    virtual void load(const std::string& name){}
    virtual void play(const std::string& name){}
    virtual void pause(const std::string& name){}
    virtual void stop(const std::string& name){}
    virtual void playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch);

private:
    static const int MAX_PLAYED = 12;

    struct BufferInfo {
        LPDIRECTSOUNDBUFFER buffer;
        bool inUse;
        BufferInfo() : buffer(NULL), inUse(false) {}
    };

    LPDIRECTSOUND8 _dsound;
    BufferInfo _buffers[MAX_PLAYED];
    int _playedCnt;
    float _rotation;
    bool _available;

    bool initDirectSound();
    int getFreeSourceIndex();
    void removeStoppedSounds();
    bool createBuffer(const SoundDesc& sound, LPDIRECTSOUNDBUFFER* outBuffer);
};

#endif