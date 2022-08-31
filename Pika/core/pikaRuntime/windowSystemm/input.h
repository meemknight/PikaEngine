#pragma once
#include <cstdint>


#define PIKA_ADD_FLAG(NAME, SETNAME, VALUE)	\
	bool NAME() {return (flags & ((std::uint32_t)1<<VALUE)); } \
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

		//true if the key is double pressed (true only for one frame, 3 presses would yield only one frame of this being true)
		PIKA_ADD_FLAG(doublePressed, setDoublePressed, 4);

		//last state of the button (last frame)
		PIKA_ADD_FLAG(lastState, setLastState, 5);

	};

	struct Input
	{

		Button lMouse = {};
		Button rMouse = {};



	};



};



#undef PIKA_ADD_FLAG