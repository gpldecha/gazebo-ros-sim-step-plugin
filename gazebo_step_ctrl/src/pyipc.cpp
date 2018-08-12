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

        char *path = "/usr";

        xsi_key key(path, 1);

        shm = new xsi_shared_memory(open_only, key);

        //Map the whole shared memory in this process
        region = new mapped_region(*shm, read_write);
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

    void ping(){
        std::memset(region->get_address(), i, region->get_size());
        i=(i+1)%128;
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
            .def("ping", &Xsi::ping)
            ;
}
