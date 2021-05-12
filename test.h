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
#include <atomic>


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

#define CODE_MESSAGE_EMPTY      0x0000
#define CODE_MESSAGE_PREP       0x5F55
//#define CODE_MESSAGE_PROC       0x5555
#define CODE_MESSAGE_NEW        0x5F5F
#define CODE_MESSAGE_READY      0x55FF
#define CODE_MESSAGE_COMPL      0x0055



#define MESSAGE_INIT            0x0001



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


int test();

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
    std::thread::id getId() {
        return t.get_id();
    }
    explicit scoped_guard(std::thread t_) : t(std::move(t_)) {
        if(!t.joinable())
            throw std::logic_error("No thread");
    }
    ~scoped_guard()
    {
//        std::cout << "end thread: " << std::hex << std::this_thread::get_id() << std::endl;
        t.join();
    }
    scoped_guard(scoped_guard const&) = delete;
    scoped_guard& operator=(scoped_guard const&) = delete;
};

struct ThreadInfo2 {
    bool cycle = false;
    std::thread thread;
};

struct InfoThread {
//    std::thread t;

public:
//    explicit InfoThread(size_t size_, std::thread t_) : thread(std::move(t_)) {
//        indxMessage = size_;
//        if(!thread.joinable())
//            throw std::logic_error("No thread");
//    }
    ~InfoThread() {
        delete threadGuard;
    }
    std::condition_variable condVar;
    std::mutex mtx;
    scoped_guard *threadGuard;
    int16_t numRequest;
    bool cycle = true;
};

struct DataMessages {
    DataMessages() {data = new std::vector<std::vector<char >>(0, std::vector<char>(SIZE_BLOCK_MESSAGE));}
    ~DataMessages() {
        std::cout << "vector (data) before clear" << std::endl;
        try {
            data->clear();

        } catch(const std::runtime_error& re) {
            std::cerr << "call0: Runtime error: " << re.what() << std::endl;
        } catch(const std::exception& ex) {
            std::cerr << "call0: Error occurred: " << ex.what() << std::endl;
        } catch(...) {
            std::cerr << "~DataMessages: Unknown failure occurred. Possible memory corruption" << std::endl;
        }
        std::cout << "vector (data) is clear" << std::endl;
        delete data;
//                    std::cout << "vector deleted" << std::endl;
    }
    std::vector<std::vector<char > > *data;
    size_t getIndexBlockReady();
    long getIndexBlockFilled();
    bool messagesIsFilled();
    bool checkRequest(int16_t numReq, int16_t check_code);
//    int16_t getCodeQueue();
    bool getMessageIsEmpty(int16_t &code, int16_t &req);
    bool getMessageReady(int16_t &code, int16_t &req);
    void getIndexOfReqNum(int16_t req, int16_t &index);
    bool setCodeOfReqNum(int16_t req, int16_t code);

    void addMessage(char *ptrData);
    bool readMessage(char *&ptrData);
    bool changePointer(int16_t numReq, char **aPointer);
    bool changeData(char *ptrData, int16_t req, int16_t code_);
    bool changeApiFunc(int16_t numReq, int16_t numApi);
    bool changeCode(int16_t numReq, int16_t change_code);
    std::mutex mtx;

public:
    bool clearMessage(int16_t req);
private:
};

struct InfoOfWork {
    long dTime;
    int16_t req;
};

class TAbstract
{
public:
    DataMessages *messages;
    bool cycle = true;
    std::string m_sName;
    TAbstract(DataMessages *data_) : messages(data_) {
    }
    virtual int writeMessage() = 0;
    virtual int readMessage() = 0;
    virtual int worker() = 0;
    virtual ~TAbstract();

//    size_t getIndexBlockReady();
//    long getIndexBlockFilled();


    TAbstract(const TAbstract&) = delete;
    TAbstract& operator=(const TAbstract&) = delete;

    std::mutex mtx;
};

class TServer : public TAbstract
{
    void (TServer::*api[COUNT_FIELD])(int16_t&, char**);
    std::thread t;
    std::vector<ThreadInfo2*> threads;
//    std::vector<std::string*> strings;
    std::condition_variable condVar;
    bool next = false;
    size_t thread_count;
public:
    int openSession(ThreadInfo2 *infoThr);
    void call0(int16_t &, char**);
    void call1(int16_t &, char**);
    void call2(int16_t &, char**);
    void call3(int16_t &, char**);

    TServer(DataMessages *messages_);

