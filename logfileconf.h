/**
 * @file logfileconf.h
 * @brief file init with config.ini
*/

#ifndef _log_file_conf_h_
#define _log_file_conf_h_

#include "logfile.h"
#include "config.h"
#include <string>
using std::string;
#include <filesystem>
//namespace fs = std::filesystem;
using std::filesystem::create_directory , std::filesystem::exists , std::filesystem::path;

struct logfileconf: logfile{
    logfileconf();
    ~logfileconf(){};
};

logfileconf::logfileconf():logfile("call no op constructor here"){
    path p(config::instance().getdir());
    std::error_code ec;
    const char* logdp = config::instance().get("logdir");
    if(nullptr != logdp){
        p += logdp;
    }
    if(!exists(p)){
        bool rb = create_directory(p , ec);
        if(!rb){
            writefile("baselog.txt" , ec.message().c_str());
            exit(-1);
        }
    }
    infof = openfile((p / "info.txt").c_str());
    errorf = openfile((p / "error.txt").c_str());
    debugf = openfile((p / "debug.txt").c_str());
}
#endif