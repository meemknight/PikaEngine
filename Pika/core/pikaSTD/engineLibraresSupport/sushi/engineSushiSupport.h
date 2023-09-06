#pragma once

#include <windowSystemm/window.h>
#include <sushi/sushiInput.h>

namespace pika
{
	::sushi::SushiInput toSushi(const pika::Input &in);
	::sushi::SushiController toSushi(const pika::Controller &in);
	::sushi::SushiButton toSushi(const pika::Button &in);

};

