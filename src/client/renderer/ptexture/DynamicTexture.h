#ifndef NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__DynamicTexture_H__
#define NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__DynamicTexture_H__

#include <vector>

class Textures;

class DynamicTexture
{
public:
    int tex;
	int replicate;
    unsigned char pixels[16*16*4];

    DynamicTexture(int tex_);
	virtual ~DynamicTexture() {}

	virtual void tick() = 0;
    virtual void bindTexture(Textures* tex);
};

class WaterTexture: public DynamicTexture
{
    typedef DynamicTexture super;
    int _tick;
	int _frame;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
    WaterTexture();
	~WaterTexture();

    void tick();
};

class WaterSideTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _tick;
	int _frame;
	int _tickCount;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
	WaterSideTexture();
	~WaterSideTexture();

	void tick();
};

class LavaTexture: public DynamicTexture
{
    typedef DynamicTexture super;
    int _tick;
	int _frame;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
    LavaTexture();
	~LavaTexture();

    void tick();
};

class LavaSideTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _tick;
	int _frame;
	int _tickCount;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
	LavaSideTexture();
	~LavaSideTexture();

	void tick();
};


class FireTexture: public DynamicTexture
{
    typedef DynamicTexture super;
    int _tick;
	int _frame;

	float* current;
	float* next;
	float* heat;
	float* heata;

public:
    FireTexture(int id);
	~FireTexture();

    void tick();
};

// Animated nether portal swirl. Cosmic purple cloud field that scrolls and
// drifts, evoking Java/Bedrock's portal animation. Lives at the portal tile's
// atlas slot (PortalTile texIndex).
class PortalTexture: public DynamicTexture
{
	typedef DynamicTexture super;
	int _tick;
	// Two scalar fields, ping-ponged. Same simple smoothing scheme as
	// LavaTexture / WaterTexture so the result is a slow diffusing
	// purple cosmos, biased toward bright sparkles.
	float* current;
	float* next;

public:
	PortalTexture();
	~PortalTexture();

	void tick();
};
#endif /*NET_MINECRAFT_CLIENT_RENDERER_PTEXTURE__DynamicTexture_H__*/
