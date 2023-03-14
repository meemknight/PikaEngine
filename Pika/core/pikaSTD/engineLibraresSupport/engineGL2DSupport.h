#pragma once

#include <gl2d/gl2d.h>
#include <baseContainer.h>


namespace pika
{

	namespace gl2d
	{


		::gl2d::Texture loadTextureWithPixelPadding(const char *path, RequestedContainerInfo &info, int blockSize,
			bool pixelated = false, bool useMipMaps = true);

		::gl2d::Texture loadTexture(const char *path, RequestedContainerInfo &info,
			bool pixelated = false, bool useMipMaps = true);

		::gl2d::Font loadFont(const char *path, RequestedContainerInfo &info);


	}

}