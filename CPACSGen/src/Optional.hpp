#pragma once

#include <boost/optional.hpp>
#include <stdexcept>
#include <typeinfo>
#include <string>

namespace tigl {
	template <typename T>
	class UninitializedOptional : public std::logic_error {
	public:
		UninitializedOptional()
			: std::logic_error("Called get() on an Optional<" + std::string(typeid(T).name()) + "> which was not set") {} // using typeid is not optimal but better than nothing
	};

	// represents an optional type
	template <typename T>
	class Optional {
	public:
		Optional() {}

		Optional(const Optional& other) {
			m_data = other.m_data;
		}

		Optional& operator=(const Optional& other) {
			m_data = other.m_data;
			return *this;
		}

		Optional(Optional&& other) {
			m_data = std::move(other.m_data);
		}

		Optional& operator=(Optional&& other) {
			m_data = std::move(other.m_data);
			return *this;
		}

		template<typename U>
		Optional& operator=(const U& t) {
			m_data = t;
			return *this;
		}

		template<typename U>
		Optional& operator=(U&& t) {
			m_data = std::move(t);
			return *this;
		}

		// in-place constructs the optional, is used in case T is not copy- or moveable
		template <typename... U>
		void construct(U&&... args) {
			m_data = boost::in_place(std::forward<U>(args)...);
		}

		void destroy() {
			m_data = boost::none;
		}

		bool isValid() const {
			return static_cast<bool>(m_data);
		}

		T& get() {
			if (!isValid())
				throw UninitializedOptional<T>();
			return *m_data;
		}

		const T& get() const {
			if (!isValid())
				throw UninitializedOptional<T>();
			return *m_data;
		}

		T* const operator->() {
			return &get();
		}

		const T* const operator->() const {
			return &get();
		}

	private:
		boost::optional<T> m_data;
	};
}
