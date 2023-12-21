#include <cstdio>
#include "Logging.h"

void* Logging::m_print_callback_data = nullptr;
PrintCallback Logging::m_print_callback = nullptr;
PrintCallback Logging::m_error_callback = nullptr;

void Logging::SetPrintCallbacks(void* ptr, PrintCallback print_callback, PrintCallback error_callback)
{
	m_print_callback_data = ptr;
	m_print_callback = print_callback;
	m_error_callback = error_callback;
}

void Logging::print_std(const char* str)
{
	if (m_print_callback != nullptr)
	{
		m_print_callback(m_print_callback_data, str);
	}
	else
	{
		printf("%s\n", str);
	}
}

void Logging::print_err(const char* str)
{
	if (m_error_callback != nullptr)
	{
		m_error_callback(m_print_callback_data, str);
	}
	else
	{
		fprintf(stderr, "%s\n", str);
	}
}


