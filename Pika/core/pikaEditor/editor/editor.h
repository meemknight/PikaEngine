#pragma once

#include <pikaImgui/pikaImgui.h>


namespace pika
{

	struct Editor
	{

		void init();

		void update();

		struct
		{
			bool dockMainWindow = 1;
		}optionsFlags;


	};



}