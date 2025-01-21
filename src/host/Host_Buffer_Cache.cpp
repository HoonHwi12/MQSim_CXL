#include "Host_Buffer_Cache.h"

namespace Host_Components {
	Host_Buffer::Host_Buffer(const sim_object_id_type& id) : Sim_Object(id) {

	}
    void Host_Buffer::Execute_simulator_event(MQSimEngine::Sim_Event* event) {
    }
    void Host_Buffer::Start_simulation(){}
	void Host_Buffer::Validate_simulation_config() {}
}