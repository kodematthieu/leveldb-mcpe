
#pragma once

#include "leveldb/compressor.h"

namespace leveldb {

	class DLLX ZlibCompressorBase : public Compressor 
	{
	public:
		int inflate(const char* input, size_t length, ::std::string &output) const;

		const int compressionLevel;
		const bool raw;
        
        virtual ~ZlibCompressorBase() {}

		ZlibCompressorBase(char uniqueCompressionID, int compressionLevel, bool raw);

		virtual void compressImpl(const char* input, size_t length, ::std::string& output) const override;

		virtual bool decompress(const char* input, size_t length, ::std::string &output) const override;

	private:

		int _window() const;

	};

	class DLLX ZlibCompressor : public ZlibCompressorBase {
	public:
		static const int SERIALIZE_ID = 2;

		ZlibCompressor(int compressionLevel = -1);
	};

	class DLLX ZlibCompressorRaw : public ZlibCompressorBase {
	public:
		static const int SERIALIZE_ID = 4;

		ZlibCompressorRaw(int compressionLevel = -1);
	};
}