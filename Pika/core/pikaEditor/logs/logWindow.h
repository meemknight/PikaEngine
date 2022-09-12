#pragma once
#include <logs/log.h>
#include <IconsForkAwesome.h>

namespace pika
{


	struct LogWindow
	{

		void init();

		void update(pika::LogManager &logManager);


		static constexpr char *ICON = ICON_FK_COMMENT_O;
		static constexpr char *NAME = "logs";
		static constexpr char *ICON_NAME = ICON_FK_COMMENT_O " logs";

	};

}
