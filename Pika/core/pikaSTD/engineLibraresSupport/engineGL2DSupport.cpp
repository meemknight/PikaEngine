#include "engineGL2DSupport.h"

//todo use global allocator and test.
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
		else
		{
			info.consoleWrite(std::string("error loading texture: ") + path);
		}

		delete[] data;
	}
	else 
	{
		info.consoleWrite(std::string("error loading texture: ") + path);
		return {}; 
	}

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
		else
		{
			info.consoleWrite(std::string("error loading texture: ") + path);
		}

		delete[] data;
	}
	else
	{
		info.consoleWrite(std::string("error loading texture: ") + path);
		return {}; 
	}

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
		else
		{
			info.consoleWrite(std::string("error loading font: ") + path);
		}

		delete[] data;
	}
	else
	{
		info.consoleWrite(std::string("error loading font: ") + path);
		return {};
	}

	return f;
}

void pika::gl2d::cameraController(::gl2d::Camera &c, Input &input, float speed, float zoomSpeed)
{
	if (input.buttons[pika::Button::W].held())
	{
		c.position.y -= speed * input.deltaTime;
	}
	if (input.buttons[pika::Button::S].held())
	{
		c.position.y += speed * input.deltaTime;
	}
	if (input.buttons[pika::Button::A].held())
	{
		c.position.x -= speed * input.deltaTime;
	}
	if (input.buttons[pika::Button::D].held())
	{
		c.position.x += speed * input.deltaTime;
	}
	if (input.buttons[pika::Button::Q].held())
	{
		c.zoom -= zoomSpeed * input.deltaTime;
	}
	if (input.buttons[pika::Button::E].held())
	{
		c.zoom += zoomSpeed * input.deltaTime;
	}

	c.zoom = glm::clamp(c.zoom, 0.001f, 1000.f);
}
