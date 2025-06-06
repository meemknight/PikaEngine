#include "gl2d/gl2d.h"
/////////////////////////////////////////////////////////////////
//gl2d.cpp				1.6.3
//Copyright(c) 2020 - 2025 Luta Vlad
//https://github.com/meemknight/gl2d
// 
//notes: 
// 1.2.1
// fixed alpha in the particle system with the 
// post process
// 
// 1.2.2
// added default values to structs
// added some more error reporting
// added option to change gl version and 
//  shader presision
// vsynk independend of gl loader
// 
// 1.2.3
// small problems fixes
// texture load flags
// working on 9pathces
// 
// 1.2.4
// working at fixing get text size
// 
// 1.2.5
// push pop shaders and camera
// added getViewRect
// 
// 1.2.6
// updated camera.follow
// removed TextureRegion
// 
// 1.3.0
// polished using custom shader api
// fixed camera follow
// moved the particle system into another file
// added a proper cmake
// used the proper stbi free function
// added a default fbo support
// added proper error reporting (with uer defined data)
// 
// 1.4.0
// much needed api refactoring
// removed capacity render limit
// added some more comments
// 
// 1.4.1
// line rendering
// rect outline rendering
// circle outline rendering
// 
// 1.5.0
// started to add some more needed text functions
// needed to be tested tho
// 
// 1.5.1
// fixed the follow function
// 
// 1.5.2
// read texture data + report error if opengl not loaded
// 
// 1.6.0
// Added post processing API + Improvements in custom shaders usage
// 
// 1.6.1
// trying to fix some text stuff
//
// 1.6.2
// finally fixed font rendering problems
// 
// 1.6.3
// added depth option to the framebuffer
// 
////////////////////////////////////////////////////////////////////////


//	todo
//
//	add particle demo
//	refactor particle system to woth with the new post process api
//	add matrices transforms
//	flags for vbos
//	add render circle
//	
//

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <gl2d/gl2d.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

//if you are not using visual studio make shure you link to "Opengl32.lib"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244 4305 4267 4996 4018)
#pragma comment(lib, "Opengl32.lib")
#endif

#undef max


namespace gl2d
{
#pragma region shaders

	static ShaderProgram defaultShader = {};
	static Camera defaultCamera{};
	static Texture white1pxSquareTexture = {};

	static const char *defaultVertexShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		"in vec3 quad_positions;\n"
		"in vec4 quad_colors;\n"
		"in vec2 texturePositions;\n"
		"out vec4 v_color;\n"
		"out vec2 v_texture;\n"
		"out vec2 v_positions;\n"
		"uniform mat4 u_viewProjection = mat4(1);\n"
		"void main()\n"
		"{\n"
		"	gl_Position = u_viewProjection * vec4(quad_positions, 1);\n"
		"	v_color = quad_colors;\n"
		"	v_texture = texturePositions;\n"
		"	v_positions = gl_Position.xy;\n"
		"}\n";

	static const char *defaultFragmentShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		"out vec4 color;\n"
		"in vec4 v_color;\n"
		"in vec2 v_texture;\n"
		"uniform sampler2D u_sampler;\n"
		"void main()\n"
		"{\n"
		"    color = v_color * texture2D(u_sampler, v_texture);\n"
		"}\n";

	static const char *defaultVertexPostProcessShader =
		GL2D_OPNEGL_SHADER_VERSION "\n"
		GL2D_OPNEGL_SHADER_PRECISION "\n"
		"in vec2 quad_positions;\n"
		"out vec2 v_positions;\n"
		"out vec2 v_texture;\n"
		"out vec4 v_color;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(quad_positions, 0, 1);\n"
		"	v_positions = gl_Position.xy;\n"
		"	v_color = vec4(1,1,1,1);\n"
		"	v_texture = (gl_Position.xy + vec2(1))/2.f;\n"
		"}\n";

#pragma endregion

	static errorFuncType *errorFunc = defaultErrorFunc;

	void defaultErrorFunc(const char *msg, void *userDefinedData)
	{
		std::cerr << "gl2d error: " << msg << "\n";
	}

	void *userDefinedData = 0;
	void setUserDefinedData(void *data)
	{
		userDefinedData = data;
	}

	errorFuncType *setErrorFuncCallback(errorFuncType *newFunc)
	{
		auto a = errorFunc;
		errorFunc = newFunc;
		return a;
	}

	namespace internal
	{
		float positionToScreenCoordsX(const float position, float w)
		{
			return (position / w) * 2 - 1;
		}

		float positionToScreenCoordsY(const float position, float h)
		{
			return -((-position / h) * 2 - 1);
		}

		stbtt_aligned_quad fontGetGlyphQuad(const Font font, const char c)
		{
			stbtt_aligned_quad quad = {0};

			float x = 0;
			float y = 0;

			stbtt_GetPackedQuad(font.packedCharsBuffer,
				font.size.x, font.size.y, c - ' ', &x, &y, &quad, 1);


			return quad;
		}

		glm::vec4 fontGetGlyphTextureCoords(const Font font, const char c)
		{
			float xoffset = 0;
			float yoffset = 0;

			const stbtt_aligned_quad quad = fontGetGlyphQuad(font, c);

			return glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1};
		}

		GLuint loadShader(const char *source, GLenum shaderType)
		{
			GLuint id = glCreateShader(shaderType);

			glShaderSource(id, 1, &source, 0);
			glCompileShader(id);

			int result = 0;
			glGetShaderiv(id, GL_COMPILE_STATUS, &result);

			if (!result)
			{
				char *message = 0;
				int   l = 0;

				glGetShaderiv(id, GL_INFO_LOG_LENGTH, &l);

				message = new char[l];

				glGetShaderInfoLog(id, l, &l, message);

				message[l - 1] = 0;

				errorFunc(message, userDefinedData);

				delete[] message;

			}

			return id;
		}

	}

#ifdef _WIN32
	typedef BOOL(WINAPI *PFNWGLSWAPINTERVALEXTPROC) (int interval);
#else
	typedef bool(*PFNWGLSWAPINTERVALEXTPROC) (int interval);
#endif

	struct
	{
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	}extensions = {};

	bool hasInitialized = 0;
	void init()
	{
		if (hasInitialized) { return; }
		hasInitialized = true;

		if (!glGenTextures)
		{
			errorFunc("OpenGL doesn't \
seem to be initialized, have you forgotten to call gladLoadGL() \
or gladLoadGLLoader() or glewInit()?", userDefinedData);
		}


		//int last = 0;
		//glGetIntegerv(GL_NUM_EXTENSIONS, &last);
		//for(int i=0; i<last; i++)
		//{
		//	const char *c = (const char*)glGetStringi(GL_EXTENSIONS, i);
		//	if(strcmp(c, "WGL_EXT_swap_control") == 0)
		//	{
		//		extensions.WGL_EXT_swap_control_ext = true;
		//		break;
		//	}
		//}

	#ifdef _WIN32
		//add linux suport

		//if you are not using visual studio make shure you link to "Opengl32.lib"
		extensions.wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	#endif

		defaultShader = createShaderProgram(defaultVertexShader, defaultFragmentShader);
		white1pxSquareTexture.create1PxSquare();

		enableNecessaryGLFeatures();
	}

	void cleanup()
	{
		white1pxSquareTexture.cleanup();
		defaultShader.clear();
		hasInitialized = false;
	}

	bool setVsync(bool b)
	{
		//add linux suport
		if (extensions.wglSwapIntervalEXT != nullptr)
		{
			bool rezult = extensions.wglSwapIntervalEXT(b);
			return rezult;
		}
		else
		{
			return false;
		}
	}

	glm::vec2 rotateAroundPoint(glm::vec2 vec, glm::vec2 point, const float degrees)
	{
		point.y = -point.y;
		float a = glm::radians(degrees);
		float s = sinf(a);
		float c = cosf(a);
		vec.x -= point.x;
		vec.y -= point.y;
		float newx = vec.x * c - vec.y * s;
		float newy = vec.x * s + vec.y * c;
		// translate point back:
		vec.x = newx + point.x;
		vec.y = newy + point.y;
		return vec;
	}

	glm::vec2 scaleAroundPoint(glm::vec2 vec, glm::vec2 point, float scale)
	{
		vec = (vec - point) * scale + point;

		return vec;
	}


	///////////////////// Shader /////////////////////
#pragma region shader

	void validateProgram(GLuint id)
	{
		int info = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &info);

		if (info != GL_TRUE)
		{
			char *message = 0;
			int   l = 0;

			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &l);

			message = new char[l];

			glGetProgramInfoLog(id, l, &l, message);

			errorFunc(message, userDefinedData);

			delete[] message;
		}

		glValidateProgram(id);
	}

	ShaderProgram createShaderProgram(const char *vertex, const char *fragment)
	{
		ShaderProgram shader = {0};

		const GLuint vertexId = internal::loadShader(vertex, GL_VERTEX_SHADER);
		const GLuint fragmentId = internal::loadShader(fragment, GL_FRAGMENT_SHADER);

		shader.id = glCreateProgram();
		glAttachShader(shader.id, vertexId);
		glAttachShader(shader.id, fragmentId);

		glBindAttribLocation(shader.id, 0, "quad_positions");
		glBindAttribLocation(shader.id, 1, "quad_colors");
		glBindAttribLocation(shader.id, 2, "texturePositions");

		glLinkProgram(shader.id);

		glDeleteShader(vertexId);
		glDeleteShader(fragmentId);

		validateProgram(shader.id);

		shader.u_sampler = glGetUniformLocation(shader.id, "u_sampler");
		shader.u_viewProjection = glGetUniformLocation(shader.id, "u_viewProjection");

		return shader;
	}

	ShaderProgram createShaderFromFile(const char *filePath)
	{
		std::ifstream fileFont(filePath, std::ios::binary);

		if (!fileFont.is_open())
		{
			std::string e = "error openning: "; e += filePath;
			errorFunc(e.c_str(), userDefinedData);
			return {};
		}

		int fileSize = 0;
		fileFont.seekg(0, std::ios::end);
		fileSize = (int)fileFont.tellg();
		fileFont.seekg(0, std::ios::beg);
		char *fileData = new char[fileSize + 1]; //null terminated
		fileFont.read((char *)fileData, fileSize);
		fileFont.close();
		fileData[fileSize] = 0; //null terminated

		auto rez = createShader(fileData);

		delete[] fileData;

		return rez;
	}

	ShaderProgram createShader(const char *fragment)
	{
		return createShaderProgram(defaultVertexShader, fragment);
	}

	ShaderProgram createPostProcessShaderFromFile(const char *filePath)
	{
		std::ifstream fileFont(filePath, std::ios::binary);

		if (!fileFont.is_open())
		{
			std::string e = "error openning: "; e += filePath;
			errorFunc(e.c_str(), userDefinedData);
			return {};
		}

		int fileSize = 0;
		fileFont.seekg(0, std::ios::end);
		fileSize = (int)fileFont.tellg();
		fileFont.seekg(0, std::ios::beg);
		char *fileData = new char[fileSize + 1]; //null terminated
		fileFont.read((char *)fileData, fileSize);
		fileFont.close();
		fileData[fileSize] = 0; //null terminated

		auto rez = createPostProcessShader(fileData);

		delete[] fileData;

		return rez;
	}

	ShaderProgram createPostProcessShader(const char *fragment)
	{
		return createShaderProgram(defaultVertexPostProcessShader, fragment);
	}

