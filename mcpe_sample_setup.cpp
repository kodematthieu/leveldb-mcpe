#include <iostream>

#include <leveldb/comparator.h>
#include <leveldb/cache.h>
#include <leveldb/db.h>
#include <leveldb/iterator.h>
#include <leveldb/slice.h>
#include <leveldb/zlib_compressor.h>
#include <leveldb/filter_policy.h>

int main() {
	leveldb::DB* db = NULL;
	leveldb::Options options;
	options.block_cache = leveldb::NewLRUCache(8 << 20);
	options.filter_policy = leveldb::NewBloomFilterPolicy(10);
	options.create_if_missing = true;
	options.error_if_exists = false;
	options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);
	options.compressors[1] = new leveldb::ZlibCompressor();
	options.write_buffer_size = 4 << 20;
	options.block_size = 4096;
	options.max_open_files = 1000;
	options.block_restart_interval = 16;
	options.max_file_size = 2 << 20;
	leveldb::DB::Open(options, "test", &db);
}