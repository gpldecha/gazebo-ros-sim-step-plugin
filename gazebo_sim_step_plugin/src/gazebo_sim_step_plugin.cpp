#include <gazebo/gazebo.hh>

namespace gazebo
{
    class SimStepWorldPlugin : public WorldPlugin{

        public:

            SimStepWorldPlugin(): WorldPlugin(){
                printf("Hello World!\n");
            }


            void Load(physics::WorldPtr _world, sdf::ElementPtr _sdf) {
            }


    };

    GZ_REGISTER_WORLD_PLUGIN(SimStepWorldPlugin)
}