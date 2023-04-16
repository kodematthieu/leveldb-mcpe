#include <iostream>

#include <leveldb/comparator.h>
#include <leveldb/cache.h>
#include <leveldb/db.h>
#include <leveldb/iterator.h>
#include <leveldb/slice.h>
#include <leveldb/zlib_compressor.h>

int main() {
	leveldb::Options options;
	options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);
	options.compressors[1] = new leveldb::ZlibCompressor();
}