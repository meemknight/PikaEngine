#pragma once
#include <type_traits>
#include <pikaConfig.h>
#include <logs/assert.h>

namespace pika
{

	struct Nullopt_t
	{
		constexpr explicit Nullopt_t() {};
	};

	inline constexpr Nullopt_t nullopt{Nullopt_t{}};


	template<class T>
	struct Optional
	{

		Optional() {};

		Optional(T &other)
		{
			data = other;
			hasValue_ = true;
		}

		Optional(T &&other)
		{
			data = std::forward<T>(other);
			hasValue_ = true;
		}

		Optional(const Nullopt_t &nullopt)
		{
			data = {};
			hasValue_ = false;
		}

		Optional &operator=(const Optional &other)
		{
			data = other.data;
			hasValue_ = true;
			return *this;
		}

		Optional &operator=(const T &other)
		{
			data = other;
			hasValue_ = true;
			return *this;
		}

		Optional &operator=(const Nullopt_t &nullopt)
		{
			data = {};
			hasValue_ = false;
			return *this;
		}

		T &value()
		{
			
			#if !PIKA_REMOVE_OPTIONAL_NOVALUE_CHECKS_IN_PRODUCTION || !PIKA_PRODUCTION
				PIKA_PERMA_ASSERT(hasValue_, "Invalid value acces in optional")
			#endif

			return data;
		}

		operator T& () const
		{
			return value();
		}

		bool hasValue() { return hasValue_; }

	private:
		bool hasValue_ = false;
		T data = {};

	};

}