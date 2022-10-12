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

			void createFramebuffer(unsigned int w, unsigned int h);

			void deleteFramebuffer();

			void resizeFramebuffer(unsigned int w, unsigned int h);
		
		};


		



	}



}