#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "../nav_common/structs.hxx"
#include "EKF_15state.hxx"

// this is a glue class to bridge between the existing python API and
// the actual uNavINS class API.  This could be handled other ways,
// but it also hides the eigen3 usage in the uNavINS interfaces

class APIHelper {

public:

    APIHelper() {}
    ~APIHelper() {}

    // set/get error characteristics of navigation sensors
    void set_config(NAVconfig config) {
        config_save = config;
        
        // commit these values
        filt.set_config(config);
    }
    
    void update(IMUdata imu, GPSdata gps) {
        current_time = imu.time;
        if ( ! initialized ) {
            filt.init(imu, gps);
            initialized = true;
        } else {
            if ( gps.time <= last_gps_time ) {
                filt.time_update(imu);
            } else {
                last_gps_time = gps.time;
                filt.time_update(imu);
                filt.measurement_update(imu, gps);
            }
        }
    }
    
    NAVdata get_nav() {
        return filt.get_nav();
    }

private:

    NAVconfig config_save;
    float current_time = 0.0;
    float last_gps_time = 0.0;
    bool initialized = false;
    EKF15_mag filt;
    
};

PYBIND11_MODULE(ekf15_mag, m) {
    py::class_<APIHelper>(m, "EKF15_mag", py::module_local())
        .def(py::init<>())
        .def("set_config", &APIHelper::set_config)
        .def("update", &APIHelper::update)
        .def("get_nav", &APIHelper::get_nav)
    ;
}