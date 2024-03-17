/**
 * @file logfile.h
 * @brief log to file at location hardcode, without config.ini
*/

#ifndef _log_file_h_
#define _log_file_h_

#include "logbase.h"
#include "logtty.h"

struct logfile:logbase{
    //FILE *finfo = nullptr , *ferror = nullptr , * fdebug = nullptr;
    logfile();
    logfile(const char*){}
    virtual ~logfile(){}
    FILE* openfile(const char*);
    static void writefile(const char* , const char*);
};
FILE* logfile::openfile(const char* p){
    if(nullptr == p) return nullptr;
    FILE* f = fopen(p , "a");
    if(nullptr == f){
        //mlerrno(locs);
        logtty l;
        l.error(locs , p);
        getc(stdin);
        exit(-1);
    }
    return f;
}
logfile::logfile(){
    infof = openfile("info.txt");
    errorf = openfile("error.txt");
    debugf = openfile("debug.txt");
}

/**
 * @brief early start processing may need this
 * @param fname file name
 * @param s content to write
*/
void logfile::writefile(const char* fname , const char* s){
    if(nullptr == fname || nullptr == s){
        //mlerrno(locs);
        logtty::instance().error(locs);
        getc(stdin);
        exit(-1);
    }
    FILE* f = fopen(fname , "a");
    if(nullptr == f){
        //mlerrno(locs);
        logtty::instance().error(locs);
        getc(stdin);
        exit(-1);
    }
    fwrite(s , 1 , strlen(s) , f);
}

#endif