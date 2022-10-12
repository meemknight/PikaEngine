#include "frameBuffer.h"

namespace pika
{
	namespace GL
	{

		void PikaFramebuffer::createFramebuffer(unsigned int w, unsigned int h)
		{
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

			//glDrawBuffer(GL_COLOR_ATTACHMENT0); //todo look into this function

			//glGenTextures(1, &depthtTexture); //todo add depth stuff
			//glBindTexture(GL_TEXTURE_2D, depthtTexture);

			//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtTexture, 0);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}

		void PikaFramebuffer::deleteFramebuffer()
		{
			if (fbo)
			{
				glDeleteFramebuffers(1, &fbo);
				fbo = 0;
			}

			if (texture)
			{
				glDeleteTextures(1, &texture);
				texture = 0;
			}
		}

		void PikaFramebuffer::resizeFramebuffer(unsigned int w, unsigned int h)
		{
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			//glBindTexture(GL_TEXTURE_2D, depthtTexture);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			
			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}


}


