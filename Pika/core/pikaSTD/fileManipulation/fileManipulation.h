#pragma once


namespace pika
{

	size_t getFileSize(const char *name);
	size_t readEntireFile(const char* name, void *buffer, size_t s, size_t from = 0);

	bool writeEntireFile(const char *name, void *buffer, size_t s);

	bool appendToFile(const char *name, void *buffer, size_t s);

	//todo implement
	void deleteFile(const char *f);

};