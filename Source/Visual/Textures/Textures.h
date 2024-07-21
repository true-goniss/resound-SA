#pragma once
#include <CFileLoader.h>
#include "Utils/Utils.h"
// originally by gennariarmando (_AG)

static class Textures
{
	static RwTexture* getTextureFromPngFile(const char* file) {

		int w, h, d, f;
		RwTexture* texture = NULL;

		if (Utils::FileCheck(file)) {
			if (RwImage* img = RtPNGImageRead(file)) {
				RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

				if (RwRaster* raster = RwRasterCreate(w, h, d, f)) {
					RwRasterSetFromImage(raster, img);

					if (texture = RwTextureCreate(raster)) {
						if ((texture->raster->cFormat & 0x80) == 0)
							RwTextureSetFilterMode(texture, rwFILTERLINEAR);
						else
							RwTextureSetFilterMode(texture, rwFILTERLINEARMIPLINEAR);

						RwTextureSetAddressing(texture, rwTEXTUREADDRESSWRAP);
					}
				}

				RwImageDestroy(img);
			}
		}

		return texture;
	}

public:

	static RwTexture* LoadPNGTexture(const char* folderName, const char* name) {
		char file[512];

		sprintf(file, "resound\\textures\\%s\\%s.png", folderName, name);

		return getTextureFromPngFile(file);
	}

	static RwTexture* LoadPNGCustomFolder(const char* folderName, const char* name) {
		char file[512];

		sprintf(file, "%s\\%s.png", folderName, name);

		return getTextureFromPngFile(file);
	}


	static void RasterFromPngFile(const char* file, RwRaster*& raster) {
		int w, h, d, f;

		/*raster = nullptr;*/

		if (RwImage* img = RtPNGImageRead(file)) {
			RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

			if (!raster) {
				if (raster = RwRasterCreate(w, h, d, f)) {
					RwRasterSetFromImage(raster, img);
				}
			}
			else {
				RwRasterSetFromImage(raster, img);
			}

			RwImageDestroy(img);
		}
	}
};

