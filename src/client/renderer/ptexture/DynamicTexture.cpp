#include "DynamicTexture.h"

#include <cstring>
#include "../Textures.h"
#include "../../../world/level/tile/Tile.h"
#include "../../../util/Mth.h"

//
// DynamicTexture
//
DynamicTexture::DynamicTexture(int tex_)
	:   tex(tex_),
	replicate(1)
{
	memset(pixels, 0, 16*16*4);
}

void DynamicTexture::bindTexture(Textures* tex) {
	tex->loadAndBindTexture("terrain.png");
}

//
// WaterTexture
// I was thinking of adding something simple (a simple frame copy from a
// "still water image sequence") every n:th tick for calm water, and shifting
// the rows of a texture for the running water. I might do that, but I got
// impressed over the java code, so I will try that first.. and I suspect they
// wont mix very good.
/*
WaterTexture::WaterTexture()
:   super(Tile::water->tex),
_tick(0),
_frame(0)
{
}

void WaterTexture::tick() {
}
*/

WaterTexture::WaterTexture()
	:   super(Tile::water->tex),
	_tick(0),
	_frame(0)
{
	current = new float[16*16];
	next = new float[16*16];
	heat = new float[16*16];
	heata = new float[16*16];

	for (int i = 0; i < 256; ++i) {
		current[i] = 0;
		next[i] = 0;
		heat[i] = 0;
		heata[i] = 0;
	}
}

WaterTexture::~WaterTexture() {
	delete[] current;
	delete[] next;
	delete[] heat;
	delete[] heata;
}

void WaterTexture::tick()
{
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++) {
			float pow = 0;
			for (int xx = x - 1; xx <= x + 1; xx++) {
				int xi = (xx) & 15;
				int yi = (y) & 15;
				pow += current[xi + yi * 16];
			}
			next[x + y * 16] = pow / 3.3f + heat[x + y * 16] * 0.8f;
		}

		for (int x = 0; x < 16; x++)
			for (int y = 0; y < 16; y++) {
				heat[x + y * 16] += heata[x + y * 16] * 0.05f;

				if (heat[x + y * 16] < 0) heat[x + y * 16] = 0;
				heata[x + y * 16] -= 0.1f;
				if (Mth::random() < 0.05f) {
					heata[x + y * 16] = 0.5f;
				}
			}

			float* tmp = next;
			next = current;
			current = tmp;

			for (int i = 0; i < 256; i++) {
				float pow = current[i];
				if (pow > 1) pow = 1;
				if (pow < 0) pow = 0;

				float pp = pow * pow;

				int r = (int) (32 + pp * 32);
				int g = (int) (50 + pp * 64);
				int b = (int) (255);
				int a = (int) (146 + pp * 50);

				//if (anaglyph3d) {
				//	int rr = (r * 30 + g * 59 + b * 11) / 100;
				//	int gg = (r * 30 + g * 70) / (100);
				//	int bb = (r * 30 + b * 70) / (100);

				//	r = rr;
				//	g = gg;
				//	b = bb;
				//}

				pixels[i * 4 + 0] = r;
				pixels[i * 4 + 1] = g;
				pixels[i * 4 + 2] = b;
				pixels[i * 4 + 3] = a;
			}
}

//
// WaterSideTexture
//
WaterSideTexture::WaterSideTexture()
	:   super(Tile::water->tex + 1),
	_tick(0),
	_frame(0),
	_tickCount(0)
{
	replicate = 2;

	current = new float[16*16];
	next = new float[16*16];
	heat = new float[16*16];
	heata = new float[16*16];

	for (int i = 0; i < 256; ++i) {
		current[i] = 0;
		next[i] = 0;
		heat[i] = 0;
		heata[i] = 0;
	}
}

WaterSideTexture::~WaterSideTexture() {
	delete[] current;
	delete[] next;
	delete[] heat;
	delete[] heata;
}

