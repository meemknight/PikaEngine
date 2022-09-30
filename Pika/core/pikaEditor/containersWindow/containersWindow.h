#pragma once
#include <IconsForkAwesome.h>
#include <imgui.h>
#include <logs/log.h>
#include <dllLoader/dllLoader.h>
#include <containerManager/containerManager.h>

namespace pika
{

	struct ContainersWindow
	{

		void init();

		void update(pika::LogManager &logManager, bool &open, pika::LoadedDll &loadedDll,
			pika::ContainerManager &containerManager);

		static constexpr char *ICON = ICON_FK_MICROCHIP;
		static constexpr char *NAME = "Containers manager";
		static constexpr char *ICON_NAME = ICON_FK_MICROCHIP " Containers manager";

	};



}