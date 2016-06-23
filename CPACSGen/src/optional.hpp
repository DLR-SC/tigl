#pragma once

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
	Optional(T&& t)
		: m_ptr(std::move(t)) {}

	Optional(const Optional& other) = delete;
	Optional& operator=(const Optional& other) = delete;

	//Optional(const Optional& other) {
	//	if (other.isValid())
	//		m_ptr = std::make_unique<T>(other.get());
	//}

	//Optional& operator=(const Optional& other) {
	//	if (other.isValid())
	//		m_ptr = std::make_unique<T>(other.get());
	//	return *this;
	//}

	Optional(Optional&&) noexcept = default;
	Optional& operator=(Optional&&) noexcept = default;

	Optional& operator=(const T& t) {
		m_ptr = std::make_unique<T>(t);
		return *this;
	}

	Optional& operator=(T&& t) {
		m_ptr = std::make_unique<T>(std::move(t));
		return *this;
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
