/**
 * @file logbase.h
 * @brief base class for log
*/
#ifndef _log_base_h_
#define _log_base_h_

#include "common.h"
#include <cstdarg>

struct logbase{
    FILE *infof = stdout;
    FILE *debugf = stdout;
    FILE *errorf = stderr;
    virtual void info(const char*) const;
    //virtual void info(loc&& , const char*) const;
    virtual void info(loc&& , const char* , ...) const;
    virtual void error(const char*) const;
    virtual void error(loc&&) const;
    //virtual void error(loc&& , const char*) const;
    virtual void error(loc&& , const char* , ...)const;
    virtual void debug(const char*)const;
    //virtual void debug(loc&& , const char*)const;
    virtual void debug(loc&& , const char* , ...)const;
    
    template<typename ... A>
    void varfprintf(FILE* f , const char *fmt , A&& ... args)const{
        if(nullptr == fmt || nullptr == f) return;
        fprintf(f , fmt , std::forward<A>(args)...);
    }
    
    logbase(){};
    virtual ~logbase(){};
};

void logbase::info(const char* s)const{
    if(nullptr == s) return;
    varfprintf(infof , "info: %s %s\n" , timestr().get() , s);
}
/*void logbase::info(loc&& l , const char* s)const{
    if(nullptr == s) return;
    fprintf(infof , "%s %s %s %d: %s\n" , timestr().get() , l.file , l.func , l.line , s);
}*/
void logbase::info(loc&& l , const char* fmt , ...)const{
    if(nullptr == fmt) return;
    fprintf(infof , "%s %s %s %d: " , timestr().get() , l.file , l.func , l.line);
    va_list args;
    va_start(args , fmt);
    vfprintf(infof , fmt , args);
    va_end(args);
}
void logbase::error(const char* s)const{
    if(nullptr == s) return;
    fprintf(errorf , "%s" , timestr().get());
    fprintf(errorf , "error: %s\n" , s);
}
void logbase::error(loc&& l)const{
    char b[UINT8_MAX]{};
    fprintf(errorf , "%s %s %s %d: " , timestr().get() , l.file , l.func , l.line);
    auto ne = errno;
    errno = 0;
    auto r = strerror_r(ne , b , UINT8_MAX);
    /*#if __arm__
    if(r != nullptr)
    #elif __CYGWIN__
    if(r == 0)
    #endif
    {
        fprintf(errorf , "%s" , b);
    }*/
    if(errno == 0){
        fprintf(errorf , "%s\n" , b);
    }else{
        fprintf(errorf , "errno: %d and resolv errno: %d\n" , ne , errno);
    }
}
/*
void logbase::error(loc&& l , const char *s)const{
    char b[UINT8_MAX]{};
    fprintf(errorf , "%s " , timestr().get());
    int r = strerror_r(errno , b , UINT8_MAX);
    if(r == 0) fprintf(errorf , "%s %s %d: %s: %s\n" , l.file , l.func , l.line , b , s);
}*/
void logbase::error(loc&& l , const char* s , ...)const{
    if(nullptr == s) return;
    fprintf(errorf , "%s " , timestr().get());
    fprintf(errorf , "%s %s %d: " , l.file , l.func , l.line);
    va_list vargs;
    va_start(vargs , s);
    vfprintf(errorf , s , vargs);
    va_end(vargs);
}
void logbase::debug(const char* s)const{
    if(nullptr == s) return;
    fprintf(debugf , "%s",timestr().get());
    fprintf(debugf , "debug: %s\n" , s);
}
void logbase::debug(loc&& l , const char* fmt , ...)const{
    if(nullptr == fmt) return;
    fprintf(debugf , "%s %s %s %d: " , timestr().get() , l.file , l.func , l.line);
    va_list args;
    va_start(args , fmt);
    vfprintf(debugf , fmt , args);
    va_end(args);
}
#endif
