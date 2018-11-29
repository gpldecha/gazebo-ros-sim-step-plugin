#include <boost/interprocess/xsi_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <gazebo/gazebo.hh>
#include <gazebo/common/Plugin.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>

#include <ros/ros.h>
#include "gazebo_step_ctrl/Cmd.h"

#include <chrono>
#include <thread>
#include <ros/node_handle.h>

namespace gazebo {

class SimStepWorldPlugin : public WorldPlugin{

public:

    enum class STATE { RUN, STOP, STEP };

public:

    SimStepWorldPlugin(): nh_ptr(NULL), WorldPlugin(){
        std::cout<< "\033[1;32m" << "SimStepWolrdPlugin [construct start]" << "\033[0m" << std::endl;

        using namespace boost::interprocess;

        char *path = "/usr";

        xsi_key key(path, 1);

        try{
            shm = new xsi_shared_memory(open_or_create, key, 1);
        }catch(boost::interprocess::interprocess_exception &e){
            std::cout<< "\\033[1;31m" << "SimStepWolrdPlugin " << e.what() << "\033[0m" << std::endl;
        }

        region = new mapped_region(*shm, read_write);
        shared_mem = static_cast<char*>(region->get_address());
        flag_c = 0;
        std::memset(region->get_address(), -1, region->get_size());
        current_state = STATE::RUN;
        previous_state = STATE::RUN;
        std::cout<< "\033[1;32m" << "SimStepWolrdPlugin [construct finished]" << "\033[0m" << std::endl;
    }

    ~SimStepWorldPlugin() override{
        struct shm_remove
        {
            int shmid_;
            shm_remove(int shmid) : shmid_(shmid){}
            ~shm_remove(){ boost::interprocess::xsi_shared_memory::remove(shmid_); }
        } remover(shm->get_shmid());
    }

    bool cmd_callback(gazebo_step_ctrl::Cmd::Request& req, gazebo_step_ctrl::Cmd::Response& res){
        res.str = "none";
        std::vector<std::string> elements;
        boost::split(elements, req.str, boost::is_any_of(" "), boost::token_compress_on);
        if(elements.size() < 2){
            std::cout<< req.str << " could not be parsed" << std::endl;
            return false;
        }
        if(elements[0] == "get"){
            if(elements[1] == "max_step_size"){
                res.str = std::to_string(world_ptr->Physics()->GetMaxStepSize());
            }
        }else if(elements[0] == "set"){
        }else{
            std::cout<< "no such command: " << elements[0] << " supported " << std::endl;
        }
        return true;
    }

    void Load(physics::WorldPtr _world, sdf::ElementPtr _sdf) override {
        world_ptr = _world;
        int argc = 0;
        char** argv = NULL;
        ros::init(argc, argv, "gazebo_step_ctrl", ros::init_options::NoSigintHandler);
        nh_ptr = new ros::NodeHandle();
        service = nh_ptr->advertiseService("gazebo_step_ctrl/cmd", &gazebo::SimStepWorldPlugin::cmd_callback, this);
        ros::spinOnce();

        updateConnection = event::Events::ConnectWorldUpdateEnd(
                    boost::bind(&SimStepWorldPlugin::one_update, this));
    }

    void stop(){
        while(((int)shared_mem[0] == -2)){
            current_state = STATE::STOP;
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            ros::spinOnce();
        }
    }

    void step(){
        while((flag_c == (int)shared_mem[0])){
            ros::spinOnce();
        }
        flag_c = (int)shared_mem[0];
    }

    void one_update(){
        ros::spinOnce();

        if(shared_mem == NULL)
            return;

        previous_state = current_state;
        switch ((int)shared_mem[0]){
            case -1: current_state = STATE::RUN; break;
            case -2: current_state = STATE::STOP; break;
            default: current_state = STATE::STEP; break;
        }
        if(current_state != previous_state) {
            std::string msg;
            switch (current_state){
                case STATE::RUN:  msg = "state: RUN"; break;
                case STATE::STOP:  msg = "state: STOP"; break;
                case STATE::STEP: msg = "state: STEP"; break;
            }
        }

        if(current_state == STATE::RUN){
            return;
        }else if(current_state == STATE::STOP){
            stop();
        }else if(current_state == STATE::STEP){
            step();
        }
    }

private:
    // Pointer to the world_controller
    transport::NodePtr node;
    event::ConnectionPtr updateConnection;
    physics::WorldPtr world_ptr;
    STATE current_state, previous_state;

    ros::NodeHandle* nh_ptr;
    ros::ServiceServer service;

    boost::interprocess::xsi_shared_memory* shm;
    boost::interprocess::mapped_region* region;
    char *shared_mem;
    int flag_c;
};

GZ_REGISTER_WORLD_PLUGIN(SimStepWorldPlugin)
}
