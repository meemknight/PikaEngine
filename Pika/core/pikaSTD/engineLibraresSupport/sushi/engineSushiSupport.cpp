#include "engineSushiSupport.h"

namespace pika
{

	::sushi::SushiInput toSushi(const pika::Input &in)
	{
		::sushi::SushiInput out = {};

		out.lMouse = toSushi(in.lMouse);
		out.rMouse = toSushi(in.rMouse);

		out.mouseX = in.mouseX;
		out.mouseY = in.mouseY;

		static_assert(::sushi::SushiButton::BUTTONS_COUNT == ::pika::Button::BUTTONS_COUNT, "");
		for (int i = 0; i < ::sushi::SushiButton::BUTTONS_COUNT; i++)
		{
			out.buttons[i] = toSushi(in.buttons[i]);
		}

		static_assert(sizeof(::sushi::SushiInput::typedInput) == sizeof(::pika::Input::typedInput), "");
		memcpy(out.typedInput, in.typedInput, sizeof(::sushi::SushiInput::typedInput));

		out.deltaTime = in.deltaTime;

		static_assert(::sushi::SushiInput::MAX_CONTROLLERS_COUNT == ::pika::Input::MAX_CONTROLLERS_COUNT, "");

		for (int i = 0; i < ::sushi::SushiInput::MAX_CONTROLLERS_COUNT; i++)
		{
			out.controllers[i] = toSushi(in.controllers[i]);
		}
		
		out.anyController = toSushi(in.anyController);

		return out;
	}
	
	::sushi::SushiController toSushi(const pika::Controller &in)
	{
		::sushi::SushiController out = {};

		out.LT = in.LT;
		out.RT = in.RT;
		out.connected = in.connected;
		out.LStick.x = in.LStick.x;
		out.LStick.y = in.LStick.y;
		out.RStick.x = in.RStick.x;
		out.RStick.y = in.RStick.y;

		static_assert(::sushi::SushiController::ButtonsCount == ::pika::Controller::ButtonsCount, "");

		for (int i = 0; i < ::sushi::SushiController::ButtonsCount; i++)
		{
			out.buttons[i] = toSushi(in.buttons[i]);
		}

		return out;
	}
	
	::sushi::SushiButton toSushi(const pika::Button &in)
	{
		::sushi::SushiButton out = {};
		out.flags = in.flags;
		return out;
	}

};
