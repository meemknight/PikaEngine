#include "engineGL2DSupport.h"


::gl2d::Texture pika::gl2d::loadTextureWithPixelPadding(const char *path, RequestedContainerInfo &info, int blockSize,
	bool pixelated, bool useMipMaps)
{
	::gl2d::Texture t = {};
	size_t s = 0;
	if (info.getFileSizeBinary(path, s))
	{
		void *data = new unsigned char[s];
		if (info.readEntireFileBinary(path, data, s))
		{
			t.createFromFileDataWithPixelPadding((unsigned char *)data, s, blockSize, pixelated, useMipMaps);

		}

		delete[] data;
	}
	else { return {}; }

	return t;
}

::gl2d::Texture pika::gl2d::loadTexture(const char *path, RequestedContainerInfo &info, bool pixelated, bool useMipMaps)
{
	::gl2d::Texture t = {};
	size_t s = 0;
	if (info.getFileSizeBinary(path, s))
	{
		void *data = new unsigned char[s];
		if (info.readEntireFileBinary(path, data, s))
		{
			t.createFromFileData((unsigned char *)data, s, pixelated, useMipMaps);
		}

		delete[] data;
	}
	else { return {}; }

	return t;
}

::gl2d::Font pika::gl2d::loadFont(const char *path, RequestedContainerInfo &info)
{
	::gl2d::Font f = {};
	size_t s = 0;
	if (info.getFileSizeBinary(path, s))
	{
		void *data = new unsigned char[s];
		if (info.readEntireFileBinary(path, data, s))
		{
			f.createFromTTF((unsigned char *)data, s);
		}

		delete[] data;
	}
	else { return f; }

	return f;
}
