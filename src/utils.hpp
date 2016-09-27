
template<typename T>
T between(const T& min, const T& v, const T& max)
{
	return std::max(std::min(v, max), min);
}
