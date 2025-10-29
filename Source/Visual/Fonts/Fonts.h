#pragma once
#include "DXFont.h"

using namespace plugin;

static class Fonts
{
	static inline bool init = false;

	static inline DXFont* fArtist = new DXFont();
	static inline DXFont* fTrack = new DXFont();

public:

	static void Initialize(const char*& fontFileName, const char*& fontName, int sizeBig, int sizeSmall) {
		if (init) return;
		
		init = true;

		InitFont(fArtist, fontFileName, fontName, sizeBig);
		InitFont(fTrack, fontFileName, fontName, sizeSmall);
	}

	static DXFont* getFontArtist() {
		return getIfValid(fArtist);
	}

	static DXFont* getFontTrack() {
		return getIfValid(fTrack);
	}

protected:

	static DXFont* getIfValid(DXFont* font) {
		if (font)
		{
			return font;
		}
		else {
			return nullptr;
		}
	}

	static void InitFont(DXFont* font, const char*& fontFileName, const char*& fontName, int size) {

		Events::initRwEvent.Add([font, fontFileName, fontName, size]()
			{ 
				font->InitFont(fontFileName, fontName, size);
			}
		);
		Events::d3dResetEvent.Add([font, fontFileName, fontName, size]()
			{ 
				font->InitFont(fontFileName, fontName, size);
			}
		);
	}
};