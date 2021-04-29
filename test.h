#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <pthread.h>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <vector>
#include <mutex>

class TAbstract;

#define COUNT_FIELD              10                                          // Колличества функций
#define SIZE_PFUNC_API          sizeof (void (TAbstract::*)(int, char**))
#define SIZE_NARGS              sizeof (int)
#define SIZE_BLOCK_MESSAGE      SIZE_PFUNC_API+sizeof(int)+sizeof(char**)
#define SIZE_API_TABLE          SIZE_PFUNC_API*COUNT_FIELD

#define GET_N_ARGS_FROM_MEM(a, b) \
                b =  0xFF&*(a+3+SIZE_PFUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+2+SIZE_PFUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+1+SIZE_PFUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+SIZE_PFUNC_API);


#define GET_P_FUNK_FROM_MEM(a, b) \
                b =  0xFF&*(a+7);     b <<= 8; \
                b |= 0xFF&*(a+6);     b <<= 8; \
                b |= 0xFF&*(a+5);     b <<= 8; \
                b |= 0xFF&*(a+4);     b <<= 8; \
                b |= 0xFF&*(a+3);     b <<= 8; \
                b |= 0xFF&*(a+2);     b <<= 8; \
                b |= 0xFF&*(a+1);     b <<= 8; \
                b |= 0xFF&*a;

#define GET_P_DADA_FROM_MEM(a, b) \
                b =  0xFF&*(a+7+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+6+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+5+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+4+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+3+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+2+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+1+SIZE_PFUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+SIZE_PFUNC_API+sizeof(int));


void test();

struct func
{
    int &i;
    func(int& i_) : i(i_){}
    void operator()()
    {
        std::cout << i << std::endl;
        for(unsigned j=0;j<100000;++j)
        {
            i |= 0xFFFFFFFF;

        }
        std::cout << i << std::endl;
    }
};

class thread_guard
{
    std::thread t;
public:
    explicit thread_guard(std::thread t_) : t(std::move(t_)) {
        if(!t.joinable())
            throw std::logic_error("No thread");
    }
    ~thread_guard()
    {
        t.join();
    }
    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};



//class CustomQueue {
//    std::vector<char*> m_tableMessage;
//public:
//    CustomQueue() {

//    }

//    size_t size() { return m_tableMessage.size(); }

//};

class TAbstract
{
    //std::vector<void(*)(int, char**) > api;
    std::vector<std::vector<char > > *data;
    size_t indexWrite;
    void (TAbstract::*api[COUNT_FIELD])(int, char**);

public:
    TAbstract(std::vector<std::vector<char > > *data_) : data(data_) {
        indexWrite = 0;
        std::cout << "SIZE_PFUNC_API: " << SIZE_PFUNC_API << std::endl;
        size_t sz = 1000;
        if(static_cast<unsigned int>(sz) > SIZE_API_TABLE) {
            for(size_t i = 0; i < COUNT_FIELD; i++)
                api[i] = &TAbstract::call;
        } else {
            std::cout << "error init block API" << std::endl;
        }
    }
    virtual int writeMessage() = 0;
    virtual int readMessage() = 0;
    virtual int worker() = 0;
    virtual ~TAbstract();
    int openSession(size_t anNumMessage);
    int createSession(size_t anNumReq = 0);
    size_t getIndexBlockReady();
    long getIndexBlockFilled();

    void call(int, char**);

    TAbstract(const TAbstract&) = delete;
    TAbstract& operator=(const TAbstract&) = delete;

    std::mutex mtx;
};

class TServer : public TAbstract
{
public:
    TServer(std::vector<std::vector<char > > *data_);
    int worker() override;
    int writeMessage() override;
    int readMessage() override;
    ~TServer() override;
};

class TClient : public TAbstract
{
public:
    TClient(std::vector<std::vector<char > > *data_);
    int worker() override;
    int writeMessage() override;
    int readMessage() override;
    ~TClient() override;
};


#endif // TEST_H