void WaterSideTexture::tick() {
	++_tickCount;
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++) {
			float pow = 0;
			for (int xx = y - 2; xx <= y; xx++) {
				int xi = (x) & 15;
				int yi = (xx) & 15;
				pow += current[xi + yi * 16];
			}
			next[x + y * 16] = pow / 3.2f + heat[x + y * 16] * 0.8f;
		}

		for (int x = 0; x < 16; x++)
			for (int y = 0; y < 16; y++) {
				heat[x + y * 16] += heata[x + y * 16] * 0.05f;

				if (heat[x + y * 16] < 0) heat[x + y * 16] = 0;
				heata[x + y * 16] -= 0.3f;
				if (Mth::random() < 0.2) {
					heata[x + y * 16] = 0.5f;
				}
			}
			float* tmp = next;
			next = current;
			current = tmp;

			for (int i = 0; i < 256; i++) {
				float pow = current[(i - _tickCount * 16) & 255];
				if (pow > 1) pow = 1;
				if (pow < 0) pow = 0;

				float pp = pow * pow;

				int r = (int) (32 + pp * 32);
				int g = (int) (50 + pp * 64);
				int b = (int) (255);
				int a = (int) (146 + pp * 50);

				//if (anaglyph3d) {
				//	int rr = (r * 30 + g * 59 + b * 11) / 100;
				//	int gg = (r * 30 + g * 70) / (100);
				//	int bb = (r * 30 + b * 70) / (100);

				//	r = rr;
				//	g = gg;
				//	b = bb;
				//}

				pixels[i * 4 + 0] = r;
				pixels[i * 4 + 1] = g;
				pixels[i * 4 + 2] = b;
				pixels[i * 4 + 3] = a;
			}
}

///
/// Lava Texture
///

LavaTexture::LavaTexture()
	:   super(Tile::lava->tex),
	_tick(0),
	_frame(0)
{
	current = new float[16*16];
	next = new float[16*16];
	heat = new float[16*16];
	heata = new float[16*16];

	for (int i = 0; i < 256; ++i) {
		current[i] = 0;
		next[i] = 0;
		heat[i] = 0;
		heata[i] = 0;
	}
}

LavaTexture::~LavaTexture() {
	delete[] current;
	delete[] next;
	delete[] heat;
	delete[] heata;
}

void LavaTexture::tick()
{
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++) {
			float pow = 0;
			int xxo = (int)(Mth::sin((float)(y) * (float)(Mth::PI) * 2.0f / 16.0f) * 1.2f);
			int yyo = (int)(Mth::sin((float)(x) * (float)(Mth::PI) * 2.0f / 16.0f) * 1.2f);
			for (int xx = x - 1; xx <= x + 1; xx++) {
				for (int yy = y - 1; yy <= y + 1; yy++) {
					int xi = xx + xxo & 15;
					int yi = yy + yyo & 15;
					pow += current[xi + yi * 16];
				}
			}
			next[x + y * 16] = pow / 10.0f + (heat[(x + 0 & 15) + (y + 0 & 15) * 16] + heat[(x + 1 & 15) + (y + 0 & 15) * 16] + heat[(x + 1 & 15) + (y + 1 & 15) * 16] + heat[(x + 0 & 15) + (y + 1 & 15) * 16]) / 4.0f * 0.8f;
			heat[x + y * 16] = heat[x + y * 16] + heata[x + y * 16] * 0.01f;
			if (heat[x + y * 16] < 0.0f) {
				heat[x + y * 16] = 0.0f;
			}
			heata[x + y * 16] = heata[x + y * 16] - 0.06f;
			if (Mth::random() < 0.005) {
				heata[x + y * 16] = 1.5f;
			}
		}


		float* tmp = next;
		next = current;
		current = tmp;

		for (int i = 0; i < 256; i++) {
			float pow = current[i] * 2.0f;
			if (pow > 1) pow = 1;
			if (pow < 0) pow = 0;

			float pp = pow * pow;

			int r = (int) (pow * 100.0f + 155.0f);
			int g = (int) (pp * 255.0f);
			int b = (int) (pp * pp * 128.0f);

			//if (anaglyph3d) {
			//	int rr = (r * 30 + g * 59 + b * 11) / 100;
			//	int gg = (r * 30 + g * 70) / (100);
			//	int bb = (r * 30 + b * 70) / (100);

			//	r = rr;
			//	g = gg;
			//	b = bb;
			//}

			pixels[i * 4 + 0] = r;
			pixels[i * 4 + 1] = g;
			pixels[i * 4 + 2] = b;
			pixels[i * 4 + 3] = -1;
		}
}

///
/// Lava Side Texture
///

LavaSideTexture::LavaSideTexture()
	:   super(Tile::lava->tex + 1),
	_tick(0),
	_frame(0),
	_tickCount(0)
{
	replicate = 2;

	current = new float[16*16];
	next = new float[16*16];
	heat = new float[16*16];
	heata = new float[16*16];

	for (int i = 0; i < 256; ++i) {
		current[i] = 0;
		next[i] = 0;
		heat[i] = 0;
		heata[i] = 0;
	}
}

