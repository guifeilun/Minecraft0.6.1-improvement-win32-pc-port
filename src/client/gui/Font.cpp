#include "Font.h"
#include "../Options.h"
#include "../renderer/Textures.h"
#include "../renderer/Tesselator.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <vector>
#include <stdint.h>

#ifndef bool_t
typedef bool bool_t;
#endif

static int utf8Iterate(const uint8_t* s, int len, int32_t* out)
{
    if (len <= 0) return 0;
    uint8_t c = s[0];
    if (c < 0x80) { *out = c; return 1; }
    if ((c & 0xE0) == 0xC0 && len >= 2) {
        *out = ((c & 0x1F) << 6) | (s[1] & 0x3F); return 2;
    }
    if ((c & 0xF0) == 0xE0 && len >= 3) {
        *out = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); return 3;
    }
    if ((c & 0xF8) == 0xF0 && len >= 4) {
        *out = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F); return 4;
    }
    *out = c; return 1;
}

Font::Font(Options* options, const std::string& name, Textures* textures)
{
    glyphSizes = 0;
    field_4 = 1;
    field_C = 10;
    lineHeight = DefaultLineHeight;
    field_1820.file = name;
    field_1820.options = options;
    field_1820.textures = textures;
    field_1820.field_18 = 16;
    field_1820.field_1C = 16;
    field_1820.field_10 = 0;
    field_1820.field_14 = 0;

    for (int i = 0; i < 256; i++) {
        charWidths[i] = 0;
        charLength[i] = 0.0f;
    }

    init(options);
}

Font::~Font()
{
    if (glyphSizes) delete[] glyphSizes;
}

void Font::onGraphicsReset()
{
    init(field_1820.options);
}

void Font::init(Options* options)
{
    TextureId texId = field_1820.textures->loadTexture(field_1820.file);
    const TextureData* data = field_1820.textures->getTemporaryTextureData(texId);
    if (!data) return;

    int32_t v7 = field_1820.field_18;
    int32_t v8 = 0;
    int32_t v9 = field_1820.field_1C * v7;

    while (v8 < v9) {
        int32_t v10 = field_1820.field_10 + 8 * (v8 % v7) + data->w * 8 * (v8 / v7) + field_1820.field_14;
        int32_t v11 = 7;
        do {
            int32_t v12 = 0;
            int32_t v13 = 8;
            while (1) {
                bool_t v27 = data->data[4 * v12 + 4 * v11 + 4 * v10] == 0;
                if (!--v13) break;
                v12 += data->w;
                if (!v27) goto SET_CHAR_LENGTH;
            }
            if (data->data[4 * v12 + 4 * v11 + 4 * v10]) break;
        } while (v11-- != 0);
SET_CHAR_LENGTH:
        if (v8 == ' ') v11 = 2;
        int32_t width = v11 + 2;
        charWidths[v8] = width;
        charLength[v8] = (float)width;
        ++v8;
    }

    for (uint32_t i = 0; i != 32; ++i) {
        int32_t v18 = 85 * ((i >> 3) & 1);
        int32_t v19 = v18 + 170 * ((i >> 2) & 1);
        int32_t v20 = v18 + 170 * ((i >> 1) & 1);
        int32_t v21 = v18 + 170 * (i & 1);
        if (i == 6) v19 = (v19 & 0x00) | ((v19 + 85) & 0xff);
        else if ((int32_t)i > 15) { v19 >>= 2; v20 >>= 2; v21 >>= 2; }
        field_1844[i] = ((v20 & 0xff00) << 8) | ((v19 & 0xff) << 16) | v21;
    }

    if (glyphSizes) { delete[] glyphSizes; glyphSizes = 0; }

    FILE* fp = fopen("data/images/font/glyph_sizes.bin", "rb");
    if (fp) {
        glyphSizes = new uint8_t[65536];
        fread(glyphSizes, 1, 65536, fp);
        fclose(fp);
    }
}

void Font::setDefaultTexture()
{
    field_1820.textures->loadAndBindTexture(field_1820.file);
}

void Font::setUnicodeTexture(int32_t g)
{
    char name[32];
    sprintf(name, "font/glyph_%02X.png", g);
    field_1820.textures->loadAndBindTexture(name);
}

float Font::buildChar(int32_t ch, float x, float y, bool_t mb)
{
    if (ch == 32) return 4.0f;

    float v7, v8;
    if (mb || ch > 255) {
        int32_t v6 = glyphSizes[ch];
        v7 = (float)(v6 >> 4);
        v8 = ((float)((v6 & 0xf) + 1) - v7) * 0.5f + 1.0f;
    } else {
        v7 = 0;
        v8 = charLength[ch];
    }

    float v9 = x - v7;
    float v10 = (float)(ch & 0xf0);
    float v11 = 16.0f * (ch & 0xf);
    float v12 = v11 * 0.0039062f;
    float v13 = floorf(x - v7);

    Tesselator& t = Tesselator::instance;
    t.vertexUV(v13, y + 8, 0, v12, (v10 + 15.99f) * 0.0039062f);
    float v14 = (v11 + 15.99f) * 0.0039062f;
    float v15 = floorf(v9 + 8.0f);
    t.vertexUV(v15, y + 8, 0, v14, (v10 + 15.99f) * 0.0039062f);
    t.vertexUV(v15, y, 0, v14, v10 * 0.0039062f);
    t.vertexUV(v13, y, 0, v12, v10 * 0.0039062f);

    return v8;
}

