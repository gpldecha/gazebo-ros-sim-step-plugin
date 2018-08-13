#include <boost/interprocess/xsi_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/bind.hpp>

#include <gazebo/gazebo.hh>
#include <gazebo/common/Plugin.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/transport.hh>

#include <chrono>
#include <thread>

void remove_old_shared_memory(const boost::interprocess::xsi_key &key)
{
    try{
        boost::interprocess::xsi_shared_memory xsi(boost::interprocess::open_only, key);
        boost::interprocess::xsi_shared_memory::remove(xsi.get_shmid());
    }
    catch(boost::interprocess::interprocess_exception &e){
        std::cerr<< "remove_old_shared_memory error: " << e.what() << std::endl;
        if(e.get_error_code() != boost::interprocess::not_found_error){
            throw;
        }
    }
}

bool exit_flag = false;
void singal_handler(int s){
    exit_flag = true;
}

namespace gazebo
{
class SimStepWorldPlugin : public WorldPlugin{

public:

    SimStepWorldPlugin(): WorldPlugin(){
        std::cout<< "SimStepWolrdPlugin [start]" << std::endl;
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = singal_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);

        using namespace boost::interprocess;

        char *path = "/usr";

        xsi_key key(path, 1);

        remove_old_shared_memory(key);

        shm = new xsi_shared_memory(create_only, key, 1);
        region = new mapped_region(*shm, read_write);
        std::cout<< "address: " << region->get_address() << std::endl;
        shared_mem = static_cast<char*>(region->get_address());
        flag_c = 0;
        std::cout<< "memory size: " << region->get_size() << std::endl;
        std::memset(region->get_address(), flag_c, region->get_size());
        std::cout<< "flag_c: " << (int)shared_mem[0] << std::endl;
        std::cout<< "SimStepWolrdPlugin  [end]" << std::endl;
    }

    ~SimStepWorldPlugin(){
        struct shm_remove
        {
            int shmid_;
            shm_remove(int shmid) : shmid_(shmid){}
            ~shm_remove(){ boost::interprocess::xsi_shared_memory::remove(shmid_); }
        } remover(shm->get_shmid());
    }

    void Load(physics::WorldPtr _world, sdf::ElementPtr _sdf) {
        // Create a new transport node
        node.reset(new transport::Node());
        // Initialize the node with the world name
        node->Init(_world->Name());
        // Create a publisher
        pub = node->Advertise<msgs::WorldControl>("~/world_control");
        // Listen to the update event. Event is broadcast every simulation
        // iteration.
        updateConnection = event::Events::ConnectWorldUpdateEnd(
                    boost::bind(&SimStepWorldPlugin::one_update, this));


        // Configure the initial message to the system
        msgs::WorldControl worldControlMsg;

        // Set the world to paused
        worldControlMsg.set_pause(0);

        // Set the step flag to true
        worldControlMsg.set_step(1);

        // Publish the initial message.
        pub->Publish(worldControlMsg);
        std::cout << "Publishing Load." << std::endl;
    }

    // Called by the world update start event.
    void one_update(){
       /*if(shared_mem == NULL)
            return;*/
       /*if((int)shared_mem[0] == -1)
           return;

       while((flag_c == (int)shared_mem[0]) && !exit_flag){std::this_thread::sleep_for(std::chrono::microseconds(5));}
       flag_c = (int)shared_mem[0];*/

       //if(exit_flag){std::this_thread::sleep_for(std::chrono::seconds(1)); }
    }

private:
    // Pointer to the world_controller
    transport::NodePtr node;
    transport::PublisherPtr pub;
    event::ConnectionPtr updateConnection;

    boost::interprocess::xsi_shared_memory* shm;
    boost::interprocess::mapped_region* region;
    char *shared_mem;
    int flag_c;
};

GZ_REGISTER_WORLD_PLUGIN(SimStepWorldPlugin)
}
