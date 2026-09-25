#ifndef NET_MINECRAFT_CLIENT_GUI__Font_H__
#define NET_MINECRAFT_CLIENT_GUI__Font_H__

#include <string>
#include <vector>
#include <stdint.h>
#include "../renderer/gles.h"

#ifndef bool_t
typedef bool bool_t;
#endif

class Textures;
class Options;

class Font
{
public:
    Font(Options*, const std::string&, Textures*);
    virtual ~Font();

    void init(Options*);
    void onGraphicsReset();

    void draw(const char*, float, float, int32_t);
    void draw(const std::string&, float, float, int32_t);
    void draw(const char*, float, float, int32_t, bool_t);
    void draw(const std::string&, float, float, int32_t, bool_t);
    void drawShadow(const char*, float, float, int32_t);
    void drawShadow(const std::string&, float, float, int32_t);
    void drawShadow(const std::string&, float, float, int32_t, int32_t);
    void drawWordWrap(const std::string&, float, float, float, int);

    int width(const std::string&);
    int height(const std::string&);

    float buildChar(int32_t, float, float, bool_t);
    int32_t charWidth(int32_t, bool_t);
    bool_t containsUnicodeChar(const std::string&);

    void drawSlow(const char*, int32_t, float, float, int32_t, bool_t);
    void drawSlow(const std::string&, float, float, int32_t, bool_t);
    void setDefaultTexture();
    void setUnicodeTexture(int32_t);

    int32_t lineHeight;
    static const int DefaultLineHeight = 10;

    int32_t charWidths[256];
    float charLength[256];
    uint8_t* glyphSizes;

    int32_t field_4;
    int32_t field_C;

    struct _Font_someStruct {
        std::string file;
        Textures* textures;
        Options* options;
        int32_t field_10, field_14, field_18, field_1C;
    } field_1820;

    int32_t field_1844[32];
};

#endif