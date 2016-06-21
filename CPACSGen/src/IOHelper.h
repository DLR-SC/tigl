#pragma once

#include <tixi.h>

#include <string>

std::string errorToString(ReturnCode ret);

class TixiError : public std::exception {
public:
	TixiError(ReturnCode ret);
	TixiError(ReturnCode ret, const std::string& message);

	virtual const char* what() const override;

private:
	ReturnCode m_ret;
	std::string m_message;
};

bool TixiCheckAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
bool TixiCheckElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);

std::string TixiGetTextAttribute   (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
double      TixiGetDoubleAttribute (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);
bool        TixiGetBoolAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute);

std::string TixiGetTextElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
double      TixiGetDoubleElement(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
bool        TixiGetBoolElement  (const TixiDocumentHandle& tixiHandle, const std::string& xpath);
int         TixiGetIntElement   (const TixiDocumentHandle& tixiHandle, const std::string& xpath);

void TixiSaveTextAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, const std::string& value);
void TixiSaveDoubleAttribute(const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, double value);
void TixiSaveBoolAttribute  (const TixiDocumentHandle& tixiHandle, const std::string& xpath, const std::string& attribute, bool value);

void TixiSaveTextElement   (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, const std::string& value);
void TixiSaveDoubleElement (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, double value);
void TixiSaveBoolElement   (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, bool value);
void TixiSaveIntElement    (const TixiDocumentHandle& tixiHandle, const std::string& parentXPath, const std::string& element, int value);
