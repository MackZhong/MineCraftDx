#pragma once

template<class T> __interface IComparable
{
	bool CompareTo(const T& other);
};
