#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <map>

class TAbstract;

#define COUNT_FIELD             10                                          // Колличества функций
#define SIZE_PARAMETERS_INT     sizeof (int64_t)
#define SIZE_CODE_MESSAGE       sizeof (int16_t)
#define SIZE_NUM_FUNC_API       sizeof (int16_t)
#define SIZE_NARGS              sizeof (int16_t)
#define SIZE_PDATA              sizeof(char**)
#define SIZE_BLOCK_MESSAGE      SIZE_PARAMETERS_INT+SIZE_PDATA
#define SIZE_API_TABLE          SIZE_BLOCK_MESSAGE*COUNT_FIELD
#define MESSAGE_FILLED          0x01
#define MESSAGE_PREPARED        0x02
#define MESSAGE_COMPLATE        0x03

#define CODE_MESSAGE_EMPTY      0x5555
#define CODE_MESSAGE_PREP       0x5F55
#define CODE_MESSAGE_NEW        0x5F5F

#define GET_N_ARGS_FROM_MEM(a) \
                b =  0xFF&*(a+3+SIZE_FUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+2+SIZE_FUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+1+SIZE_FUNC_API);     b <<= 8; \
                b |= 0xFF&*(a+SIZE_FUNC_API); \
                return b;


#define GET_P_FUNK_FROM_MEM(a) \
                b = 0xFF&*(a+3);     b <<= 8; \
                b |= 0xFF&*(a+2);     b <<= 8; \
                b |= 0xFF&*(a+1);     b <<= 8; \
                b |= 0xFF&*a; \
                return b;

#define GET_P_DADA_FROM_MEM(a, b) \
                b =  0xFF&*(a+7+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+6+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+5+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+4+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+3+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+2+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+1+SIZE_FUNC_API+sizeof(int));     b <<= 8; \
                b |= 0xFF&*(a+SIZE_FUNC_API+sizeof(int));


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

class scoped_guard
{
    std::thread t;
public:
    explicit scoped_guard(std::thread t_) : t(std::move(t_)) {
        if(!t.joinable())
            throw std::logic_error("No thread");
    }
    ~scoped_guard()
    {
        t.join();
        std::cout << "end thread: " << std::hex << std::this_thread::get_id() << std::endl;
    }
    scoped_guard(scoped_guard const&) = delete;
    scoped_guard& operator=(scoped_guard const&) = delete;
};


struct InfoThread {
    std::condition_variable condVar;
    std::mutex mtx;
    scoped_guard *scGuard;
    size_t indxMessage;
};

struct DataMessages {
    std::vector<std::vector<char > > *data;
    size_t getIndexBlockReady();
    long getIndexBlockFilled();
private:
    std::mutex mtx;
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
//    std::vector<void(*)(int, char**) > api;
    std::vector<std::vector<char > > *data;
    size_t indexWrite;
    void (TAbstract::*api[COUNT_FIELD])(int16_t&, char**);

    std::mutex getMtx;

public:
    std::string m_sName;
    std::map<size_t, InfoThread* > queueThreads;
    TAbstract(std::vector<std::vector<char > > *data_) : data(data_) {
        indexWrite = 0;
        std::cout << "SIZE_PFUNC_API: " << SIZE_NUM_FUNC_API << std::endl;
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
    int createRequest(InfoThread *infoThr, int16_t aCountArgs, char **appchData);
    size_t getIndexBlockReady();
    long getIndexBlockFilled();

    void call(int16_t &, char**);

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