    int worker() override;
    int writeMessage() override;
    int readMessage() override;
    ~TServer() override;
    TServer(TServer const&) = delete;
    TServer& operator=(TServer const&) = delete;
};

class TClient : public TAbstract
{
    enum STATE_CLIENT {STATE_START, STATE_WORK, STATE_COMPLATE_LAUNCH,
                      STATE_COMPLATE_PROC, STATE_COMPLATE_FINISH} state = STATE_START;
    enum STATE_REQUEST {STATE_REQ_INIT, STATE_REQ_WORK, STATE_REQ_PRINT, STATE_REQ_COMPLATE };

    std::mutex mtxDataOfWork;

    std::map<int16_t, InfoOfWork* > infoOfWork;
    std::map<int16_t, InfoThread* > queueThreads;
    std::thread t;
    bool complate = false;
    int createRequest(InfoThread *infoThr, int16_t aCountArgs, char **appchData);
    int createRequestStopServer(InfoThread *infoThr, int16_t aCountArgs, char **appchData);
    void setSnapShotTime(int16_t numReq, InfoOfWork *iow);
public:
    TClient(DataMessages *messages_);
    int worker() override;
    int writeMessage() override;
    int readMessage() override;
    ~TClient() override;
    TClient(TClient const&) = delete;
    TClient& operator=(TClient const&) = delete;
};








//template<typename T>
//class threadsafe_queue
//{
//private:
//    struct node
//    {
//        std::shared_ptr<T> data;
//        std::unique_ptr<node> next;
//    };
//    std::mutex head_mutex;
//    std::unique_ptr<node> head;
//    std::mutex tail_mutex;
//    node* tail;
//    node* get_tail()
//    {
//        std::lock_guard<std::mutex> tail_lock(tail_mutex);
//        return tail;
//    }
//    std::unique_ptr<node> pop_head()
//    {
//        std::lock_guard<std::mutex> head_lock(head_mutex);
//        if(head.get()==get_tail())
//        {
//            return nullptr;
//        }
//        std::unique_ptr<node> old_head=std::move(head);
//        head=std::move(old_head->next);
//        return old_head;
//    }
//public:
//    threadsafe_queue():
//        head(new node),tail(head.get())
//    {}
//    threadsafe_queue(const threadsafe_queue& other)=delete;
//    threadsafe_queue& operator=(const threadsafe_queue& other)=delete;
//    void try_pop(std::shared_ptr<T> &t)
//    {
//        std::unique_ptr<node> old_head=pop_head();
//        t = old_head?old_head->data:std::shared_ptr<T>();

//    }
//    void push(T new_value)
//    {
//        std::shared_ptr<T> new_data(
//                    std::make_shared<T>(std::move(new_value)));
//        std::unique_ptr<node> p(new node);
//        node* const new_tail=p.get();
//        std::lock_guard<std::mutex> tail_lock(tail_mutex);
//        tail->data=new_data;
//        tail->next=std::move(p);
//        tail=new_tail;
//    }
//};

//class join_threads
//{
//    std::vector<std::thread>& threads;
//public:
//    explicit join_threads(std::vector<std::thread>& threads_):
//        threads(threads_)
//    {}
//    ~join_threads()
//    {
//        for(unsigned long i=0;i<threads.size();++i)
//        {
//            if(threads[i].joinable())
//                threads[i].join();
//        }
//    }
//};

//class thread_pool
//{
//    std::atomic_bool done;
//    threadsafe_queue<std::function<void()> > work_queue;
//    std::vector<std::thread> threads;
//    join_threads joiner;
//    void worker_thread()
//    {
//        while(!done)
//        {
//            std::function<void()> task;
//            std::shared_ptr<std::function<void()>> ptrTask;
//            if(work_queue.try_pop(ptrTask))
//            {
//                task();
//            }
//            else
//            {
//                std::this_thread::yield();
//            }
//        }
//    }
//public:
//    thread_pool():
//        done(false),joiner(threads)
//    {
//        unsigned const thread_count=std::thread::hardware_concurrency();
//        try
//        {
//            for(unsigned i=0;i<thread_count;++i)
//            {
//                threads.push_back(
//                            std::thread(&thread_pool::worker_thread,this));
//            }
//        }
//        catch(...)
//        {
//            done=true;
//            throw;
//        }
//    }
//    ~thread_pool()
//    {
//        done=true;
//    }
//    template<typename FunctionType>
//    void submit(FunctionType f)
//    {
//        work_queue.push(std::function<void()>(f));
//    }
//};


#endif // TEST_H
