#pragma once

typedef void (*PrintCallback)(void* ptr, const char* str);

class Logging
{
public:
	static void SetPrintCallbacks(void* ptr, PrintCallback print_callback, PrintCallback error_callback);
	static void print_std(const char* str);
	static void print_err(const char* str);

private:
	static void* m_print_callback_data;
	static PrintCallback m_print_callback;
	static PrintCallback m_error_callback;


};