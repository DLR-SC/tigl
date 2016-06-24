#pragma once

#include <boost/optional.hpp>
#include <memory>
#include <stdexcept>

class UninitializedOptional : public std::logic_error {
public:
	UninitializedOptional()
		: std::logic_error("Called get() on an Optional which was not set") {}
};

// represents an optional type
template <typename T>
class Optional {
public:
	Optional() = default;

	Optional(const Optional&) = delete;
	Optional& operator=(const Optional&) = delete;

	Optional(Optional&&) noexcept = default;
	Optional& operator=(Optional&&) noexcept = default;

	template<typename U>
	Optional& operator=(const U& t) {
		m_ptr = std::make_unique<T>(t);
		return *this;
	}

	template<typename U>
	Optional& operator=(U&& t) {
		m_ptr = std::make_unique<T>(std::move(t));
		return *this;
	}

	void construct() {
		m_ptr = std::make_unique<T>();
	}

	bool isValid() const {
		return m_ptr != nullptr;
	}

	T& get() {
		if (!isValid())
			throw UninitializedOptional();
		return *m_ptr;
	}

	const T& get() const {
		if (!isValid())
			throw UninitializedOptional();
		return *m_ptr;
	}

	T* const operator->() {
		return &get();
	}

	const T*const operator->() const {
		return &get();
	}

private:
	std::unique_ptr<T> m_ptr;
};
