/**
 * @file udbcb.cc
 * udp server with callback
*/

#include "config.h"
#include "ev.h"
#include "log.h"
#include "socket.h"

#include <map>
#include <vector>
#include <memory>
#include <functional>

using std::unique_ptr , std::make_unique , std::shared_ptr , std::make_shared;
using std::map , std::vector , std::function;
using usnbT = udpsocket<nonblocktype>;
using ucnbT = udpclient<nonblocktype>;
using spsT = shared_ptr<sockbase>;
using upsT = unique_ptr<sockbase>;

int main(int argc , char* argv[]){
    fprintf(stdout , "start ***********************\n");
    config::init(argv[0]);
    evs<fd_set> ev;
    const char* listenip = config::instance().get("listenip");
    if(nullptr == listenip){
        log::instance().info("please config the listen address");
        return -1;
    }
    const char* serverip = config::instance().get("serverip");
    if(nullptr == serverip){
        log::instance().info("please config the dns server address");
        return -1;
    }

    log::instance().debug(locs , "listen: %s\n" , listenip);
    log::instance().debug(locs , "query: %s\n" , serverip);
    
    map<int , shared_ptr<sockbase>> msp;
    map<int , int> mss;
    while(1){
        log::instance().debug("--------------------");
        ev.zeroall();
        shared_ptr<sockbase> sps{make_shared<usnbT>(listenip , 53)};
        log::instance().debug(locs , "socket %d listen\n" , sps->getsocket());
        //shared_ptr<sockbase> spc = make_shared<usnbT>(serverip , 53);
        sps->setreuse();
        sps->bind();
        sps->nonblock();
        sps->wantread();
        sps->callback = [&,sps]() mutable{
            shared_ptr<sockbase> spc = make_shared<ucnbT>(serverip , 53);
            log::instance().debug(locs , "socket %d relay to server\n" , spc->getsocket());
            auto cr = ::connect(spc->getsocket() , (sockaddr*)&spc->sin , sizeof(spc->sin));
            if(cr < 0){
                log::instance().error(locs);
                exit(-1);
            }
            spc->nonblock();
            spc->callback = [spc,sps]() mutable{
                log::instance().debug(locs , "answer from socket %d  to client %d\n" , spc->getsocket() , sps->getsocket());
                spc->wantclose();
                sps->vs.swap(spc->vr);
                sps->send();
            };
            spc->vs.swap(sps->vr);
            if(spc->send() > 0){
                log::instance().debug("quick query");
                if(spc->recv() > 0){
                    log::instance().debug("quick answer");
                    sps->vs.swap(spc->vr);
                    sps->send();
                }
                if(spc->iswantread()){
                    log::instance().debug(locs , "add socket %d : %hhx to map after read\n" , spc->getsocket() , spc->ss);
                    if(spc->iswantclose()) spc->clearclose();
                    msp.emplace(spc->getsocket() , spc);
                }
            }else{
                if(spc->iswantwrite()){
                    log::instance().debug(locs , "add socket %d : %hhx to map after send\n" , spc->getsocket() , spc->ss);
                    spc->wantread();
                    msp.emplace(spc->getsocket() , spc);
                }
                else{
                    spc->wantclose();
                    sps->wantclose();
                }
            }
        };
        
        if(!sps->iswantclose()) msp.emplace(sps->getsocket() , sps);
        //mss.emplace(sps->getsocket() , spc->getsocket());
        log::instance().debug(locs , "add socket %d to map\n" , sps->getsocket());
        ev.addreadfd(sps->getsocket());
        for(auto itr = msp.begin() ; itr != msp.end();){
            auto& sp = itr->second;
            if(sp->iswantclose()){
                log::instance().debug(locs , "erase socket %d from map\n" , sp->getsocket());
                itr = msp.erase(itr);
                //mss.erase(sp->getsocket());
                continue;
            }else if(sp->iswantread()){
                log::instance().debug(locs , "add socket %d to read fdset\n" , sp->getsocket());
                ev.addreadfd(sp->getsocket());
            }else if(sp->iswantwrite()){
                log::instance().debug(locs , "add socket %d to write fdset\n" , sp->getsocket());
                ev.addwritefd(sp->getsocket());
            }else{
                log::instance().debug(locs , "socket %d except %hhx\n" , sp->getsocket() , sp->ss);
            }
            itr++;
        }
        auto r = ev.run(nullptr);
        if(r < 0){
            log::instance().error(locs);
            return -1;
        }else if(r == 0){
            log::instance().debug("select timeout\n");
            continue;
        }else{
            log::instance().debug(locs , "select %d events\n" , r);
            /*if(ev.isread(sps->getsocket())){
                r--;
                sps->recv();
                if(sps->callback != nullptr){
                    sps->callback();
                    sps->callback = nullptr;
                }
            }else*/{
                while(r > 0){
                    for(auto& [i,p] : msp){
                        if(ev.isread(i)){
                            log::instance().debug(locs , "socket %d read\n" , p->getsocket());
                            r--;
                            auto rr = p->recv();
                            if(rr > 0 && p->callback != nullptr){
                                p->callback();
                                p->callback = nullptr;
                            }
                        }
                        if(ev.iswrite(i)){
                            log::instance().debug(locs , "socket %d send\n" , p->getsocket());
                            r--;
                            p->send();
                        }
                        if(ev.isexcept(i)){
                            log::instance().debug(locs , "socket %d except\n" , p->getsocket());
                            r--;
                            p->wantclose();
                        }
                    }
                }
            }
        }
    }
    return 0;
}