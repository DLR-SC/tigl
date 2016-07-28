#pragma once

#include <stdexcept>
#include <string>

namespace {
	class NotImplementedException : public std::exception {
	public:
		NotImplementedException(const std::string& msg)
			: m_msg(msg) {}

		virtual const char* what() const throw() override {
			return m_msg.c_str();
		}

	private:
		std::string m_msg;
	};
}
