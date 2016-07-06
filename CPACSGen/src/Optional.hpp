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

	// constructs the optional, is used in case T is not copy- or moveable
	template <typename... U>
	void construct(U&&... args) {
		m_ptr = std::make_unique<T>(std::forward<U>(args)...);
	}

	void destroy() {
		m_ptr = nullptr;
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
