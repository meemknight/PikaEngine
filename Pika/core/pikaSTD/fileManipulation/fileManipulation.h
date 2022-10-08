#pragma once


namespace pika
{

	size_t getFileSize(const char *name);
	bool readEntireFile(const char* name, void *buffer, size_t s);

	bool writeEntireFile(const char *name, void *buffer, size_t s);

	bool appendToFile(const char *name, void *buffer, size_t s);

	//todo implement
	void deleteFile(const char *f);

};