#pragma endregion

	///////////////////// Texture /////////////////////
#pragma region Texture

	void convertFromRetardedCoordonates(int tSizeX, int tSizeY, int x, int y, int sizeX, int sizeY, int s1, int s2, int s3, int s4, Texture_Coords *outer, Texture_Coords *inner)
	{
		float newX = (float)tSizeX / (float)x;
		float newY = (float)tSizeY / (float)y;
		newY = 1 - newY;

		float newSizeX = (float)tSizeX / (float)sizeX;
		float newSizeY = (float)tSizeY / (float)sizeY;

		if (outer)
		{
			outer->x = newX;
			outer->y = newY;
			outer->z = newX + newSizeX;
			outer->w = newY - newSizeY;
		}

		if (inner)
		{
			inner->x = newX + ((float)s1 / tSizeX);
			inner->y = newY - ((float)s2 / tSizeY);
			inner->z = newX + newSizeX - ((float)s3 / tSizeX);
			inner->w = newY - newSizeY + ((float)s4 / tSizeY);
		}

	}

#pragma endregion

	///////////////////// Font /////////////////////
#pragma	region Font

	void Font::createFromTTF(const unsigned char *ttf_data, const size_t ttf_data_size,
		bool monospaced)
	{
		this->monospaced = monospaced;

		size.x = 2000;
		size.y = 2000;
		max_height = 0;
		packedCharsBufferSize = ('~' - ' ');

		// Initialize stbtt_fontinfo to get font metrics
		stbtt_fontinfo fontInfo;
		stbtt_InitFont(&fontInfo, ttf_data, stbtt_GetFontOffsetForIndex(ttf_data, 0));

		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

		float scale = stbtt_ScaleForPixelHeight(&fontInfo, 64); // Match font size used in PackFontRange
		max_height = (ascent - descent + lineGap) * scale;



		// STB TrueType will give us a one channel buffer of the font that we then convert to RGBA for OpenGL
		const size_t fontMonochromeBufferSize = size.x * size.y;
		const size_t fontRgbaBufferSize = size.x * size.y * 4;

		unsigned char *fontMonochromeBuffer = new unsigned char[fontMonochromeBufferSize];
		unsigned char *fontRgbaBuffer = new unsigned char[fontRgbaBufferSize];

		packedCharsBuffer = new stbtt_packedchar[packedCharsBufferSize]{};

		stbtt_pack_context stbtt_context;
		stbtt_PackBegin(&stbtt_context, fontMonochromeBuffer, size.x, size.y, 0, 2, NULL);
		stbtt_PackSetOversampling(&stbtt_context, 2, 2);
		stbtt_PackFontRange(&stbtt_context, ttf_data, 0, 64, ' ', '~' - ' ', packedCharsBuffer);
		stbtt_PackEnd(&stbtt_context);

		// Convert monochrome buffer to RGBA
		for (int i = 0; i < fontMonochromeBufferSize; i++)
		{
			fontRgbaBuffer[(i * 4)] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 1] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 2] = fontMonochromeBuffer[i];
			fontRgbaBuffer[(i * 4) + 3] = fontMonochromeBuffer[i] > 1 ? 255 : 0;
		}

		// Init texture
		glGenTextures(1, &texture.id);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontRgbaBuffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


		//if (monospaced)
		//{
		//	for (char c = ' '; c <= '~'; c++)
		//	{
		//		stbtt_packedchar &glyph = packedCharsBuffer[c - ' '];
		//		float charWidth = glyph.x1 - glyph.x0;
		//		if (charWidth > max_height)
		//			max_height = charWidth;
		//	}
		//}

		if (monospaced)
		{
			spaceSize = max_height;
		}
		else
		{
			spaceSize = packedCharsBuffer[' ' - ' '].xadvance;
		}


		delete[] fontMonochromeBuffer;
		delete[] fontRgbaBuffer;
	}

	void Font::createFromFile(const char *file, bool monospaced)
	{
		std::ifstream fileFont(file, std::ios::binary);

		if (!fileFont.is_open())
		{
			char c[300] = {0};
			strcat(c, "error openning: ");
			strcat(c + strlen(c), file);
			errorFunc(c, userDefinedData);
			return;
		}

		int fileSize = 0;
		fileFont.seekg(0, std::ios::end);
		fileSize = (int)fileFont.tellg();
		fileFont.seekg(0, std::ios::beg);
		unsigned char *fileData = new unsigned char[fileSize];
		fileFont.read((char *)fileData, fileSize);
		fileFont.close();

		createFromTTF(fileData, fileSize, monospaced);

		delete[] fileData;
	}

	void Font::cleanup()
	{
		texture.cleanup();
		*this = {};
	}


#pragma endregion

	///////////////////// Camera /////////////////////
#pragma region Camera


	glm::mat4x4 gl2d::Camera3D::getProjectionMatrix()
	{
		if (std::isinf(this->aspectRatio) || std::isnan(this->aspectRatio) || this->aspectRatio == 0)
		{
			return glm::mat4x4(1.f);
		}

		auto mat = glm::perspective(this->fovRadians, this->aspectRatio, this->closePlane,
			this->farPlane);

		return mat;
	}

	glm::mat4x4 gl2d::Camera3D::getViewMatrix()
	{

		glm::vec3 dir = glm::vec3(position) + glm::vec3(viewDirection);
		return  glm::lookAt(glm::vec3(position), dir, up);
	}

	glm::mat4x4 gl2d::Camera3D::getViewProjectionMatrix()
	{
		return getProjectionMatrix() * getViewMatrix();
	}

	void gl2d::Camera3D::rotateCamera(const glm::vec2 delta)
	{

		constexpr float PI = 3.1415926;

		yaw += delta.x;
		pitch += delta.y;

		if (yaw >= 2 * PI)
		{
			yaw -= 2 * PI;
		}
		else if (yaw < 0)
		{
			yaw = 2 * PI - yaw;
		}

		if (pitch > PI / 2.f - 0.01) { pitch = PI / 2.f - 0.01; }
		if (pitch < -PI / 2.f + 0.01) { pitch = -PI / 2.f + 0.01; }

		viewDirection = glm::vec3(0, 0, -1);

		viewDirection = glm::mat3(glm::rotate(yaw, up)) * viewDirection;

		glm::vec3 rotatePitchAxe = glm::cross(viewDirection, up);
		viewDirection = glm::mat3(glm::rotate(pitch, rotatePitchAxe)) * viewDirection;
	}

	void gl2d::Camera3D::moveFPS(glm::vec3 direction)
	{
		viewDirection = glm::normalize(viewDirection);

		//forward
		float forward = -direction.z;
		float leftRight = direction.x;
		float upDown = direction.y;

		glm::vec3 move = {};

		move += up * upDown;
		move += glm::normalize(glm::cross(viewDirection, up)) * leftRight;
		move += viewDirection * forward;

		this->position += move;
	}

	void gl2d::Camera3D::rotateFPS(glm::ivec2 mousePos, float speed)
	{
		glm::vec2 delta = lastMousePos - mousePos;
		delta *= speed;

		rotateCamera(delta);

		lastMousePos = mousePos;
	}



#pragma endregion

	///////////////////// Renderer2D /////////////////////
