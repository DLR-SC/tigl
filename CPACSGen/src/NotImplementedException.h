#pragma once

#include <stdexcept>
#include <string>

class NotImplementedException : public std::exception {
public:
	NotImplementedException(const std::string& msg)
		: m_msg(msg) {}

	virtual const char* what() const override {
		return m_msg.c_str();
	}

private:
	std::string m_msg;
};
