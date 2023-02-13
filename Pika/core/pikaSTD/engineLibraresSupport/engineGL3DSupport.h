#pragma once

#include <gl3d.h>
#include <baseContainer.h>


void errorCallbackCustom(std::string err, void *userData);
std::string readEntireFileCustom(const char *fileName, bool &couldNotOpen, void *userData);
std::vector<char> readEntireFileBinaryCustom(const char *fileName, bool &couldNotOpen, void *userData);
bool defaultFileExistsCustom(const char *fileName, void *userData);



#if PIKA_SHOULD_REMOVE_IMGUI == 0

#endif

#include <pikaImgui/pikaImgui.h>

namespace pika
{
	namespace gl3d
	{
		void generalSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);

		void fxaaSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);

		void ssaoSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);

		void ssrSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);

		void bloomSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);

		void chromaticAberationSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);

		void lightEditorSettingsWindow(int imguiId, ::gl3d::Renderer3D &renderer);


	};
};