#pragma region Renderer2D

	//won't bind any fbo
	void internalFlush(gl2d::Renderer2D &renderer, bool clearDrawData)
	{
		enableNecessaryGLFeatures();

		if (!hasInitialized)
		{
			errorFunc("Library not initialized. Have you forgotten to call gl2d::init() ?", userDefinedData);
			renderer.clearDrawData();
			return;
		}

		if (!renderer.vao)
		{
			errorFunc("Renderer not initialized. Have you forgotten to call gl2d::Renderer2D::create() ?", userDefinedData);
			renderer.clearDrawData();
			return;
		}

		if (renderer.windowH == 0 || renderer.windowW == 0)
		{
			if (clearDrawData)
			{
				renderer.clearDrawData();
			}

			return;
		}

		if (renderer.windowH < 0 || renderer.windowW < 0)
		{
			if (clearDrawData)
			{
				renderer.clearDrawData();
			}

			errorFunc("Negative windowW or windowH, have you forgotten to call updateWindowMetrics(w, h)?", userDefinedData);

			return;
		}

		if (renderer.spriteTextures.empty())
		{
			return;
		}

		glViewport(0, 0, renderer.windowW, renderer.windowH);

		glBindVertexArray(renderer.vao);

		glUseProgram(renderer.currentShader.id);

		glUniform1i(renderer.currentShader.u_sampler, 0);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::quadPositions]);
		glBufferData(GL_ARRAY_BUFFER, renderer.spritePositions.size() * sizeof(glm::vec4), renderer.spritePositions.data(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::quadColors]);
		glBufferData(GL_ARRAY_BUFFER, renderer.spriteColors.size() * sizeof(glm::vec4), renderer.spriteColors.data(), GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::texturePositions]);
		glBufferData(GL_ARRAY_BUFFER, renderer.texturePositions.size() * sizeof(glm::vec2), renderer.texturePositions.data(), GL_STREAM_DRAW);

		bool use3D = false;

		//todo 2 different shaders to properly optimize this
		if (renderer.currentShader.u_viewProjection >= 0)
		{
			if (renderer.currentCamera3D.use)
			{
				//renderer.currentCamera3D.aspectRatio = (float)renderer.windowW / renderer.windowH;
				renderer.currentCamera3D.aspectRatio = 1;

				glUniformMatrix4fv(renderer.currentShader.u_viewProjection, 1, GL_FALSE, &renderer.currentCamera3D.getViewProjectionMatrix()[0][0]);
				use3D = true;
			}
			else
			{
				glUniformMatrix4fv(renderer.currentShader.u_viewProjection, 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
			}

		}

		GLint oldDepthFunc = 0;
		GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

		if (use3D)
		{
			glEnable(GL_DEPTH_TEST);

			glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
			glDepthFunc(GL_LEQUAL);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}

		//Instance render the textures
		{
			const int size = renderer.spriteTextures.size();
			int pos = 0;
			unsigned int id = renderer.spriteTextures[0].id;

			renderer.spriteTextures[0].bind();

			for (int i = 1; i < size; i++)
			{
				if (renderer.spriteTextures[i].id != id)
				{
					glDrawArrays(GL_TRIANGLES, pos * 6, 6 * (i - pos));

					pos = i;
					id = renderer.spriteTextures[i].id;

					renderer.spriteTextures[i].bind();
				}

			}

			glDrawArrays(GL_TRIANGLES, pos * 6, 6 * (size - pos));

			glBindVertexArray(0);
		}

		if (use3D)
		{
			glDepthFunc(oldDepthFunc);
			if (depthTestEnabled)
				glEnable(GL_DEPTH_TEST);
			if (!depthTestEnabled)
				glDisable(GL_DEPTH_TEST);
		}

		if (clearDrawData)
		{
			renderer.clearDrawData();
		}
	}

	void gl2d::Renderer2D::flush(bool clearDrawData)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
		internalFlush(*this, clearDrawData);
	}

	void Renderer2D::flushFBO(FrameBuffer frameBuffer, bool clearDrawData)
	{
		if (frameBuffer.fbo == 0)
		{
			errorFunc("Framebuffer not initialized", userDefinedData);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.fbo);
		glBindTexture(GL_TEXTURE_2D, 0); //todo investigate and remove

		internalFlush(*this, clearDrawData);

		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
	}

	void Renderer2D::renderFrameBufferToTheEntireScreen(gl2d::FrameBuffer fbo, gl2d::FrameBuffer screen)
	{
		renderTextureToTheEntireScreen(fbo.texture, screen);
	}

	//doesn't bind or unbind stuff, except the vertex array,
	//doesn't set the viewport
	void renderQuadToScreenInternal(gl2d::Renderer2D &renderer)
	{
		static float positions[26] = {
		-1, 1,0,1,
		-1, -1,0,1,
		1, 1,0,1,

		1, 1,0,1,
		-1, -1,0,1,
		1, -1,0,1};

		//not used
		static float colors[6 * 4] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};
		static float texCoords[12] = {
			0, 1,
			0, 0,
			1, 1,

			1, 1,
			0, 0,
			1, 0,
		};

		enableNecessaryGLFeatures();
		

		glBindVertexArray(renderer.vao);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::quadPositions]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), positions, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::quadColors]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), colors, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, renderer.buffers[Renderer2DBufferType::texturePositions]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), texCoords, GL_STREAM_DRAW);

		{
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	void Renderer2D::renderTextureToTheEntireScreen(gl2d::Texture t, gl2d::FrameBuffer screen)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, screen.fbo);

		enableNecessaryGLFeatures();

		if (!hasInitialized)
		{
			errorFunc("Library not initialized. Have you forgotten to call gl2d::init() ?", userDefinedData);
			return;
		}

		if (!vao)
		{
			errorFunc("Renderer not initialized. Have you forgotten to call gl2d::Renderer2D::create() ?", userDefinedData);
			return;
		}

		if (!currentShader.id)
		{
			errorFunc("Post Process Shader not created.", userDefinedData);
			return;
		}

		glm::ivec2 size = {windowW, windowH};

		if (screen.fbo)
		{
			size = screen.texture.GetSize();
		}
		if (size.x == 0 || size.y == 0)
		{
			return;
		}

		glViewport(0, 0, size.x, size.y);

		glUseProgram(currentShader.id);
		glUniform1i(currentShader.u_sampler, 0);
		t.bind();

		renderQuadToScreenInternal(*this);

		glBindVertexArray(0);

	}

	void Renderer2D::flushPostProcess(const std::vector<ShaderProgram> &postProcesses,
		FrameBuffer frameBuffer, bool clearDrawData)
	{

		if (frameBuffer.fbo == 0)
		{
			frameBuffer.fbo = defaultFBO;
		}

		if (postProcesses.empty())
		{
			if (clearDrawData)
			{
				this->clearDrawData();
				return;
			}
		}

		if (!postProcessFbo1.fbo) { postProcessFbo1.create(windowW, windowH); }

		postProcessFbo1.resize(windowW, windowH);
		postProcessFbo1.clear();

		flushFBO(postProcessFbo1, clearDrawData);

		internalPostProcessFlip = 1;
		postProcessOverATexture(postProcesses, postProcessFbo1.texture, frameBuffer);

	}

	void Renderer2D::postProcessOverATexture(const std::vector<ShaderProgram> &postProcesses,
		gl2d::Texture in,
		FrameBuffer frameBuffer)
	{
		if (postProcesses.empty())
		{
			return;
		}


		if (!postProcessFbo1.fbo) { postProcessFbo1.create(0, 0); }
		if (!postProcessFbo2.fbo && postProcesses.size() > 1)
		{
			postProcessFbo2.create(0, 0);
		}

		if (internalPostProcessFlip == 0)
		{
			postProcessFbo1.resize(windowW, windowH);
			postProcessFbo1.clear();
			postProcessFbo2.resize(windowW, windowH);
			postProcessFbo2.clear();
		}
		else if (postProcessFbo2.fbo)
		{
			//postProcessFbo1 has already been resized
			postProcessFbo2.resize(windowW, windowH);
			postProcessFbo2.clear();
		}

		for (int i = 0; i < postProcesses.size(); i++)
		{
			gl2d::FrameBuffer output;
			gl2d::Texture input;

			if (internalPostProcessFlip == 0)
			{
				input = postProcessFbo2.texture;
				output = postProcessFbo1;
			}
			else
			{
				input = postProcessFbo1.texture;
				output = postProcessFbo2;
			}

			if (i == 0)
			{
				input = in;
			}

			if (i == postProcesses.size() - 1)
			{
				output = frameBuffer;
			}
			output.clear();

			renderPostProcess(postProcesses[i], input, output);
			internalPostProcessFlip = !internalPostProcessFlip;
		}


		internalPostProcessFlip = 0;
	}

	void enableNecessaryGLFeatures()
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	///////////////////// Renderer2D - render ///////////////////// 

	void Renderer2D::renderRectangle(const Rect transforms, const Texture texture, 
		const Color4f colors[4], const glm::vec2 origin, const float rotation, const glm::vec4 textureCoords, float positionZ)
	{
		glm::vec2 newOrigin;
		newOrigin.x = origin.x + transforms.x + (transforms.z / 2);
		newOrigin.y = origin.y + transforms.y + (transforms.w / 2);
		renderRectangleAbsRotation(transforms, texture, colors, newOrigin, rotation, textureCoords, positionZ);
	}

	void gl2d::Renderer2D::renderRectangleAbsRotation(const Rect transforms,
		const Texture texture, const Color4f colors[4], const glm::vec2 origin, const float rotation, const glm::vec4 textureCoords, float positionZ)
	{
		Texture textureCopy = texture;

		if (textureCopy.id == 0)
		{
			errorFunc("Invalid texture", userDefinedData);
			textureCopy = white1pxSquareTexture;
		}

		//We need to flip texture_transforms.y
		const float transformsY = transforms.y * -1;

		glm::vec2 v1 = {transforms.x,				  transformsY};
		glm::vec2 v2 = {transforms.x,				  transformsY - transforms.w};
		glm::vec2 v3 = {transforms.x + transforms.z, transformsY - transforms.w};
		glm::vec2 v4 = {transforms.x + transforms.z, transformsY};

		//Apply rotations
		if (rotation != 0)
		{
			v1 = rotateAroundPoint(v1, origin, rotation);
			v2 = rotateAroundPoint(v2, origin, rotation);
			v3 = rotateAroundPoint(v3, origin, rotation);
			v4 = rotateAroundPoint(v4, origin, rotation);
		}

		//Apply camera transformations
		v1.x -= currentCamera.position.x;
		v1.y += currentCamera.position.y;
		v2.x -= currentCamera.position.x;
		v2.y += currentCamera.position.y;
		v3.x -= currentCamera.position.x;
		v3.y += currentCamera.position.y;
		v4.x -= currentCamera.position.x;
		v4.y += currentCamera.position.y;

		//Apply camera rotation
		if (currentCamera.rotation != 0)
		{
			glm::vec2 cameraCenter;

			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = windowH / 2.0f;

			v1 = rotateAroundPoint(v1, cameraCenter, currentCamera.rotation);
			v2 = rotateAroundPoint(v2, cameraCenter, currentCamera.rotation);
			v3 = rotateAroundPoint(v3, cameraCenter, currentCamera.rotation);
			v4 = rotateAroundPoint(v4, cameraCenter, currentCamera.rotation);
		}

		//Apply camera zoom
		//if(renderer->currentCamera.zoom != 1)
		{

			glm::vec2 cameraCenter;
			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = -windowH / 2.0f;

			v1 = scaleAroundPoint(v1, cameraCenter, currentCamera.zoom);
			v2 = scaleAroundPoint(v2, cameraCenter, currentCamera.zoom);
			v3 = scaleAroundPoint(v3, cameraCenter, currentCamera.zoom);
			v4 = scaleAroundPoint(v4, cameraCenter, currentCamera.zoom);
		}

		v1.x = internal::positionToScreenCoordsX(v1.x, (float)windowW);
		v2.x = internal::positionToScreenCoordsX(v2.x, (float)windowW);
		v3.x = internal::positionToScreenCoordsX(v3.x, (float)windowW);
		v4.x = internal::positionToScreenCoordsX(v4.x, (float)windowW);
		v1.y = internal::positionToScreenCoordsY(v1.y, (float)windowH);
		v2.y = internal::positionToScreenCoordsY(v2.y, (float)windowH);
		v3.y = internal::positionToScreenCoordsY(v3.y, (float)windowH);
		v4.y = internal::positionToScreenCoordsY(v4.y, (float)windowH);

		spritePositions.push_back(glm::vec4{v1.x, v1.y, positionZ, 1});
		spritePositions.push_back(glm::vec4{v2.x, v2.y, positionZ, 1});
		spritePositions.push_back(glm::vec4{v4.x, v4.y, positionZ, 1});

		spritePositions.push_back(glm::vec4{v2.x, v2.y, positionZ, 1});
		spritePositions.push_back(glm::vec4{v3.x, v3.y, positionZ, 1});
		spritePositions.push_back(glm::vec4{v4.x, v4.y, positionZ, 1});

		spriteColors.push_back(colors[0]);
		spriteColors.push_back(colors[1]);
		spriteColors.push_back(colors[3]);
		spriteColors.push_back(colors[1]);
		spriteColors.push_back(colors[2]);
		spriteColors.push_back(colors[3]);

		texturePositions.push_back(glm::vec2{textureCoords.x, textureCoords.y}); //1
		texturePositions.push_back(glm::vec2{textureCoords.x, textureCoords.w}); //2
		texturePositions.push_back(glm::vec2{textureCoords.z, textureCoords.y}); //4
		texturePositions.push_back(glm::vec2{textureCoords.x, textureCoords.w}); //2
		texturePositions.push_back(glm::vec2{textureCoords.z, textureCoords.w}); //3
		texturePositions.push_back(glm::vec2{textureCoords.z, textureCoords.y}); //4

		spriteTextures.push_back(textureCopy);
	}

	void Renderer2D::renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, float positionZ)
	{
		renderRectangle(transforms, white1pxSquareTexture, colors, origin, rotation, GL2D_DefaultTextureCoords, positionZ);
	}

	void Renderer2D::renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, float positionZ)
	{
		renderRectangleAbsRotation(transforms, white1pxSquareTexture, colors, origin, rotation, GL2D_DefaultTextureCoords,  positionZ);
	}

	void Renderer2D::renderLine(const glm::vec2 position, const float angleDegrees, const float length, const Color4f color, const float width, float positionZ)
	{
		renderRectangle({position - glm::vec2(0,width / 2.f), length, width},
			color, {-length / 2, 0}, angleDegrees, positionZ);
	}

	void Renderer2D::renderLine(const glm::vec2 start, const glm::vec2 end, const Color4f color, const float width, float positionZ)
	{
		glm::vec2 vector = end - start;
		float length = glm::length(vector);
		float angle = std::atan2(vector.y, vector.x);
		renderLine(start, -glm::degrees(angle), length, color, width, positionZ);
	}

	void Renderer2D::renderRectangleOutline(const glm::vec4 position, const Color4f color, const float width,
		const glm::vec2 origin, const float rotationDegrees, float positionZ)
	{

		glm::vec2 topLeft = position;
		glm::vec2 topRight = glm::vec2(position) + glm::vec2(position.z, 0);
		glm::vec2 bottomLeft = glm::vec2(position) + glm::vec2(0, position.w);
		glm::vec2 bottomRight = glm::vec2(position) + glm::vec2(position.z, position.w);

		glm::vec2 p1 = topLeft + glm::vec2(-width / 2.f, 0);
		glm::vec2 p2 = topRight + glm::vec2(+width / 2.f, 0);
		glm::vec2 p3 = topRight + glm::vec2(0, +width / 2.f);
		glm::vec2 p4 = bottomRight + glm::vec2(0, -width / 2.f);
		glm::vec2 p5 = bottomRight + glm::vec2(width / 2.f, 0);
		glm::vec2 p6 = bottomLeft + glm::vec2(-width / 2.f, 0);
		glm::vec2 p7 = bottomLeft + glm::vec2(0, -width / 2.f);
		glm::vec2 p8 = topLeft + glm::vec2(0, +width / 2.f);

		if (rotationDegrees != 0)
		{
			glm::vec2 o = origin + glm::vec2(position.x, -position.y) + glm::vec2(position.z, -position.w) / 2.f;

			p1 = rotateAroundPoint(p1, o, -rotationDegrees);
			p2 = rotateAroundPoint(p2, o, -rotationDegrees);
			p3 = rotateAroundPoint(p3, o, -rotationDegrees);
			p4 = rotateAroundPoint(p4, o, -rotationDegrees);
			p5 = rotateAroundPoint(p5, o, -rotationDegrees);
			p6 = rotateAroundPoint(p6, o, -rotationDegrees);
			p7 = rotateAroundPoint(p7, o, -rotationDegrees);
			p8 = rotateAroundPoint(p8, o, -rotationDegrees);
		}

		auto renderPoint = [&](glm::vec2 pos)
		{
			renderRectangle({pos - glm::vec2(1,1),width,width}, Colors_Black, {}, 0, positionZ);
		};

		//renderPoint(p1);
		//renderPoint(p2);
		//renderPoint(p3);
		//renderPoint(p4);
		//renderPoint(p5);
		//renderPoint(p6);
		//renderPoint(p7);
		//renderPoint(p8);

		//add a padding so the lines align properly.
		renderLine(p1, p2, color, width, positionZ); //top line
		renderLine(p3, p4, color, width, positionZ);
		renderLine(p5, p6, color, width, positionZ); //bottom line
		renderLine(p7, p8, color, width, positionZ);

	}

	void  Renderer2D::renderCircleOutline(const glm::vec2 position,
		const float size, const Color4f color,
		const float width, const unsigned int segments, float positionZ)
	{

		auto calcPos = [&](int p)
		{
			glm::vec2 circle = {size,0};

			float a = 3.1415926 * 2 * ((float)p / segments);

			float c = std::cos(a);
			float s = std::sin(a);

			circle = {c * circle.x - s * circle.y, s * circle.x + c * circle.y};

			return circle + position;
		};


		glm::vec2 lastPos = calcPos(1);
		renderLine(calcPos(0), lastPos, color, width, positionZ);
		for (int i = 1; i < segments; i++)
		{

			glm::vec2 pos1 = lastPos;
			glm::vec2 pos2 = calcPos(i + 1);

			renderLine(pos1, pos2, color, width, positionZ);

			lastPos = pos2;
		}

	}



	void Renderer2D::render9Patch(const Rect position, const int borderSize, 
		const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, 
		const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords, float positionZ)
	{
		glm::vec4 colorData[4] = {color, color, color, color};

		//inner
		Rect innerPos = position;
		innerPos.x += borderSize;
		innerPos.y += borderSize;
		innerPos.z -= borderSize * 2;
		innerPos.w -= borderSize * 2;
		renderRectangle(innerPos, texture, colorData, Position2D{0, 0}, 0, inner_texture_coords, positionZ);

		//top
		Rect topPos = position;
		topPos.x += borderSize;
		topPos.z -= (float)borderSize * 2;
		topPos.w = (float)borderSize;
		glm::vec4 upperTexPos;
		upperTexPos.x = inner_texture_coords.x;
		upperTexPos.y = textureCoords.y;
		upperTexPos.z = inner_texture_coords.z;
		upperTexPos.w = inner_texture_coords.y;
		renderRectangle(topPos, texture, colorData, Position2D{0, 0}, 0, upperTexPos, positionZ);

		//bottom
		Rect bottom = position;
		bottom.x += (float)borderSize;
		bottom.y += (float)position.w - borderSize;
		bottom.z -= (float)borderSize * 2;
		bottom.w = (float)borderSize;
		glm::vec4 bottomTexPos;
		bottomTexPos.x = inner_texture_coords.x;
		bottomTexPos.y = inner_texture_coords.w;
		bottomTexPos.z = inner_texture_coords.z;
		bottomTexPos.w = textureCoords.w;
		renderRectangle(bottom, texture, colorData, Position2D{0, 0}, 0, bottomTexPos, positionZ);

		//left
		Rect left = position;
		left.y += borderSize;
		left.z = (float)borderSize;
		left.w -= (float)borderSize * 2;
		glm::vec4 leftTexPos;
		leftTexPos.x = textureCoords.x;
		leftTexPos.y = inner_texture_coords.y;
		leftTexPos.z = inner_texture_coords.x;
		leftTexPos.w = inner_texture_coords.w;
		renderRectangle(left, texture, colorData, Position2D{0, 0}, 0, leftTexPos, positionZ);

		//right
		Rect right = position;
		right.x += position.z - borderSize;
		right.y += borderSize;
		right.z = (float)borderSize;
		right.w -= (float)borderSize * 2;
		glm::vec4 rightTexPos;
		rightTexPos.x = inner_texture_coords.z;
		rightTexPos.y = inner_texture_coords.y;
		rightTexPos.z = textureCoords.z;
		rightTexPos.w = inner_texture_coords.w;
		renderRectangle(right, texture, colorData, Position2D{0, 0}, 0, rightTexPos, positionZ);

		//topleft
		Rect topleft = position;
		topleft.z = (float)borderSize;
		topleft.w = (float)borderSize;
		glm::vec4 topleftTexPos;
		topleftTexPos.x = textureCoords.x;
		topleftTexPos.y = textureCoords.y;
		topleftTexPos.z = inner_texture_coords.x;
		topleftTexPos.w = inner_texture_coords.y;
		renderRectangle(topleft, texture, colorData, Position2D{0, 0}, 0, topleftTexPos, positionZ);

		//topright
		Rect topright = position;
		topright.x += position.z - borderSize;
		topright.z = (float)borderSize;
		topright.w = (float)borderSize;
		glm::vec4 toprightTexPos;
		toprightTexPos.x = inner_texture_coords.z;
		toprightTexPos.y = textureCoords.y;
		toprightTexPos.z = textureCoords.z;
		toprightTexPos.w = inner_texture_coords.y;
		renderRectangle(topright, texture, colorData, Position2D{0, 0}, 0, toprightTexPos, positionZ);

		//bottomleft
		Rect bottomleft = position;
		bottomleft.y += position.w - borderSize;
		bottomleft.z = (float)borderSize;
		bottomleft.w = (float)borderSize;
		glm::vec4 bottomleftTexPos;
		bottomleftTexPos.x = textureCoords.x;
		bottomleftTexPos.y = inner_texture_coords.w;
		bottomleftTexPos.z = inner_texture_coords.x;
		bottomleftTexPos.w = textureCoords.w;
		renderRectangle(bottomleft, texture, colorData, Position2D{0, 0}, 0, bottomleftTexPos, positionZ);

		//bottomright
		Rect bottomright = position;
		bottomright.y += position.w - borderSize;
		bottomright.x += position.z - borderSize;
		bottomright.z = (float)borderSize;
		bottomright.w = (float)borderSize;
		glm::vec4 bottomrightTexPos;
		bottomrightTexPos.x = inner_texture_coords.z;
		bottomrightTexPos.y = inner_texture_coords.w;
		bottomrightTexPos.z = textureCoords.z;
		bottomrightTexPos.w = textureCoords.w;
		renderRectangle(bottomright, texture, colorData, Position2D{0, 0}, 0, bottomrightTexPos, positionZ);

	}

	void Renderer2D::render9Patch2(const Rect position, const Color4f color, const glm::vec2 origin, 
		const float rotation, const Texture texture, const Texture_Coords textureCoords, 
		const Texture_Coords inner_texture_coords, float positionZ)
	{
		glm::vec4 colorData[4] = {color, color, color, color};

		int w = 0;
		int h = 0;
		glBindTexture(GL_TEXTURE_2D, texture.id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		float textureSpaceW = textureCoords.z - textureCoords.x;
		float textureSpaceH = textureCoords.y - textureCoords.w;

		float topBorder = (textureCoords.y - inner_texture_coords.y) / textureSpaceH * position.w;
		float bottomBorder = (inner_texture_coords.w - textureCoords.w) / textureSpaceH * position.w;
		float leftBorder = (inner_texture_coords.x - textureCoords.x) / textureSpaceW * position.z;
		float rightBorder = (textureCoords.z - inner_texture_coords.z) / textureSpaceW * position.z;

		float newAspectRatio = position.z / position.w;

		if (newAspectRatio < 1.f)
		{
			topBorder *= newAspectRatio;
			bottomBorder *= newAspectRatio;
		}
		else
		{
			leftBorder /= newAspectRatio;
			rightBorder /= newAspectRatio;
		}



		//topBorder = 50;
		//bottomBorder = -50;
		//leftBorder = 0;
		//rightBorder = 0;


		//inner
		Rect innerPos = position;
		innerPos.x += leftBorder;
		innerPos.y += topBorder;
		innerPos.z -= leftBorder + rightBorder;
		innerPos.w -= topBorder + bottomBorder;
		renderRectangle(innerPos, texture, colorData, Position2D{0, 0}, 0, inner_texture_coords, positionZ);

		//top
		Rect topPos = position;
		topPos.x += leftBorder;
		topPos.z -= leftBorder + rightBorder;
		topPos.w = topBorder;
		glm::vec4 upperTexPos;
		upperTexPos.x = inner_texture_coords.x;
		upperTexPos.y = textureCoords.y;
		upperTexPos.z = inner_texture_coords.z;
		upperTexPos.w = inner_texture_coords.y;
		renderRectangle(topPos, texture, colorData, Position2D{0, 0}, 0, upperTexPos, positionZ);

		//Rect topPos = position;
		//topPos.x += leftBorder;
		//topPos.w = topBorder;
		//topPos.z = topBorder;
		//float end = rightBorder;
		//float size = topBorder;
		//
		//while(1)
		//{
		//	if(topPos.x + size <= end)
		//	{
		//
		//		//draw
		//		renderRectangle(topPos, colorData, Position2D{ 0, 0 }, 0, texture, upperTexPos);
		//
		//		topPos += size;
		//	}else
		//	{
		//		float newW = end - topPos.x;
		//		if(newW>0)
		//		{
		//			topPos.z = newW;
		//			renderRectangle(topPos, colorData, Position2D{ 0, 0 }, 0, texture, upperTexPos);
		//		}
		//		break;
		//	}
		//
		//}


		//bottom
		Rect bottom = position;
		bottom.x += leftBorder;
		bottom.y += (float)position.w - bottomBorder;
		bottom.z -= leftBorder + rightBorder;
		bottom.w = bottomBorder;
		glm::vec4 bottomTexPos;
		bottomTexPos.x = inner_texture_coords.x;
		bottomTexPos.y = inner_texture_coords.w;
		bottomTexPos.z = inner_texture_coords.z;
		bottomTexPos.w = textureCoords.w;
		renderRectangle(bottom, texture, colorData, Position2D{0, 0}, 0, bottomTexPos, positionZ);

		//left
		Rect left = position;
		left.y += topBorder;
		left.z = leftBorder;
		left.w -= topBorder + bottomBorder;
		glm::vec4 leftTexPos;
		leftTexPos.x = textureCoords.x;
		leftTexPos.y = inner_texture_coords.y;
		leftTexPos.z = inner_texture_coords.x;
		leftTexPos.w = inner_texture_coords.w;
		renderRectangle(left, texture, colorData, Position2D{0, 0}, 0, leftTexPos, positionZ);

		//right
		Rect right = position;
		right.x += position.z - rightBorder;
		right.y += topBorder;
		right.z = rightBorder;
		right.w -= topBorder + bottomBorder;
		glm::vec4 rightTexPos;
		rightTexPos.x = inner_texture_coords.z;
		rightTexPos.y = inner_texture_coords.y;
		rightTexPos.z = textureCoords.z;
		rightTexPos.w = inner_texture_coords.w;
		renderRectangle(right, texture, colorData, Position2D{0, 0}, 0, rightTexPos, positionZ);

		//topleft
		Rect topleft = position;
		topleft.z = leftBorder;
		topleft.w = topBorder;
		glm::vec4 topleftTexPos;
		topleftTexPos.x = textureCoords.x;
		topleftTexPos.y = textureCoords.y;
		topleftTexPos.z = inner_texture_coords.x;
		topleftTexPos.w = inner_texture_coords.y;
		renderRectangle(topleft, texture, colorData, Position2D{0, 0}, 0, topleftTexPos, positionZ);
		//repair here?


		//topright
		Rect topright = position;
		topright.x += position.z - rightBorder;
		topright.z = rightBorder;
		topright.w = topBorder;
		glm::vec4 toprightTexPos;
		toprightTexPos.x = inner_texture_coords.z;
		toprightTexPos.y = textureCoords.y;
		toprightTexPos.z = textureCoords.z;
		toprightTexPos.w = inner_texture_coords.y;
		renderRectangle(topright, texture, colorData, Position2D{0, 0}, 0, toprightTexPos, positionZ);

		//bottomleft
		Rect bottomleft = position;
		bottomleft.y += position.w - bottomBorder;
		bottomleft.z = leftBorder;
		bottomleft.w = bottomBorder;
		glm::vec4 bottomleftTexPos;
		bottomleftTexPos.x = textureCoords.x;
		bottomleftTexPos.y = inner_texture_coords.w;
		bottomleftTexPos.z = inner_texture_coords.x;
		bottomleftTexPos.w = textureCoords.w;
		renderRectangle(bottomleft, texture, colorData, Position2D{0, 0}, 0, bottomleftTexPos, positionZ);

		//bottomright
		Rect bottomright = position;
		bottomright.y += position.w - bottomBorder;
		bottomright.x += position.z - rightBorder;
		bottomright.z = rightBorder;
		bottomright.w = bottomBorder;
		glm::vec4 bottomrightTexPos;
		bottomrightTexPos.x = inner_texture_coords.z;
		bottomrightTexPos.y = inner_texture_coords.w;
		bottomrightTexPos.z = textureCoords.z;
		bottomrightTexPos.w = textureCoords.w;
		renderRectangle(bottomright, texture, colorData, Position2D{0, 0}, 0, bottomrightTexPos, positionZ);

	}

	void Renderer2D::create(GLuint fbo, size_t quadCount)
	{
		if (!hasInitialized)
		{
			errorFunc("Library not initialized. Have you forgotten to call gl2d::init() ?", userDefinedData);
		}

		defaultFBO = fbo;

		clearDrawData();
		spritePositions.reserve(quadCount * 6);
		spriteColors.reserve(quadCount * 6);
		texturePositions.reserve(quadCount * 6);
		spriteTextures.reserve(quadCount);

		this->resetCameraAndShader();

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(Renderer2DBufferType::bufferSize, buffers);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadPositions]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::quadColors]);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[Renderer2DBufferType::texturePositions]);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glBindVertexArray(0);
	}

	void Renderer2D::cleanup()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(Renderer2DBufferType::bufferSize, buffers);

		postProcessFbo1.cleanup();
		postProcessFbo2.cleanup();
		internalPostProcessFlip = 0;
	}

	void Renderer2D::pushShader(ShaderProgram s)
	{
		shaderPushPop.push_back(currentShader);
		currentShader = s;
	}

	void Renderer2D::popShader()
	{
		if (shaderPushPop.empty())
		{
			errorFunc("Pop on an empty stack on popShader", userDefinedData);
		}
		else
		{
			currentShader = shaderPushPop.back();
			shaderPushPop.pop_back();
		}
	}

	void Renderer2D::pushCamera(Camera c)
	{
		cameraPushPop.push_back(currentCamera);
		currentCamera = c;
	}

	void Renderer2D::popCamera()
	{
		if (cameraPushPop.empty())
		{
			errorFunc("Pop on an empty stack on popCamera", userDefinedData);
		}
		else
		{
			currentCamera = cameraPushPop.back();
			cameraPushPop.pop_back();
		}
	}

	void Renderer2D::pushCamera3D(Camera3D c)
	{
		camera3DPushPop.push_back(currentCamera3D);
		currentCamera3D = c;
	}

	void Renderer2D::popCamera3D()
	{
		if (camera3DPushPop.empty())
		{
			errorFunc("Pop on an empty stack on popCamera3D", userDefinedData);
		}
		else
		{
			currentCamera3D = camera3DPushPop.back();
			camera3DPushPop.pop_back();
		}
	}

	glm::vec4 Renderer2D::getViewRect()
	{
		auto rect = glm::vec4{0, 0, windowW, windowH};

		glm::mat3 mat =
		{1.f, 0, currentCamera.position.x ,
		 0, 1.f, currentCamera.position.y,
		 0, 0, 1.f};
		mat = glm::transpose(mat);

		glm::vec3 pos1 = {rect.x, rect.y, 1.f};
		glm::vec3 pos2 = {rect.z + rect.x, rect.w + rect.y, 1.f};

		pos1 = mat * pos1;
		pos2 = mat * pos2;

		glm::vec2 point((pos1.x + pos2.x) / 2.f, (pos1.y + pos2.y) / 2.f);

		pos1 = glm::vec3(scaleAroundPoint(pos1, point, 1.f / currentCamera.zoom), 1.f);
		pos2 = glm::vec3(scaleAroundPoint(pos2, point, 1.f / currentCamera.zoom), 1.f);

		rect = {pos1.x, pos1.y, pos2.x - pos1.x, pos2.y - pos1.y};

		return rect;
	}

	glm::vec4 Renderer2D::toScreen(const glm::vec4 &transform)
	{
		//We need to flip texture_transforms.y
		const float transformsY = transform.y * -1;

		glm::vec2 v1 = {transform.x,				  transformsY};
		glm::vec2 v2 = {transform.x,				  transformsY - transform.w};
		glm::vec2 v3 = {transform.x + transform.z, transformsY - transform.w};
		glm::vec2 v4 = {transform.x + transform.z, transformsY};

		//Apply camera transformations
		v1.x -= currentCamera.position.x;
		v1.y += currentCamera.position.y;
		v2.x -= currentCamera.position.x;
		v2.y += currentCamera.position.y;
		v3.x -= currentCamera.position.x;
		v3.y += currentCamera.position.y;
		v4.x -= currentCamera.position.x;
		v4.y += currentCamera.position.y;

		//Apply camera zoom
		//if(renderer->currentCamera.zoom != 1)
		{

			glm::vec2 cameraCenter;
			cameraCenter.x = windowW / 2.0f;
			cameraCenter.y = -windowH / 2.0f;

			v1 = scaleAroundPoint(v1, cameraCenter, currentCamera.zoom);
			v3 = scaleAroundPoint(v3, cameraCenter, currentCamera.zoom);
		}

		v1.x = internal::positionToScreenCoordsX(v1.x, (float)windowW);
		v3.x = internal::positionToScreenCoordsX(v3.x, (float)windowW);
		v1.y = internal::positionToScreenCoordsY(v1.y, (float)windowH);
		v3.y = internal::positionToScreenCoordsY(v3.y, (float)windowH);

		return glm::vec4(v1.x, v1.y, v3.x, v3.y);
	}

	glm::vec2 Renderer2D::getTextSize(const char *text, const Font font,
		const float sizePixels, const float spacing, const float line_space)
	{
		if (font.texture.id == 0)
		{
			errorFunc("Missing font", userDefinedData);
			return {};
		}

		float size = sizePixels / 64.f;

		glm::vec2 position = {};

		const int text_length = (int)strlen(text);
		Rect rectangle = {};
		rectangle.x = position.x;
		float linePositionY = position.y;

		if (text_length == 0) { return {}; }

		//This is the y position we render at because it advances when we encounter newlines
		float maxPos = 0;
		float maxPosY = 0;
		float bonusY = 0;
		int lineCount = 1;
		bool firstLine = true;
		float firstLineSize = 0;

		for (int i = 0; i < text_length; i++)
		{
			if (text[i] == '\n')
			{
				rectangle.x = position.x;
				linePositionY += (font.max_height + line_space) * size;
				bonusY += (font.max_height + line_space) * size;
				maxPosY = 0;
				lineCount++;
				firstLine = false;
			}
			else if (text[i] == '\t')
			{
				float x = font.max_height;
				rectangle.x += (x + spacing) * size * 3;
			}
			else if (text[i] == ' ')
			{
				rectangle.x += (font.spaceSize + spacing) * size;

				//float x = font.max_height;
				//rectangle.x += x * size + spacing * size;
			}
			else if (text[i] >= ' ' && text[i] <= '~')
			{
				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, text[i]);

				rectangle.z = quad.x1 - quad.x0;
				rectangle.w = quad.y1 - quad.y0;

				if (firstLine && rectangle.w > firstLineSize)
				{
					firstLineSize = rectangle.w;
				}

				rectangle.z *= size;
				rectangle.w *= size;

				rectangle.y = linePositionY + quad.y0 * size; //not needed

				if (font.monospaced)
				{
					rectangle.x += font.max_height + spacing * size;
				}
				else
				{
					rectangle.x += rectangle.z + spacing * size;
				}


				maxPosY = std::max(maxPosY, rectangle.y);
				maxPos = std::max(maxPos, rectangle.x);
			}
		}

		maxPos = std::max(maxPos, rectangle.x);
		maxPosY = std::max(maxPosY, rectangle.y);

		float paddX = maxPos;

		float paddY = maxPosY;

		paddY += font.max_height * size + bonusY;

		//paddY = ((lineCount-1) * font.max_height + (lineCount - 1) * line_space + firstLineSize) * size;
		paddY = ((lineCount)*font.max_height + (lineCount - 1) * line_space) * size;

		return glm::vec2{paddX, paddY};

	}

	float Renderer2D::determineTextRescaleFitSmaller(const std::string &str,
		gl2d::Font &f, glm::vec4 transform, float maxSize)
	{
		auto s = getTextSize(str.c_str(), f, maxSize);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{
			return maxSize;
		}
		else
		{
			if (ratioX < ratioY)
			{
				return maxSize * ratioX;
			}
			else
			{
				return maxSize * ratioY;
			}
		}
	}


	float Renderer2D::determineTextRescaleFitBigger(const std::string &str,
		gl2d::Font &f, glm::vec4 transform, float minSize)
	{
		auto s = getTextSize(str.c_str(), f, minSize);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{
			if (ratioX > ratioY)
			{
				return minSize * ratioY;
			}
			else
			{
				return minSize * ratioX;
			}
		}
		else
		{

		}

		return minSize;

	}

	float Renderer2D::determineTextRescaleFit(const std::string &str,
		gl2d::Font &f, glm::vec4 transform)
	{
		float ret = 1;

		auto s = getTextSize(str.c_str(), f, ret);

		float ratioX = transform.z / s.x;
		float ratioY = transform.w / s.y;


		if (ratioX > 1 && ratioY > 1)
		{
			if (ratioX > ratioY)
			{
				return ret * ratioY;
			}
			else
			{
				return ret * ratioX;
			}
		}
		else
		{
			if (ratioX < ratioY)
			{
				return ret * ratioX;
			}
			else
			{
				return ret * ratioY;
			}
		}

		return ret;
	}

	int  Renderer2D::wrap(const std::string &in, gl2d::Font &f,
		float baseSize, float maxDimension, std::string *outRez)
	{
		if (outRez)
		{
			*outRez = "";
			outRez->reserve(in.size() + 10);
		}

		std::string word = "";
		std::string currentLine = "";
		currentLine.reserve(in.size() + 10);

		bool wrap = 0;
		bool newLine = 1;
		int newLineCounter = 0;

		for (int i = 0; i < in.size(); i++)
		{
			word.push_back(in[i]);
			currentLine.push_back(in[i]);

			if (in[i] == ' ')
			{
				if (wrap)
				{
					if (outRez)
					{
						outRez->push_back('\n'); currentLine = "";
					}
					newLineCounter++;

				}

				if (outRez)
				{
					*outRez += word;
				}
				word = "";
				wrap = 0;
				newLine = false;
			}
			else if (in[i] == '\n')
			{
				if (wrap)
				{
					if (outRez)
					{
						outRez->push_back('\n');
					}
					newLineCounter++;
				}

				currentLine = "";

				if (outRez)
				{
					*outRez += word;
				}
				word = "";
				wrap = 0;
				newLine = true;
			}
			else
			{
				//let's check, only if needed
				if (!wrap && !newLine)
				{
					float size = baseSize;
					auto textSize = getTextSize(currentLine.c_str(), f, size);

					if (textSize.x >= maxDimension && !newLine)
					{
						//wrap last word
						wrap = 1;
					}
				};
			}

		}

		{
			if (wrap) { if (outRez)outRez->push_back('\n'); newLineCounter++; }

			if (outRez)
			{
				*outRez += word;
			}
		}

		return newLineCounter + 1;
	}

	void Renderer2D::renderText(glm::vec2 position, const char *text, const Font font,
		const Color4f color, const float sizePixels, const float spacing, const float line_space, bool showInCenter,
		const Color4f ShadowColor
		, const Color4f LightColor, float positionZ
	)
	{

		float size = sizePixels / 64.f;

		if (font.texture.id == 0)
		{
			errorFunc("Missing font", userDefinedData);
			return;
		}

		const int text_length = (int)strlen(text);
		Rect rectangle;
		rectangle.x = position.x;
		float linePositionY = position.y;

		if (showInCenter)
		{
			auto textSize = this->getTextSize(text, font, sizePixels, spacing, line_space);

			position.x -= textSize.x / 2.f;
			position.y -= textSize.y / 2.f;
			position.y += (font.max_height * size / 2.f) * 1.5f;
		}

		rectangle = {};
		rectangle.x = position.x;

		//This is the y position we render at because it advances when we encounter newlines
		linePositionY = position.y;

		for (int i = 0; i < text_length; i++)
		{
			if (text[i] == '\n')
			{
				rectangle.x = position.x;
				linePositionY += (font.max_height + line_space) * size;
			}
			else if (text[i] == '\t')
			{
				float x = font.max_height;
				rectangle.x += (x + spacing) * size * 3;
			}
			else if (text[i] == ' ')
			{
				rectangle.x += (font.spaceSize + spacing) * size;

				//float x = font.max_height;
				//rectangle.x += x * size + spacing * size;
			}
			else if (text[i] >= ' ' && text[i] <= '~')
			{

				const stbtt_aligned_quad quad = internal::fontGetGlyphQuad
				(font, text[i]);

				rectangle.z = quad.x1 - quad.x0;
				rectangle.w = quad.y1 - quad.y0;

				rectangle.z *= size;
				rectangle.w *= size;

				//rectangle.y = linePositionY - rectangle.w;
				rectangle.y = linePositionY + quad.y0 * size;

				glm::vec4 colorData[4] = {color, color, color, color};

				if (ShadowColor.w)
				{
					glm::vec2 pos = {-5, 3};
					pos *= size;
					renderRectangle({rectangle.x + pos.x, rectangle.y + pos.y,  rectangle.z, rectangle.w},
						font.texture, ShadowColor, glm::vec2{0, 0}, 0,
						glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1}, positionZ);

				}

				renderRectangle(rectangle, font.texture, colorData, glm::vec2{0, 0}, 0,
					glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1}, positionZ);

				if (LightColor.w)
				{
					glm::vec2 pos = {-2, 1};
					pos *= size;
					renderRectangle({rectangle.x + pos.x, rectangle.y + pos.y,  rectangle.z, rectangle.w},
						font.texture,
						LightColor, glm::vec2{0, 0}, 0,
						glm::vec4{quad.s0, quad.t0, quad.s1, quad.t1}, positionZ);

				}

				if (font.monospaced)
				{
					rectangle.x += font.max_height + spacing * size;
				}
				else
				{
					rectangle.x += rectangle.z + spacing * size;
				}

			}
		}
	}

	void Renderer2D::renderTextWrapped(const std::string &text,
		gl2d::Font f, glm::vec4 textPos, glm::vec4 color, float baseSize,
		float spacing, float lineSpacing,
		bool showInCenter, glm::vec4 shadowColor, glm::vec4 lightColor)
	{
		std::string newText;
		wrap(text, f, baseSize, textPos.z, &newText);
		renderText(textPos,
			newText.c_str(), f, color, baseSize, spacing, lineSpacing, showInCenter,
			shadowColor, lightColor);
	}

	glm::vec2 Renderer2D::getTextSizeWrapped(const std::string &text,
		gl2d::Font f, float maxTextLenght, float baseSize, float spacing, float lineSpacing)
	{
		std::string newText;
		wrap(text, f, baseSize, maxTextLenght, &newText);
		auto rez = getTextSize(
			newText.c_str(), f, baseSize, spacing, lineSpacing);

		return rez;
	}

	void Renderer2D::clearScreen(const Color4f color)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);

	#if GL2D_USE_OPENGL_130
		GLfloat oldColor[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, oldColor);

		GLfloat oldDepth;
		glGetFloatv(GL_DEPTH_CLEAR_VALUE, &oldDepth);

		glClearColor(color.r, color.g, color.b, color.a);
		glClearDepth(1.0f); // Optional, sets the value to which the depth buffer is cleared

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Restore old state
		glClearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
		glClearDepth(oldDepth);
	#else
		glClearBufferfv(GL_COLOR, 0, &color[0]);
		GLfloat depthClearValue = 1.0f;
		glClearBufferfv(GL_DEPTH, 0, &depthClearValue);
	#endif

	}

	void Renderer2D::setShaderProgram(const ShaderProgram shader)
	{
		currentShader = shader;
	}

	void Renderer2D::setCamera(const Camera camera)
	{
		currentCamera = camera;
	}

	void Renderer2D::resetCameraAndShader()
	{
		currentCamera = defaultCamera;
		currentShader = defaultShader;
	}

	void Renderer2D::renderPostProcess(ShaderProgram shader,
		Texture input, FrameBuffer result)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, result.fbo);

		enableNecessaryGLFeatures();

		if (!hasInitialized)
		{
			errorFunc("Library not initialized. Have you forgotten to call gl2d::init() ?", userDefinedData);
			return;
		}

		if (!vao)
		{
			errorFunc("Renderer not initialized. Have you forgotten to call gl2d::Renderer2D::create() ?", userDefinedData);
			return;
		}

		if (!shader.id)
		{
			errorFunc("Post Process Shader not created.", userDefinedData);
			return;
		}

		auto size = input.GetSize();

		if (size.x == 0 || size.y == 0)
		{
			return;
		}

		glViewport(0, 0, size.x, size.y);

		glUseProgram(shader.id);
		glUniform1i(shader.u_sampler, 0);

		input.bind();

		renderQuadToScreenInternal(*this);

		glBindVertexArray(0);


	}

