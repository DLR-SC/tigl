#pragma once

#include <boost/variant.hpp>

#include "Optional.hpp"

namespace tigl {
	template <typename... Ts>
	class Variant {
	public:
		Variant() {}

		template<typename T>
		Variant(const T& t) {
			m_data = t;
		}

		Variant(const Variant& other) {
			m_data = other.m_data;
		}

		auto& operator=(const Variant& other) {
			m_data = other.m_data;
			return *this;
		}

		Variant(Variant&& other) {
			m_data = std::move(other.m_data);
		}

		auto& operator=(Variant&& other) {
			m_data = std::move(other.m_data);
			return *this;
		}

		template<typename T>
		auto& operator=(const T& t) {
			m_data = t;
			return *this;
		}

		template<typename Visitor>
		void visit(Visitor func) {
			if (m_data.isValid())
				m_data->apply_visitor(VisitorWrapper<Visitor>(func));
		}

		template<typename Visitor>
		void visit(Visitor func) const {
			if (m_data.isValid())
				m_data->apply_visitor(VisitorWrapper<Visitor>(func));
		}

	private:
		// adapts a visitor for boost
		template <typename Func>
		struct VisitorWrapper : public boost::static_visitor<> {
			VisitorWrapper(Func func)
				: m_func(func) {}

			template<typename T>
			void operator()(T&& arg) {
				m_func(std::forward<T>(arg));
			}

		private:
			Func m_func;
		};

		Optional<boost::variant<Ts...>> m_data;
	};
}
