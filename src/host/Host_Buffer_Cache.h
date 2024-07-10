// * hoonhwi

#include <vector>
#include <deque>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include "Host_IO_Request.h"

extern int testbit;

// * hoonhwi; buffer cache
const bool PAGE_TABLE_ON = true;
const size_t PAGE_SIZE = 4096;
const size_t MAX_PAGE_TABLE_SIZE = 64;
const size_t MAX_BUFFER_CACHE_SIZE = 64;
const size_t buffer_cache_time_coeff = 1000;

extern sim_time_type buffer_cache_read_time;
extern sim_time_type buffer_cache_write_time;

struct PageTableEntry {
	size_t virtualPageNumber;
    size_t frameNumber;
    bool valid;
};
enum class Host_IO_Request_Type { READ, WRITE };
struct BufferCacheEntry {
	sim_time_type Arrival_time;
	sim_time_type Enqueue_time;
	LHA_type Start_LBA;
	unsigned int LBA_count;
	Host_IO_Request_Type Type;
	uint16_t IO_queue_info;
	uint16_t Source_flow_id;
    bool valid;
};
class PageTable {
private:
    std::vector<PageTableEntry> pageTable;
	std::deque<size_t> lruQueue;
	void updateLRU(size_t index);
public:
	PageTable();
    int64_t translate_pageTable(size_t virtualAddress);
    int map_pageTable(size_t virtualPageNumber, size_t physicalFrameNumber);
    void unmap_pageTable(size_t virtualPageNumber);
};
class BufferCache {
private:
	std::vector<BufferCacheEntry> bufferCache;
	std::deque<size_t> lruQueue;
	void updateLRU(size_t index);
public:
	BufferCache();
	BufferCacheEntry translate_bufferCache(Host_Components::Host_IO_Request request);
	BufferCacheEntry update_dirty(Host_Components::Host_IO_Request request);
	BufferCacheEntry select_one();
	bool empty();
    void map_bufferCache(Host_Components::Host_IO_Request request);
    void unmap_bufferCache(Host_Components::Host_IO_Request request);	
};

// *