LavaSideTexture::~LavaSideTexture() {
	delete[] current;
	delete[] next;
	delete[] heat;
	delete[] heata;
}

void LavaSideTexture::tick() {
	++_tickCount;
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++) {
			float pow = 0;
			int yl = (int)(Mth::sin((float)(y) * (float)(Mth::PI)* 2.0f / 16.0f) * 1.2f); // var2 is y
			int xl = (int)(Mth::sin((float)(x) * (float)(Mth::PI)* 2.0f / 16.0f) * 1.2f); // var1 is x
			for (int yy = x - 1; yy <= x + 1; yy++) {
				for (int xx = y - 1; xx <= y + 1; xx++) {
					int xi = (yy + yl) & 15; // var8
					int yi = (xx + xl) & 15; //var9
					pow += current[xi + yi * 16];
				}
			}
			next[x + y * 16] =
				next[x + y * 16] =
				pow / 10.0f + (
				heat[(x + 0 & 15) + (y + 0 & 15) *
				16] + heat[
					(x + 1 & 15) + (y + 0 & 15) * 16] + heat[(x + 1 & 15) + (y + 1 & 15) *
						16] + heat[
							(x + 0 & 15) + (y + 1 & 15) * 16]) / 4.0f
								* 0.8f;
							heat[x + y * 16] = heat[x + y * 16] + heata[x + y * 16] * 0.01f;

							if (heat[x + y * 16] < 0.0f) {
								heat[x + y * 16] = 0.0f;
							}
							heata[x + y * 16] = heata[x + y * 16] - 0.06f;

							if (Mth::random() < 0.005) {
								heata[x + y * 16] = 1.5f;
							}
		}
		float* tmp = next;
		next = current;
		current = tmp;

		for (int i = 0; i < 256; i++) {
			float pow = current[(i - _tickCount / 3 * 16) & 255] * 2.0f;
			if (pow > 1) pow = 1;
			if (pow < 0) pow = 0;

			float pp = pow * pow;

			int r = (int) (pow * 100.0f + 155.0f);
			int g = (int) (pow * pow * 255.0f);
			int b = (int) (pow * pow * pow * pow * 128.0f);
//			int a = (int) (146 + pp * 50);

			//if (anaglyph3d) {
			//	int rr = (r * 30 + g * 59 + b * 11) / 100;
			//	int gg = (r * 30 + g * 70) / (100);
			//	int bb = (r * 30 + b * 70) / (100);

			//	r = rr;
			//	g = gg;
			//	b = bb;
			//}

			pixels[i * 4 + 0] = r;
			pixels[i * 4 + 1] = g;
			pixels[i * 4 + 2] = b;
			pixels[i * 4 + 3] = -1;
		}
}

FireTexture::FireTexture(int id)
	:   super(((Tile*)Tile::fire)->tex + id * 16),
	_tick(0),
	_frame(0)
{
	current = new float[16*20];
	next = new float[16*20];
	heat = new float[16*20];
	heata = new float[16*20];

	for (int i = 0; i < 256; ++i) {
		current[i] = 0;
		next[i] = 0;
		heat[i] = 0;
		heata[i] = 0;
	}
}

FireTexture::~FireTexture() {
	delete[] current;
	delete[] next;
	delete[] heat;
	delete[] heata;
}


// oh boy time to implement fire textures, i am so fucked - shredder

