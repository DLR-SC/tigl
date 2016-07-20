#pragma once

#include <memory>
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
				throw UninitializedOptional<T>();
			return *m_ptr;
		}

		const T& get() const {
			if (!isValid())
				throw UninitializedOptional<T>();
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
}
