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
#include <imfilebrowser.h>
#include <windowSystemm/input.h>
#include <baseContainer.h>


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

		void fpsInput(::gl3d::Renderer3D &renderer, pika::Input &input, float moveSpeed, glm::dvec2 &lastMousePos,
			RequestedContainerInfo &requestedInfo, glm::ivec2 windowSize);

		bool loadSettingsFromFileName(::gl3d::Renderer3D &renderer, std::string file, RequestedContainerInfo &info);

		struct General3DEditor
		{
			General3DEditor() 
			{
				skyBoxFileSelector.setInfo("Sellect skyBox", PIKA_RESOURCES_PATH, {".hdr", ".png"});
				settingsFileSelector.setInfo("Gl3D file", PIKA_RESOURCES_PATH, {".gl3d"});
			};

			void loadFromFile(::gl3d::Renderer3D &renderer, std::string file, RequestedContainerInfo &info);

			void update(int imguiId, ::gl3d::Renderer3D &renderer, pika::Input &input, float moveSpeed
				,RequestedContainerInfo &info, glm::ivec2 windowSize);
			glm::dvec2 lastMousePos = {};
			::gl3d::AtmosfericScatteringSettings atmosphericScattering;
			
			std::string currentSkyBox;

			pika::pikaImgui::FileSelector skyBoxFileSelector;

			pika::pikaImgui::FileSelector settingsFileSelector;


			void saveToFile(::gl3d::Renderer3D &renderer, RequestedContainerInfo &info);
		};
	};
};

