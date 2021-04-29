#include "test.h"
#include <iterator>
#include <string.h>
#include <iomanip>
#include <algorithm>

TAbstract::~TAbstract() {

}

int TAbstract::openSession(size_t anNumMessage)
{

    void (*pFunc)(int, char**);

    std::cout << std::endl;
    std::cout << "Server " << std::endl;
    for(auto i: data->at(anNumMessage))
        std::cout << (0xFF&i) << " ";
    std::cout << std::endl;

    long pfunc = 0;
    int nArgs = 0;
    long addressData = 0;
    GET_P_FUNK_FROM_MEM(data->at(anNumMessage).data(), pfunc)
    GET_N_ARGS_FROM_MEM(data->at(anNumMessage).data(), nArgs)
    GET_P_DADA_FROM_MEM(data->at(anNumMessage).data(), addressData)
    char **pTmp = reinterpret_cast<char**>(addressData);
    pFunc = reinterpret_cast<void (*)(int, char**)>(pfunc);

    nArgs &= 0xFFFF;
    pFunc(nArgs, pTmp);
    return 0;
}

void TAbstract::call(int nArgs, char **ppchArgs){
    std::cout << "nArgs =" << nArgs << std::endl;
    std::cout << "ppchArgs =" << std::hex << ppchArgs << std::endl;

    std::cout << "\t ppchArgs: " << std::hex << ppchArgs  << std::endl;
    std::cout << "\t arg" << 1 << " = " << std::hex << *(int*)ppchArgs[0] << std::endl;
    std::cout << "\t arg" << 2 << " = " << std::hex << *(size_t*)ppchArgs[1] << std::endl;

    for(int i = 2; i < nArgs; i++) {
        std::cout << "\t arg" << i << " = " << std::hex << *ppchArgs[i] << std::endl;
    }
}

int TAbstract::createSession(size_t anNumReq)
{
    std::lock_guard<std::mutex> lock(mtx);
    const int nArgs = 1 | 0x010000;
    const size_t indexData = getIndexBlockReady();
    memcpy(data->at(indexData).data(), &api[0], SIZE_PFUNC_API);                    // функция
    memcpy(data->at(indexData).data() + SIZE_PFUNC_API, &nArgs, SIZE_NARGS);        // количество параметров
    char **ppchTmp = new char*[static_cast<size_t>(nArgs)] {
            reinterpret_cast<char*>(new size_t(anNumReq))
    };
    memcpy(data->at(indexData).data() + SIZE_PFUNC_API + SIZE_NARGS, &ppchTmp, sizeof ppchTmp);

    std::cout << std::endl << "Client" << std::hex << std::endl;
    for(auto i: data->at(indexData))
        std::cout << (0xFF&i) << " ";
    std::cout << std::endl;

    return 0;
}

size_t TAbstract::getIndexBlockReady()
{
    for(size_t i = 0; i < data->size(); i++) {
        if(data->at(i)[SIZE_PFUNC_API+2] == 0x00) {
            return i;
        }
    }
    data->push_back(std::vector<char>(SIZE_BLOCK_MESSAGE));
    return data->size()-1;
}

long TAbstract::getIndexBlockFilled()
{
    for(size_t i = 0; i < data->size(); i++) {
        if(data->at(i)[SIZE_PFUNC_API+2] == 0x01) {
            return static_cast<long>(i);
        }
    }
    return -1;
}


//union SizetToBytesUnion {
//  size_t value;
//  unsigned char bytes[sizeof(value)];
//} u;
//size_t value = 0xFEFEFEF;
////SizetToBytesUnion u;
//u.value = value;
//size_t zeroBitsN = 0;
//for (size_t i = 0; i != sizeof(u.bytes); ++i)
//  zeroBitsN |= u.bytes[i];


TServer::TServer(std::vector<std::vector<char > > *data_) : TAbstract(data_) {
}

int TServer::writeMessage() {
    std::cout << "Server write" << std::endl;
    return 0;
}

int TServer::readMessage() {
    return 0;
}

TServer::~TServer() {
    std::cout << "~Sever()" << std::endl;
}

int TServer::worker()
{
    long tmp_counter = 0;
    while(true) {
        const long indexData = getIndexBlockFilled();
        if(indexData < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }


        thread_guard thr_req(std::thread(&TServer::openSession, this, indexData));


        if(tmp_counter%10 == 10)
            std::cout << this << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(tmp_counter >= 100)
            break;
        tmp_counter++;
    }
    std::cout << "worker end" << std::endl;
    return 0;
}


/************************************************
 *                  Client
 * **********************************************/
TClient::TClient(std::vector<std::vector<char > > *data_) : TAbstract(data_) {
}


int TClient::worker()
{
    long tmp_counter = 0;
    while(true) {


        thread_guard thr_req(std::thread(&TClient::createSession, this, tmp_counter));


        if(tmp_counter%10 == 10)
            std::cout << this << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(tmp_counter >= 100)
            break;
        tmp_counter++;
    }
    std::cout << "worker end" << std::endl;
    return 0;
}

int TClient::writeMessage() {


    std::cout << "Client write" << std::endl;
    return 0;
}

int TClient::readMessage() {
    return 0;
}

TClient::~TClient() {
    std::cout << "~Client()" << std::endl;
}
static int i = 0;

void test2() {
    i++;
    if(i > 1000000000)
        return;
    test2();
}

class A {
    static int in;
public:
    A(int i = 0){in++; std::cout << "construct A" << std::endl;}
    ~A(){std::cout << "destruct A" << std::endl;}
    A(const A&a){std::cout << "copy construct A" << std::endl;}
    A& operator=(const A&a){std::cout << "assignment A" << std::endl; return *this;}
    void print() const {std::cout << this << " print "  << std::endl;}
};
int A::in = 0;
void test3(const A &a) {
    a.print();
    std::cout << &a << std::endl;
}

void test() {


    std::vector<std::vector<char >> vec(0, std::vector<char>(SIZE_BLOCK_MESSAGE));

//    stack[0] = 1;
//    std::cout << stack[0] << std::endl;
//    std::cout << std::endl;

    TServer server(&vec);
    TClient client(&vec);

//    long hardware_threads = std::thread::hardware_concurrency();
//    std::cout << hardware_threads << std::endl;

    thread_guard thrClient(std::thread(&TClient::worker, &client));
    thread_guard thrServer(std::thread(&TServer::worker, &server));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    hardware_threads = std::thread::hardware_concurrency();
//    std::cout << hardware_threads << std::endl;
    //thrClient.join();
    //delete client;

    std::cout << "do something in current thread" << std::endl;

}