#pragma endregion

	glm::ivec2 Texture::GetSize()
	{
		glm::ivec2 s;
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &s.x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &s.y);
		return s;
	}

	void Texture::createFromBuffer(const char *image_data, const int width, const int height
		, bool pixelated, bool useMipMaps)
	{
		GLuint id = 0;

		glActiveTexture(GL_TEXTURE0);

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		if (pixelated)
		{
			if (useMipMaps)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			if (useMipMaps)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		glGenerateMipmap(GL_TEXTURE_2D);


		this->id = id;
	}

	void Texture::create1PxSquare(const char *b)
	{
		if (b == nullptr)
		{
			const unsigned char buff[] =
			{
				0xff,
				0xff,
				0xff,
				0xff
			};

			createFromBuffer((char *)buff, 1, 1);
		}
		else
		{
			createFromBuffer(b, 1, 1);
		}

	}

	void Texture::createFromFileData(const unsigned char *image_file_data, const size_t image_file_size
		, bool pixelated, bool useMipMaps)
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char *decodedImage = stbi_load_from_memory(image_file_data, (int)image_file_size, &width, &height, &channels, 4);

		createFromBuffer((const char *)decodedImage, width, height, pixelated, useMipMaps);

		STBI_FREE(decodedImage);
	}

	void Texture::createFromFileDataWithPixelPadding(const unsigned char *image_file_data, const size_t image_file_size, int blockSize,
		bool pixelated, bool useMipMaps)
	{
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;

		const unsigned char *decodedImage = stbi_load_from_memory(image_file_data, (int)image_file_size, &width, &height, &channels, 4);

		int newW = width + ((width * 2) / blockSize);
		int newH = height + ((height * 2) / blockSize);

		auto getOld = [decodedImage, width](int x, int y, int c)->const unsigned char
		{
			return decodedImage[4 * (x + (y * width)) + c];
		};


		unsigned char *newData = new unsigned char[newW * newH * 4] {};

		auto getNew = [newData, newW](int x, int y, int c)
		{
			return &newData[4 * (x + (y * newW)) + c];
		};

		int newDataCursor = 0;
		int dataCursor = 0;

		//first copy data
		for (int y = 0; y < newH; y++)
		{
			int yNo = 0;
			if ((y == 0 || y == newH - 1
				|| ((y) % (blockSize + 2)) == 0 ||
				((y + 1) % (blockSize + 2)) == 0
				))
			{
				yNo = 1;
			}

			for (int x = 0; x < newW; x++)
			{
				if (
					yNo ||

					((
					x == 0 || x == newW - 1
					|| (x % (blockSize + 2)) == 0 ||
					((x + 1) % (blockSize + 2)) == 0
					)
					)

					)
				{
					newData[newDataCursor++] = 0;
					newData[newDataCursor++] = 0;
					newData[newDataCursor++] = 0;
					newData[newDataCursor++] = 0;
				}
				else
				{
					newData[newDataCursor++] = decodedImage[dataCursor++];
					newData[newDataCursor++] = decodedImage[dataCursor++];
					newData[newDataCursor++] = decodedImage[dataCursor++];
					newData[newDataCursor++] = decodedImage[dataCursor++];
				}

			}

		}

		//then add margins


		for (int x = 1; x < newW - 1; x++)
		{
			//copy on left
			if (x == 1 ||
				(x % (blockSize + 2)) == 1
				)
			{
				for (int y = 0; y < newH; y++)
				{
					*getNew(x - 1, y, 0) = *getNew(x, y, 0);
					*getNew(x - 1, y, 1) = *getNew(x, y, 1);
					*getNew(x - 1, y, 2) = *getNew(x, y, 2);
					*getNew(x - 1, y, 3) = *getNew(x, y, 3);
				}

			}
			else //copy on rigght
				if (x == newW - 2 ||
					(x % (blockSize + 2)) == blockSize
					)
				{
					for (int y = 0; y < newH; y++)
					{
						*getNew(x + 1, y, 0) = *getNew(x, y, 0);
						*getNew(x + 1, y, 1) = *getNew(x, y, 1);
						*getNew(x + 1, y, 2) = *getNew(x, y, 2);
						*getNew(x + 1, y, 3) = *getNew(x, y, 3);
					}
				}
		}

		for (int y = 1; y < newH - 1; y++)
		{
			if (y == 1 ||
				(y % (blockSize + 2)) == 1
				)
			{
				for (int x = 0; x < newW; x++)
				{
					*getNew(x, y - 1, 0) = *getNew(x, y, 0);
					*getNew(x, y - 1, 1) = *getNew(x, y, 1);
					*getNew(x, y - 1, 2) = *getNew(x, y, 2);
					*getNew(x, y - 1, 3) = *getNew(x, y, 3);
				}
			}
			else
				if (y == newH - 2 ||
					(y % (blockSize + 2)) == blockSize
					)
				{
					for (int x = 0; x < newW; x++)
					{
						*getNew(x, y + 1, 0) = *getNew(x, y, 0);
						*getNew(x, y + 1, 1) = *getNew(x, y, 1);
						*getNew(x, y + 1, 2) = *getNew(x, y, 2);
						*getNew(x, y + 1, 3) = *getNew(x, y, 3);
					}
				}

		}

		createFromBuffer((const char *)newData, newW, newH, pixelated, useMipMaps);

		STBI_FREE(decodedImage);
		delete[] newData;
	}

	void Texture::loadFromFile(const char *fileName, bool pixelated, bool useMipMaps)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			char c[300] = {0};
			strcat(c, "error openning: ");
			strcat(c + strlen(c), fileName);
			errorFunc(c, userDefinedData);
			return;
		}

		int fileSize = 0;
		file.seekg(0, std::ios::end);
		fileSize = (int)file.tellg();
		file.seekg(0, std::ios::beg);
		unsigned char *fileData = new unsigned char[fileSize];
		file.read((char *)fileData, fileSize);
		file.close();

		createFromFileData(fileData, fileSize, pixelated, useMipMaps);

		delete[] fileData;

	}

	void Texture::loadFromFileWithPixelPadding(const char *fileName, int blockSize,
		bool pixelated, bool useMipMaps)
	{
		std::ifstream file(fileName, std::ios::binary);

		if (!file.is_open())
		{
			char c[300] = {0};
			strcat(c, "error openning: ");
			strcat(c + strlen(c), fileName);
			errorFunc(c, userDefinedData);
			return;
		}

		int fileSize = 0;
		file.seekg(0, std::ios::end);
		fileSize = (int)file.tellg();
		file.seekg(0, std::ios::beg);
		unsigned char *fileData = new unsigned char[fileSize];
		file.read((char *)fileData, fileSize);
		file.close();

		createFromFileDataWithPixelPadding(fileData, fileSize, blockSize, pixelated, useMipMaps);

		delete[] fileData;

	}

	size_t Texture::getMemorySize(int mipLevel, glm::ivec2 *outSize)
	{
		glBindTexture(GL_TEXTURE_2D, id);

		glm::ivec2 stub = {};

		if (!outSize)
		{
			outSize = &stub;
		}

		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_WIDTH, &outSize->x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_HEIGHT, &outSize->y);

		glBindTexture(GL_TEXTURE_2D, 0);

		return outSize->x * outSize->y * 4;
	}

	void Texture::readTextureData(void *buffer, int mipLevel)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexImage(GL_TEXTURE_2D, mipLevel, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}

	std::vector<unsigned char> Texture::readTextureData(int mipLevel, glm::ivec2 *outSize)
	{
		glBindTexture(GL_TEXTURE_2D, id);

		glm::ivec2 stub = {};

		if (!outSize)
		{
			outSize = &stub;
		}

		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_WIDTH, &outSize->x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, mipLevel, GL_TEXTURE_HEIGHT, &outSize->y);

		std::vector<unsigned char> data;
		data.resize(outSize->x * outSize->y * 4);
		glGetTexImage(GL_TEXTURE_2D, mipLevel, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		glBindTexture(GL_TEXTURE_2D, 0);

		return data;
	}

	void Texture::bind(const unsigned int sample)
	{
		glActiveTexture(GL_TEXTURE0 + sample);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Texture::unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::cleanup()
	{
		glDeleteTextures(1, &id);
		*this = {};
	}

	//glm::mat3 Camera::getMatrix()
	//{
	//	glm::mat3 m;
	//	m = { zoom, 0, position.x ,
	//		 0, zoom, position.y,
	//		0, 0, 1,
	//	};
	//	m = glm::transpose(m);
	//	return m; //todo not tested, add rotation
	//}

	void Camera::follow(glm::vec2 pos, float speed, float min, float max, float w, float h)
	{
		pos.x -= w / 2.f;
		pos.y -= h / 2.f;

		glm::vec2 delta = pos - position;
		bool signX = delta.x >= 0;
		bool signY = delta.y >= 0;

		float len = glm::length(delta);

		delta = glm::normalize(delta);

		if (len < min * 2)
		{
			speed /= 4.f;
		}
		else if (len < min * 4)
		{
			speed /= 2.f;
		}

		if (len > min)
		{
			if (len > max)
			{
				len = max;
				position = pos - (max * delta);
				//osition += delta * speed;
			}
			else
			{
				position += delta * speed;


			}

			glm::vec2 delta2 = pos - position;
			bool signX2 = delta.x >= 0;
			bool signY2 = delta.y >= 0;
			if (signX2 != signX || signY2 != signY || glm::length(delta2) > len)
			{
				//position = pos;
			}
		}
	}

	glm::vec2 internal::convertPoint(const Camera &camera, const glm::vec2 &p, float windowW, float windowH)
	{
		glm::vec2 r = p;


		//Apply camera transformations
		r.x += camera.position.x;
		r.y += camera.position.y;

		{
			glm::vec2 cameraCenter = {camera.position.x + windowW / 2, -camera.position.y - windowH / 2};

			r = rotateAroundPoint(r,
				cameraCenter,
				camera.rotation);
		}

		{
			glm::vec2 cameraCenter = {camera.position.x + windowW / 2, camera.position.y + windowH / 2};

			r = scaleAroundPoint(r,
				cameraCenter,
				1.f / camera.zoom);
		}

		//if (this->rotation != 0)
		//{
		//	glm::vec2 cameraCenter;
		//
		//	cameraCenter.x = windowW / 2.0f;
		//	cameraCenter.y = windowH / 2.0f;
		//
		//	r = rotateAroundPoint(r, cameraCenter, this->rotation);
		//
		//}

		//{
		//	glm::vec2 cameraCenter;
		//	cameraCenter.x = windowW / 2.0f;
		//	cameraCenter.y = -windowH / 2.0f;
		//
		//	r = scaleAroundPoint(r, cameraCenter, this->zoom);
		//
		//}

		return r;
	}

	void FrameBuffer::create(int w, int h, bool hasDepth, bool nearestFilter)
	{
		if (w < 0) { w = 0; }
		if (h < 0) { h = 0; }

		this->w = w;
		this->h = h;

		auto filter = GL_LINEAR;
		if (nearestFilter) { filter = GL_NEAREST; }

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &texture.id);
		glBindTexture(GL_TEXTURE_2D, texture.id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

		if (hasDepth)
		{
			glGenTextures(1, &depthTexture.id); //todo add depth stuff
			glBindTexture(GL_TEXTURE_2D, depthTexture.id);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.id, 0);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void FrameBuffer::resize(int w, int h)
	{
		if (w < 0) { w = 0; }
		if (h < 0) { h = 0; }

		if (this->w != w || this->h != h)
		{
			this->w = w;
			this->h = h;

			glBindTexture(GL_TEXTURE_2D, texture.id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			if (depthTexture.id)
			{
				glBindTexture(GL_TEXTURE_2D, depthTexture.id);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}

	void FrameBuffer::cleanup()
	{
		if (fbo)
		{
			glDeleteFramebuffers(1, &fbo);
			fbo = 0;
		}

		texture.cleanup();
		depthTexture.cleanup();

		*this = {};
	}

	void FrameBuffer::clear()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearColor(0, 0, 0, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	glm::vec4 computeTextureAtlas(int xCount, int yCount, int x, int y, bool flip)
	{
		float xSize = 1.f / xCount;
		float ySize = 1.f / yCount;

		if (flip)
		{
			return {(x + 1) * xSize, 1 - (y * ySize), (x)*xSize, 1.f - ((y + 1) * ySize)};
		}
		else
		{
			return {x * xSize, 1 - (y * ySize), (x + 1) * xSize, 1.f - ((y + 1) * ySize)};
		}

	}

	glm::vec4 computeTextureAtlasWithPadding(int mapXsize, int mapYsize,
		int xCount, int yCount, int x, int y, bool flip)
	{
		float xSize = 1.f / xCount;
		float ySize = 1.f / yCount;

		float Xpadding = 1.f / mapXsize;
		float Ypadding = 1.f / mapYsize;

		glm::vec4 noFlip = {x * xSize + Xpadding, 1 - (y * ySize) - Ypadding, (x + 1) * xSize - Xpadding, 1.f - ((y + 1) * ySize) + Ypadding};

		if (flip)
		{
			glm::vec4 flip = {noFlip.z, noFlip.y, noFlip.x, noFlip.w};

			return flip;
		}
		else
		{
			return noFlip;
		}
	}




}

#ifdef _MSC_VER
#pragma warning( pop )
#endif