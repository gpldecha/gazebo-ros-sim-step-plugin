#include <boost/python.hpp>
#include <boost/interprocess/xsi_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>

using namespace boost::interprocess;

#ifndef _XSI_H
#define _XSI_H

class Xsi{
public:

    Xsi(){

        std::cout<< "\033[1;32m" << "Xsi [constructor start]" << "\033[0m" << std::endl;

        char *path = "/usr";

        xsi_key key(path, 1);

        try {
            shm = new xsi_shared_memory(open_or_create, key, 1);
        }catch(boost::interprocess::interprocess_exception &e){
            std::cout<< "\\033[1;31m" << "Xsi " << e.what() << "\033[0m" << std::endl;
        }

            //Map the whole shared memory in this process
        region = new mapped_region(*shm, read_write);

        std::cout<< "\033[1;32m" << "Xsi [constructor end]" << "\033[0m" << std::endl;
    }

    ~Xsi(){
        //Remove shared memory on destruction
        struct shm_remove
        {
            int shmid_;
            shm_remove(int shmid) : shmid_(shmid){}
            ~shm_remove(){ xsi_shared_memory::remove(shmid_); }
        } remover(shm->get_shmid());
    }

    void step(){
        std::memset(region->get_address(), i, region->get_size());
        i=(i+1)%128;
    }

    void initialise_step(){
        std::memset(region->get_address(), 0, region->get_size());
    }

    void run(){
        std::memset(region->get_address(), -1, region->get_size());
    }

    void stop(){
        std::memset(region->get_address(), -2, region->get_size());
    }

private:

    xsi_shared_memory* shm;
    mapped_region* region;
    int i = 0;
};

#endif // _XSI_H


BOOST_PYTHON_MODULE(pyipc)
{
    using namespace boost::python;
    class_<Xsi>("Xsi")
            .def("step", &Xsi::step)
            .def("initialise_step", &Xsi::initialise_step)
            .def("run",  &Xsi::run)
            .def("stop", &Xsi::stop)
    ;
}
