#pragma once
#ifndef _ARRAY_HPP
#define _ARRAY_HPP

#include <memory>

template<typename T>
class Array
{
public:
	typedef std::size_t size_type;
	typedef Array<T> self_type;
	typedef T value_type;
	typedef T& reference_type;
	typedef T* pointer_type;
	typedef pointer_type  iterator;
	typedef const pointer_type const_iterator;
public:
	Array()
		: mSize(0)
	{
	}

	Array(Array&& _old)
		: mSize(_old.mSize)
	{
		mValues = std::move(_old.mValues);
		_old.mSize = 0;
	}

	~Array()
	{
	}

	self_type& operator =(Array&& _old)
	{
		mValues = std::move(_old.mValues);
		mSize = _old.mSize;
		_old.mSize = 0;

		return (*this);
	}

	self_type& operator =(std::nullptr_t)
	{
		mSize = 0;
		mValues.reset();

		return (*this);
	}

	reference_type operator [](size_type _idx)
	{
		assert(_idx < mSize);
		return mValues[_idx];
	}

	const reference_type operator [](size_type _idx) const
	{
		assert(_idx < mSize);
		return mValues[_idx];
	}

	const reference_type at(size_type _idx) const
	{
		assert(_idx < mSize);
		return mValues[_idx];
	}

	size_type size() const
	{
		return mSize;
	}

	pointer_type data()
	{
		return mValues.get();
	}

	const pointer_type data() const
	{
		return mValues.get();
	}

	value_type& front()
	{
		return *mValues;
	}

	const value_type& front() const
	{
		return *mValues;
	}

	value_type& back()
	{
		return *(--end());
	}

	const value_type& back() const
	{
		return *(--end());
	}

	iterator begin()
	{
		return mValues.get();
	}

	iterator end()
	{
		return begin() + size();
	}

	const_iterator begin() const
	{
		return mValues.get();
	}

	const_iterator end() const
	{
		return begin() + size();
	}

	const_iterator cbegin() const
	{
		return mValues.get();
	}

	const_iterator cend() const
	{
		return cbegin() + size();
	}

	self_type clone() const
	{
		return Copy(*this);
	}

	static self_type New(size_type _size)
	{
		if(_size > 0)
		{
			return Array(new value_type[_size], _size);
		}else{
			return Array();
		}
	}

	static self_type Copy(const self_type& _other)
	{
		self_type new_arr = New(_other.size());
		std::copy(_other.cbegin(), _other.cend(), new_arr.begin());
		return new_arr;
	}

private:
	Array(value_type* values, size_type _size)
		: mValues(values)
		, mSize(_size)
	{
	}

	Array(const Array&);
	Array& operator =(const Array&);
private:
	size_type mSize;
	std::unique_ptr<value_type[]> mValues;
};

#endif
