// * hoonhwi

#include <vector>
#include <deque>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include "Host_IO_Request.h"

#include <list>
#include <cstdint>
#include "../sim/Sim_Defs.h"
#include "../sim/Sim_Object.h"
#include "../sim/Sim_Event.h"
#include"../sim/Engine.h"
#include "../host/IO_Flow_Base.h"
#include "../host/PCIe_Switch.h"
#include "../ssd/Host_Interface_Defs.h"

extern int testbit;

// * hoonhwi; buffer cache
extern bool PAGE_TABLE_ON;
const size_t PAGE_SIZE = 4096;
const size_t CACHE_LINE_SIZE = 64; // byte
extern size_t DIMM_MEMORY_SIZE_KB;
const size_t KB = 1024;
const size_t MB = 1024*1024;
extern size_t MAX_PAGE_TABLE_ENTRY_SIZE;
const size_t MAX_BUFFER_CACHE_SIZE = 64;
const size_t buffer_cache_time_coeff = 1;

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
	std::deque<size_t> lruQueue;
	void updateLRU(size_t index);
	std::unordered_map<size_t, size_t> pageMap;
public:
	std::vector<PageTableEntry> pageTable;
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
namespace Host_Components {
	class Host_Buffer :public MQSimEngine::Sim_Object {
	public:
		Host_Buffer(const sim_object_id_type& id);
		void Execute_simulator_event(MQSimEngine::Sim_Event* event);
		void Start_simulation();
		void Validate_simulation_config();
	};
}
// *