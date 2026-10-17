#ifndef SoundSystemAL_H__
#define SoundSystemAL_H__

#include "SoundSystem.h"

#if defined(__APPLE__)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include <vector>
#include <list>

class SoundSystemAL : public SoundSystem
{
public:
    SoundSystemAL();
    ~SoundSystemAL();

    virtual void init();
    virtual void destroy();

    virtual void enable(bool status);

    virtual void setListenerPos(float x, float y, float z);
    virtual void setListenerAngle(float deg);

    virtual void load(const std::string& name) {}
    virtual void play(const std::string& name) {}
    virtual void pause(const std::string& name) {}
    virtual void stop(const std::string& name) {}
    virtual void playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch);

private:
    class Buffer {
    public:
        Buffer() : inited(false) {}
        bool inited;
        ALuint bufferID;
        char* framePtr;
    };

    void removeStoppedSounds();

    static const int MaxNumSources = 12;

    Vec3 _listenerPos;
    float _rotation;

    bool available;

    ALCcontext* context;
    ALCdevice* device;

    ALuint _sources[MaxNumSources];
    std::vector<Buffer> _buffers;

    bool getFreeSourceIndex(int* src);
    bool getBufferId(const SoundDesc& sound, ALuint* buf);
};

#endif