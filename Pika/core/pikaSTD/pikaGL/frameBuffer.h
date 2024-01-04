#pragma once
#include <glad/glad.h>

namespace pika
{

	namespace GL
	{

		struct PikaFramebuffer
		{
			GLuint fbo = 0;
			GLuint texture = 0;
			GLuint depthTexture = 0;

			unsigned int w = 0;
			unsigned int h = 0;

			void createFramebuffer(unsigned int w, unsigned int h, bool hasDepth = 0);

			void deleteFramebuffer();

			void resizeFramebuffer(unsigned int w, unsigned int h);
		
			void clear();

		};


		



	}



}