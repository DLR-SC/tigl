#pragma once

#include <tixi.h>

#include <string>

std::string errorToString(ReturnCode ret);

class TixiError : public std::exception {
public:
	TixiError(ReturnCode ret)
		: m_ret(ret) {
		m_message = "ReturnCode: " + errorToString(m_ret);
	}

	TixiError(ReturnCode ret, const std::string& message)
		: m_ret(ret) {
		m_message = message + "\nReturnCode: " + errorToString(m_ret);
	}

	virtual const char* what() const override {
		return m_message.c_str();
	}

private:
	ReturnCode m_ret;
	std::string m_message;
};

bool TixiCheckAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

std::string TixiGetTextAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
std::string TixiGetTextElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

void TixiSaveTextAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
void TixiSaveTextElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
