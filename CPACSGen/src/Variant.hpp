#pragma once

#include <typeinfo>

template <std::size_t Int, std::size_t... Ints>
struct max {
	static constexpr const std::size_t value = max<Int, max<Ints...>::value>::value;
};

template <std::size_t Int1, std::size_t Int2>
struct max<Int1, Int2> {
	static constexpr const std::size_t value = Int1 > Int2 ? Int1 : Int2;
};

template <std::size_t Int>
struct max<Int> {
	static constexpr const std::size_t value = Int;
};

template <std::size_t Int, std::size_t... Ints>
constexpr std::size_t max_v = max<Int, Ints...>::value;


template<typename... Ts>
class Variant {
public:
	Variant()
		: m_type(nullptr) { }

	template<typename T>
	Variant(const T& t)  {
		init(t);
	}

	Variant(const Variant& other)  {
		other.visit([this](auto& t) { init(t); });
	}

	auto& operator=(const Variant& other) {
		other.visit([this](auto& t) { init(t); });
		return *this;
	}

	~Variant() {
		erase();
	}

	template<typename T>
	auto& operator=(const T& t) {
		erase();
		init(t);
		return *this;
	}

	template<typename Visitor>
	void visit(Visitor func) {
		auto dummy = {
			(m_type != nullptr && *m_type == typeid(Ts) ? (func(*reinterpret_cast<Ts*>(m_data)), 0) : 0)...
		};
	}

	template<typename Visitor>
	void visit(Visitor func) const {
		auto dummy = {
			(m_type != nullptr && *m_type == typeid(Ts) ? (func(*reinterpret_cast<const Ts*>(m_data)), 0) : 0)...
		};
	}

private:
	void erase() {
		if (m_type)
			m_type = nullptr;
	}

	template <typename T>
	void init(const T& t) {
		new (m_data) T(t);
		m_type = &typeid(t);
	}

	const std::type_info* m_type;
	unsigned char m_data[max_v<sizeof(Ts)...>];
};