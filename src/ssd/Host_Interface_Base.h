#ifndef HOST_INTERFACE_BASE_H
#define HOST_INTERFACE_BASE_H

#include <vector>
#include "../sim/Sim_Object.h"
#include "../sim/Sim_Reporter.h"
#include "../host/PCIe_Switch.h"
#include "../host/PCIe_Message.h"
#include "User_Request.h"
#include "Data_Cache_Manager_Base.h"
#include "../cxl/DRAM_Model.h"
#include <stdint.h>
#include <cstring>

namespace Host_Components
{
	class PCIe_Switch;
}

namespace SSD_Components
{
#define COPYDATA(DEST,SRC,SIZE) if(Simulator->Is_integrated_execution_mode()) {DEST = new char[SIZE]; memcpy(DEST, SRC, SIZE);} else DEST = SRC;
#define DELETE_REQUEST_NVME(REQ) \
	delete (Submission_Queue_Entry*)REQ->IO_command_info; \
	if(Simulator->Is_integrated_execution_mode())\
		{if(REQ->Data != NULL) delete[] (char*)REQ->Data;} \
	if(REQ->Transaction_list.size() != 0) PRINT_ERROR("Deleting an unhandled user requests in the host interface! MQSim thinks something is going wrong!")\
	delete REQ;

	class Data_Cache_Manager_Base;
	class CXL_DRAM_Model;
	class Host_Interface_Base;

	class Input_Stream_Base
	{
	public:
		Input_Stream_Base();
		virtual ~Input_Stream_Base();
		unsigned int STAT_number_of_read_requests;
		unsigned int STAT_number_of_write_requests;
		unsigned int STAT_number_of_read_transactions;
		unsigned int STAT_number_of_write_transactions;
		sim_time_type STAT_sum_of_read_transactions_execution_time, STAT_sum_of_read_transactions_transfer_time, STAT_sum_of_read_transactions_waiting_time;
		sim_time_type STAT_sum_of_write_transactions_execution_time, STAT_sum_of_write_transactions_transfer_time, STAT_sum_of_write_transactions_waiting_time;
		unsigned STAT_SYNC_READ, STAT_SYNC_WRITE;
	};

	class Input_Stream_Manager_Base
	{
		friend class Request_Fetch_Unit_Base;
		friend class Request_Fetch_Unit_NVMe;
		friend class Request_Fetch_Unit_CXL;
		friend class Request_Fetch_Unit_SATA;
		friend class CXL_Manager;
	public:
		Input_Stream_Manager_Base(Host_Interface_Base* host_interface);
		virtual ~Input_Stream_Manager_Base();
		virtual void Handle_new_arrived_request(User_Request* request) = 0;
		virtual void Handle_arrived_write_data(User_Request* request) = 0;
		virtual void Handle_serviced_request(User_Request* request) = 0;
		void Update_transaction_statistics(NVM_Transaction* transaction);
		// *hoonhwi
		uint32_t Get_stat_sync_write(stream_id_type stream_id);
		uint32_t Get_stat_sync_read(stream_id_type stream_id);
		uint32_t Get_number_of_read_transactions(stream_id_type stream_id);
		uint32_t Get_number_of_write_transactions(stream_id_type stream_id);
		// *
		double Get_average_read_transaction_turnaround_time(stream_id_type stream_id);//in microseconds
		double Get_average_read_transaction_execution_time(stream_id_type stream_id);//in microseconds
		double Get_average_read_transaction_transfer_time(stream_id_type stream_id);//in microseconds
		double Get_average_read_transaction_waiting_time(stream_id_type stream_id);//in microseconds
		double Get_average_write_transaction_turnaround_time(stream_id_type stream_id);//in microseconds
		double Get_average_write_transaction_execution_time(stream_id_type stream_id);//in microseconds
		double Get_average_write_transaction_transfer_time(stream_id_type stream_id);//in microseconds
		double Get_average_write_transaction_waiting_time(stream_id_type stream_id);//in microseconds
	protected:
		Host_Interface_Base* host_interface;
		virtual void segment_user_request(User_Request* user_request) = 0;
		std::vector<Input_Stream_Base*> input_streams;
	};

	class Request_Fetch_Unit_Base
	{
	public:
		Request_Fetch_Unit_Base(Host_Interface_Base* host_interface);
		virtual ~Request_Fetch_Unit_Base();
		virtual void Fetch_next_request(stream_id_type stream_id) = 0;
		virtual void Fetch_write_data(User_Request* request) = 0;
		virtual void Send_read_data(User_Request* request) = 0;
		virtual void Process_pcie_write_message(uint64_t, void *, unsigned int) = 0;
		virtual void Process_pcie_read_message(uint64_t, void *, unsigned int) = 0;
	protected:
		enum class DMA_Req_Type { REQUEST_INFO, WRITE_DATA };
		struct DMA_Req_Item
		{
			DMA_Req_Type Type;
			void * object;
		};
		Host_Interface_Base* host_interface;
		std::list<DMA_Req_Item*> dma_list;
	};