int32_t Font::charWidth(int32_t ch, bool_t hasUnicode)
{
    if (ch == ' ') return 4;
    if (ch <= 255 && !hasUnicode) return charWidths[ch];
    if (glyphSizes) {
        int32_t r = glyphSizes[ch];
        if (r) return (int32_t)((r & 0xf) + 1 - (r >> 4)) / 2 + 1;
    }
    return 0;
}

bool_t Font::containsUnicodeChar(const std::string& str)
{
    int32_t i;
    const uint8_t* data = (const uint8_t*)str.c_str();
    int32_t len = str.length();

    do {
        int32_t v4 = utf8Iterate(data, len, &i);
        if (v4 <= 0) return 0;
        data += v4;
        len -= v4;
    } while (i <= 255);
    return 1;
}

void Font::draw(const char* s, float x, float y, int32_t color)
{
    draw(s, x, y, color, 0);
}

void Font::draw(const char* s, float x, float y, int32_t color, bool_t shadow)
{
    drawSlow(s, strlen(s), x, y, color, shadow);
}

void Font::draw(const std::string& s, float x, float y, int32_t color)
{
    draw(s, x, y, color, 0);
}

void Font::draw(const std::string& s, float x, float y, int32_t color, bool_t shadow)
{
    drawSlow(s, x, y, color, shadow);
}

void Font::drawShadow(const char* s, float x, float y, int32_t color)
{
    draw(s, x + 1, y + 1, color, 1);
    draw(s, x, y, color);
}

void Font::drawShadow(const std::string& s, float x, float y, int32_t color)
{
    draw(s, x + 1, y + 1, color, 1);
    draw(s, x, y, color);
}

void Font::drawShadow(const std::string& s, float x, float y, int32_t c1, int32_t c2)
{
    draw(s, x + 1, y + 1, c2);
    draw(s, x, y, c1);
}

void Font::drawSlow(const char* s, int32_t len, float x, float y, int32_t color, bool_t shadow)
{
    int32_t col = color;
    const uint8_t* ss;
    float v12, v13;
    bool_t v15;
    int32_t alpha, v17;

    if (!s) return;

    if (shadow) col = (color & 0xff000000) + ((color & 0xfcfcfc) >> 2);

    ss = (const uint8_t*)s;
    v12 = 0;
    v13 = 0;
    int32_t v14 = -1;
    v15 = containsUnicodeChar(s);

    alpha = (col >> 24) & 0xff;
    if (!alpha) alpha = 255;

    int32_t r = (col >> 16) & 0xff;
    int32_t g = (col >> 8) & 0xff;
    int32_t b = col & 0xff;

    Tesselator& t = Tesselator::instance;
    int32_t cp;

    while (1) {
        int32_t n = utf8Iterate(ss, len, &cp);
        if (n <= 0) break;
        ss += n;
        len -= n;

        v17 = cp / 256;

        if (cp / 256 != v14 && cp != ' ') {
            if (v14 != -1) {
                t.draw();
                t.addOffset(-x, -y, 0);
            }

            if (v17 || v15) setUnicodeTexture(v17);
            else setDefaultTexture();

            t.begin();
            t.color(r, g, b, alpha);
            v14 = v17;
            t.addOffset(x, y, 0);
        }

        if (cp == 10) {
            v13 = 0;
            v12 += field_C;
        } else if (v14 || v15) {
            v13 += buildChar(cp, v13, v12, 1);
        } else {
            v13 += buildChar(cp, v13, v12, 0);
        }
    }

    if (v14 != -1) {
        t.draw();
        t.addOffset(-x, -y, 0);
    }
    field_4 = 1;
}

void Font::drawSlow(const std::string& s, float x, float y, int32_t col, bool_t shadow)
{
    drawSlow(s.c_str(), s.length(), x, y, col, shadow);
}

void Font::drawWordWrap(const std::string& str, float x, float y, float w, int col)
{
    std::vector<std::string> words;
    const char* lims = " \n\t\r";
    char* cstr = new char[str.length() + 1];
    strncpy(cstr, str.c_str(), str.length());
    cstr[str.length()] = 0;
    char* ptok = strtok(cstr, lims);
    while (ptok) { words.push_back(ptok); ptok = strtok(0, lims); }
    delete[] cstr;

    float py = y;
    int pos = 0;
    while (pos < (int)words.size()) {
        std::string line = words[pos++] + " ";
        while (pos < (int)words.size() && width(line + words[pos]) < w) {
            line += words[pos++] + " ";
        }
        drawShadow(line, x, py, col);
        py += field_C;
    }
}

int Font::height(const std::string& s)
{
    int h = 0;
    bool hasLine = false;
    for (unsigned int i = 0; i < s.length(); ++i) {
        if (s[i] == '\n') hasLine = true;
        else {
            if (hasLine) h += field_C;
            hasLine = false;
        }
    }
    return h;
}

int Font::width(const std::string& s)
{
    bool_t hasUnicode = containsUnicodeChar(s);
    int32_t maxLine = 0;
    int32_t cur = 0;
    int32_t len = s.length();
    const uint8_t* data = (const uint8_t*)s.c_str();
    int32_t cp;

    while (1) {
        int32_t n = utf8Iterate(data, len, &cp);
        if (n <= 0) break;
        data += n;
        len -= n;

        int32_t cw = charWidth(cp, hasUnicode);

        if (cp == 10) {
            if (cur > maxLine) maxLine = cur;
            cur = 0;
        } else {
            if (cw > 0) cur += cw;
        }
    }
    return cur > maxLine ? cur : maxLine;
}