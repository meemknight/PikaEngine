#pragma once
#include <cstdint>

#define PIKA_ADD_FLAG(NAME, SETNAME, VALUE)	\
	bool NAME() const {return (flags & ((std::uint32_t)1<<VALUE)); } \
	void SETNAME(bool s) { \
		if (s) { flags = flags | ((std::uint32_t)1 << VALUE); }	\
			else { flags = flags & ~((std::uint32_t)1 << VALUE); }	\
		}
	

namespace pika
{


	struct Button
	{
		//internal use only
		float timer = 0;
	
		//internal use only
		std::uint32_t flags = 0;

		//true in the first frame the key is pressed
		PIKA_ADD_FLAG(pressed, setPressed, 0);

		//true while the key is held
		PIKA_ADD_FLAG(held, setHeld, 1);
	
		//true in the frame the key is released
		PIKA_ADD_FLAG(released, setReleased, 2);

		//true in the first frame is pressed then after a pause true every few milliseconds
		PIKA_ADD_FLAG(typed, setTyped, 3);

		//todo implement
		//true if the key is double pressed (true only for one frame, 3 presses would yield only one frame of this being true)
		PIKA_ADD_FLAG(doublePressed, setDoublePressed, 4);

		//last state of the button (last frame)
		PIKA_ADD_FLAG(lastState, setLastState, 5);

		enum
		{
			A = 0,
			B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			NR0, NR1, NR2, NR3, NR4, NR5, NR6, NR7, NR8, NR9,
			Space, Enter, Escape,
			Up, Down, Left, Right,
			LeftCtrl, Tab, LeftAlt,
			BUTTONS_COUNT, //
		};

	};

	struct Controller
	{
		enum Buttons
		{
			A = 0,
			B,
			X,
			Y,
			LBumper,
			RBumper,
			Back,
			Start,
			Guide,
			LThumb,
			Rthumb,
			Up,
			Right,
			Down,
			Left,
			ButtonsCount
		};

		Button buttons[Buttons::ButtonsCount] = {};
		
		float LT = 0.f;
		float RT = 0.f;

		struct
		{
			constexpr static float JOYSTICK_SENSITIVITY = 0.7f;

			float x = 0.f, y = 0.f;

			bool left() { return x < -JOYSTICK_SENSITIVITY; }
			bool right() { return x > JOYSTICK_SENSITIVITY; }
			bool up() { return y > JOYSTICK_SENSITIVITY; }
			bool down() { return y < -JOYSTICK_SENSITIVITY; }

		}LStick, RStick;

		bool connected = 0;

		void resetAllButtons() 
		{
			*this = {};
		}
	};

	struct Input
	{
		//typed input doesn't work with mouse buttons
		Button lMouse = {};
		Button rMouse = {};

		//mouse position relative to window
		int mouseX = 0;
		int mouseY = 0;

		Button buttons[Button::BUTTONS_COUNT] = {};
		
		char typedInput[20] = {};

		float deltaTime = 0;

		//focus is here because it makes sense for the replay
		bool hasFocus = 0;
		bool lastFrameHasFocus = 0;

		constexpr static int MAX_CONTROLLERS_COUNT = 4; //don't change
		Controller controllers[MAX_CONTROLLERS_COUNT] = {};

		//a logic or between all the controllers
		Controller anyController = {};
	};


	void processAButton(pika::Button &b, int action);

};



#undef PIKA_ADD_FLAG