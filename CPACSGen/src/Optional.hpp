#pragma once

#include <boost/optional.hpp>
#include <stdexcept>
#include <typeinfo>
#include <string>

namespace tigl {
	template <typename T>
	class UninitializedOptional : public std::runtime_error {
	public:
		UninitializedOptional()
			: std::runtime_error("Called get() on an Optional<" + std::string(typeid(T).name()) + "> which was not set") {} // using typeid is not optimal but better than nothing
	};

	// represents an optional type
	// basically wraps boost::optional in a simpler form
	template <typename T>
	class Optional {
	public:
		// ctor
		Optional() {}

		// copy ctor
		Optional(const Optional& other) {
			m_data = other.m_data;
		}

		// copy assign
		Optional& operator=(const Optional& other) {
			m_data = other.m_data;
			return *this;
		}

		// move ctor
		Optional(Optional&& other) {
			m_data = std::move(other.m_data);
		}

		// move assign
		Optional& operator=(Optional&& other) {
			m_data = std::move(other.m_data);
			return *this;
		}

		// copy assigns the contained object
		template<typename U>
		Optional& operator=(const U& t) {
			m_data = t;
			return *this;
		}

		// move assigns the contained object
		template<typename U>
		Optional& operator=(U&& t) {
			m_data = std::move(t);
			return *this;
		}

		// constructs the optional in-place, is used in case T is not copy- or moveable
		template <typename... U>
		void construct(U&&... args) {
			m_data = boost::in_place(std::forward<U>(args)...);
		}

		// destroys the contained object
		void destroy() {
			m_data = boost::none;
		}

		// evaluates to true when the Optional is set
		explicit operator bool() const {
			return static_cast<bool>(m_data);
		}

		// evaluates to true when the Optional is not set
		bool operator!() const {
			return !m_data;
		}

		// retrieves a reference to the contained object
		T& operator*() {
			if (!m_data)
				throw UninitializedOptional<T>();
			return *m_data;
		}

		// retrieves a reference to the contained object
		const T& operator*() const {
			if (!m_data)
				throw UninitializedOptional<T>();
			return *m_data;
		}

		// direct access to members of contained object
		T* const operator->() {
			return &operator*();
		}

		// direct access to members of contained object
		const T* const operator->() const {
			return &operator*();
		}

	private:
		boost::optional<T> m_data;
	};
}