void FireTexture::tick() {
	//  loop generates fire texture  on the empty texture grid, hopefully shouldnt be too taxing on older hardware - shredder
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 20; y++) {
			int count = 18;
			float pow = this->current[x + (y + 1) % 20 * 16] * (float)(count);
			for (int xx = x - 1; xx <= x + 1; xx++) {
				for (int yy = y; yy <= y + 1; yy++) {
					if (xx >= 0 && yy >= 0 && xx < 16 && yy < 20) {
						pow += this->current[xx + yy * 16];
					}
					count++;
				}
			}
			this->next[x + y * 16] = pow / (float(count) * 1.06f);
			if (y >= 19) {
				this->next[x + y * 16] = float(Mth::random() * Mth::random() * Mth::random() * 4.0 + Mth::random() * 0.1f + 0.2f);
			}
		}
	}

	// hopefully this doesn't cause any mysterious issues - shredder
	float* tmp = next;
	next = current;
	current = tmp;

	for (int i = 0; i < 256; i++) {
		float pow = this->current[i] * 1.8f;
		if (pow > 1.0f) {
			pow = 1.0f;
		}
		if (pow < 0.0f) {
			pow = 0.0f;
		}


		int r = (int) (pow * 155.0f + 100.0f);
		int g = (int)(pow * pow * 255.0f);
		int b = (int)(pow * pow * pow * pow * pow * pow * pow * pow * pow * pow * 255.0f);
		int a = 255;
		if (pow < 0.5f) {
			a = 0;
		}

		// @TODO: cant be arsed rn to implement the anaglyph3d check would be nice to check if it does - shredder
		//if (this->anaglyph3d) {
		//    float rr = (r * 30 + g * 59 + b * 11) / 100;
		//    float gg = (r * 30 + g * 70) / 100;
		//    float bb = (r * 30 + b * 70) / 100;
		//    r = rr;
		//    g = gg;
		//    b = bb;
		//}

		pixels[i * 4 + 0] = r;
		pixels[i * 4 + 1] = g;
		pixels[i * 4 + 2] = b;
		pixels[i * 4 + 3] = a;

	}
}

//
// PortalTexture
//
// Animated swirl. Each tick we (a) diffuse a 16x16 field, (b) inject random
// "spark" hotspots, (c) sample the field with two scrolling/rotating offsets
// so the visible pattern keeps moving even when the field itself is calm,
// and (d) map the brightness to a violet palette.
//
PortalTexture::PortalTexture()
	: super(Tile::portalTile ? Tile::portalTile->tex : 0),
	  _tick(0)
{
	current = new float[16*16];
	next = new float[16*16];
	for (int i = 0; i < 256; ++i) {
		current[i] = 0;
		next[i] = 0;
	}
}

PortalTexture::~PortalTexture()
{
	delete[] current;
	delete[] next;
}

void PortalTexture::tick()
{
	++_tick;

	// Diffuse: each cell averages a 3x3 box of its previous neighbours, with
	// a small decay so old hotspots fade.
	for (int y = 0; y < 16; ++y)
	for (int x = 0; x < 16; ++x) {
		float sum = 0;
		for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			int xi = (x + dx) & 15;
			int yi = (y + dy) & 15;
			sum += current[xi + yi * 16];
		}
		next[x + y * 16] = sum * (1.0f / 9.2f);
	}

	// Inject a few random sparks each tick so the field doesn't decay flat.
	for (int i = 0; i < 4; ++i) {
		int px = (int)(Mth::random() * 16) & 15;
		int py = (int)(Mth::random() * 16) & 15;
		next[px + py * 16] += 0.9f;
	}

	float* tmp = next; next = current; current = tmp;

	// Sample: shift the field by a slowly-rotating offset so the visible
	// pattern drifts continuously. The two trig terms give a swirling feel
	// when combined with the diffusion above.
	float t = _tick * 0.05f;
	float ox = Mth::sin(t)        * 3.0f;
	float oy = Mth::cos(t * 0.7f) * 3.0f;

	for (int y = 0; y < 16; ++y)
	for (int x = 0; x < 16; ++x) {
		int sx = ((int)(x + ox) & 15);
		int sy = ((int)(y + oy) & 15);
		float v = current[sx + sy * 16];
		// Add a second sample on a different offset, multiplied in, so
		// peaks line up where both samples are bright — that produces the
		// "stars in mist" look.
		int sx2 = ((int)(x - ox * 0.7f) & 15);
		int sy2 = ((int)(y - oy * 0.7f) & 15);
		float v2 = current[sx2 + sy2 * 16];
		float b = v * 0.6f + v2 * v2 * 1.4f;
		if (b > 1.0f) b = 1.0f;
		if (b < 0.0f) b = 0.0f;

		// Violet palette: dark indigo base, bright magenta peaks.
		int r = (int)(40  + b * 180);  // 40..220
		int g = (int)(10  + b *  60);  // 10..70
		int b8 = (int)(80 + b * 175);  // 80..255
		int a = (int)(180 + b *  75);  // 180..255

		int idx = (x + y * 16) * 4;
		pixels[idx + 0] = (unsigned char)(r  > 255 ? 255 : r);
		pixels[idx + 1] = (unsigned char)(g  > 255 ? 255 : g);
		pixels[idx + 2] = (unsigned char)(b8 > 255 ? 255 : b8);
		pixels[idx + 3] = (unsigned char)(a  > 255 ? 255 : a);
	}
}