#pragma once
//////////////////////////////////////////
//staticVector.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#include <logs/assert.h>


namespace pika
{


	template<class T, size_t N>
	struct StaticVector
	{
		typedef T *iterator;
		typedef const T *constIterator;

		iterator begin() { return &((T *)beg_)[0]; }
		constIterator begin() const { return &((T *)beg_)[0]; }
		iterator end() { return &((T *)beg_)[size_]; }
		constIterator end() const { return &((T *)beg_)[size_]; }

		static constexpr unsigned int MAX_SIZE = N;


		StaticVector() {};

		StaticVector(StaticVector &&other)
		{
			for (size_t i = 0; i < other.size_; i++)
			{
				beg_[i] = std::move(other.beg_[i]);
			}
			
			this->size_ = other.size_;
			other.size_ = 0;
		}

		StaticVector(const StaticVector &other)
		{
			for (size_t i = 0; i < other.size_; i++)
			{
				beg_[i] = other.beg_[i];
			}

			this->size_ = other.size_;
		}

		size_t size()const { return size_; }

		bool empty() const
		{
			return (size_ == 0);
		}

		T *data()
		{
			return beg_;
		}

		StaticVector &operator= (const StaticVector &other)
		{
			if (this == &other)
			{
				return *this;
			}

			for (size_t i = 0; i < other.size_; i++)
			{
				beg_[i] = other.beg_[i];
			}
			this->size_ = other.size_;

			return *this;
		}

		StaticVector &operator= (StaticVector &&other)
		{
			if (this == &other)
			{
				return *this;
			}

			for (size_t i = 0; i < other.size_; i++)
			{
				beg_[i] = std::move(other.beg_[i]);
			}

			this->size_ = other.size_;
			other.size_ = 0;

			return *this;
		}

		T &operator[] (size_t index)
		{
			PIKA_PERMA_ASSERT(index < size_, "buffer overflow on acces");
			return static_cast<T *>(beg_)[index];
		}

		T operator[] (size_t index) const
		{
			PIKA_PERMA_ASSERT(index < size_, "buffer overflow on acces");
			return static_cast<T *>(beg_)[index];
		}

		T &back()
		{
			return (*this)[size_ - 1];
		}

		const T &back() const
		{
			return (*this)[size_ - 1];
		}

		void clear() { size_ = 0; }

		void push_back(const T &el)
		{
			PIKA_PERMA_ASSERT(size_ < MAX_SIZE, "exceded max size in push back");
			beg_[size_] = std::forward<T>(el);
			size_++;
		}

		void push_back(T &&el)
		{
			PIKA_PERMA_ASSERT(size_ < MAX_SIZE, "exceded max size in push back");
			beg_[size_] = std::forward<T>(el);
			size_++;
		}

		void pop_back()
		{
			PIKA_PERMA_ASSERT(size_ > 0, "buffer underflow on pop back");
			size_--;
		}

	private:

		size_t size_ = 0;
		T beg_[N];

	};



}