	class Host_Interface_Base : public MQSimEngine::Sim_Object, public MQSimEngine::Sim_Reporter
	{
		friend class Input_Stream_Manager_Base;
		friend class Input_Stream_Manager_NVMe;
		friend class Input_Stream_Manager_CXL;
		friend class Input_Stream_Manager_SATA;
		friend class Request_Fetch_Unit_Base;
		friend class Request_Fetch_Unit_NVMe;
		friend class Request_Fetch_Unit_CXL;
		friend class Request_Fetch_Unit_SATA;
		friend class CXL_Manager;
	public:
		Host_Interface_Base(const sim_object_id_type& id, HostInterface_Types type, LHA_type max_logical_sector_address, 
			unsigned int sectors_per_page, Data_Cache_Manager_Base* cache);
		virtual ~Host_Interface_Base();
		void Setup_triggers();
		void Validate_simulation_config();

		typedef void(*UserRequestArrivedSignalHandlerType) (User_Request*);
		void Connect_to_user_request_arrived_signal(UserRequestArrivedSignalHandlerType function)
		{
			connected_user_request_arrived_signal_handlers.push_back(function);
		}

		virtual void Consume_pcie_message(Host_Components::PCIe_Message* message)
		{ //virtual is added so Host_interface_cxl can override this method
			if (message->Type == Host_Components::PCIe_Message_Type::READ_COMP) {
				request_fetch_unit->Process_pcie_read_message(message->Address, message->Payload, message->Payload_size);
			} else {
				request_fetch_unit->Process_pcie_write_message(message->Address, message->Payload, message->Payload_size);
			}
			delete message;
		}

		virtual void Update_CXL_DRAM_state(bool rw, uint64_t lba, bool& falsehit){
		}
		virtual void Update_CXL_DRAM_state_when_miss_data_ready(bool rw, uint64_t lba, bool serviced_before, bool& completed_removed_from_mshr) {
		}
		virtual void Handle_CXL_false_hit(bool rw, uint64_t lba){}
		virtual void Notify_DRAM_is_free() {}
		virtual uint64_t Get_flush_count() { return 0; }
		virtual void print_prefetch_info(){}
		
		void Notify_DRAM_is_full();
		void Notify_host_DRAM_is_free();
		void Notify_CXL_Host_request_complete();
		void Notify_CXL_Host_mshr_full();
		void Notify_CXL_Host_mshr_not_full();
		void Notify_CXL_Host_flash_full();
		void Notify_CXL_Host_flash_not_full();
	
		void Send_read_message_to_host(uint64_t addresss, unsigned int request_read_data_size);
		void Send_write_message_to_host(uint64_t addresss, void* message, unsigned int message_size);

		HostInterface_Types GetType() { return type; }
		void Attach_to_device(Host_Components::PCIe_Switch* pcie_switch);
		LHA_type Get_max_logical_sector_address();
		unsigned int Get_no_of_LHAs_in_an_NVM_write_unit();
	protected:
		HostInterface_Types type;
		LHA_type max_logical_sector_address;
		unsigned int sectors_per_page;
		unsigned int sectors_per_subpage; 
		static Host_Interface_Base* _my_instance;
		Input_Stream_Manager_Base* input_stream_manager;
		Request_Fetch_Unit_Base* request_fetch_unit;
		Data_Cache_Manager_Base* cache;
		CXL_DRAM_Model* cxl_dram;
		std::vector<UserRequestArrivedSignalHandlerType> connected_user_request_arrived_signal_handlers;

		void broadcast_user_request_arrival_signal(User_Request* user_request)
		{
			for (std::vector<UserRequestArrivedSignalHandlerType>::iterator it = connected_user_request_arrived_signal_handlers.begin();
				it != connected_user_request_arrived_signal_handlers.end(); it++) {
				(*it)(user_request);
			}
		}

		static void handle_user_request_serviced_signal_from_cache(User_Request* user_request)
		{
			_my_instance->input_stream_manager->Handle_serviced_request(user_request);
		}

		static void handle_user_memory_transaction_serviced_signal_from_cache(NVM_Transaction* transaction)
		{
			_my_instance->input_stream_manager->Update_transaction_statistics(transaction);
		}
	private:
		Host_Components::PCIe_Switch* pcie_switch;
	};
}

#endif // !HOST_INTERFACE_BASE_H
