#include <gazebo/gazebo.hh>


#include <boost/bind.hpp>

#include "gazebo/gazebo.hh"
#include "gazebo/common/Plugin.hh"
#include "gazebo/msgs/msgs.hh"
#include "gazebo/physics/physics.hh"
#include "gazebo/transport/transport.hh"

#include <chrono>
#include <thread>



namespace gazebo
{
    class SimStepWorldPlugin : public WorldPlugin{

        public:

            SimStepWorldPlugin(): WorldPlugin(){}


            void Load(physics::WorldPtr _parent, sdf::ElementPtr _sdf) {
                // Create a new transport node
                _world = _parent;
                this->node.reset(new transport::Node());

                // Initialize the node with the world name
                node->Init(_parent->Name());
                //cmd_node->Init(_parent->Name());

                // Create a publisher
                pub = node->Advertise<msgs::WorldControl>("~/world_control");
                //cmd_node->Subscribe

                 // Listen to the update event. Event is broadcast every simulation
                 // iteration.
                 this->updateConnection = event::Events::ConnectWorldUpdateEnd(
                 boost::bind(&SimStepWorldPlugin::OnUpdate, this));


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
            void OnUpdate(){
                // Throttle Publication
                gazebo::common::Time::MSleep(1000);
                msgs::WorldControl msg;
                msg.set_step(1);
                pub->Publish(msg);
                //std::cout << "Publishing OnUpdate." << std::endl;
            }


        private:
            // Pointer to the world_controller
            physics::WorldPtr _world;
            transport::NodePtr node;
            transport::PublisherPtr pub;
            event::ConnectionPtr updateConnection;

            gazebo::msgs::ServerControl serverMsg;

    };

    GZ_REGISTER_WORLD_PLUGIN(SimStepWorldPlugin)
}
