#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <mutex>
#include <cstring>
#include <stdlib.h>

#define MAX_SIZE_SHARE_BUF	8			// Размер кольцевого буфера



#define COUNT_FIELD             10                                          // Колличества функций
#define SIZE_PFUNC_API          sizeof (void (*)(int, char**))
#define SIZE_NARGS              sizeof (int)
#define SIZE_API_TABLE          SIZE_PFUNC_API*COUNT_FIELD
#define SIZE_BLOCK_MESSAGE      SIZE_PFUNC_API+sizeof(int)+sizeof(char**)

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


void call(int nArgs, char **ppchArgs) {
    std::cout << "nArgs =" << nArgs << std::endl;
    std::cout << "ppchArgs =" << std::hex << ppchArgs << std::endl;

    std::cout << "\t ppchArgs: " << std::hex << ppchArgs  << std::endl;
    std::cout << "\t arg" << 1 << " = " << std::hex << *(int*)ppchArgs[0] << std::endl;
    std::cout << "\t arg" << 2 << " = " << std::hex << *(size_t*)ppchArgs[1] << std::endl;

    for(int i = 2; i < nArgs; i++) {
        std::cout << "\t arg" << i << " = " << std::hex << *ppchArgs[i] << std::endl;
    }
    return;
}




static class SysInfo {
public:
    SysInfo() {
        iCurCMD = 0;
        srand(static_cast<unsigned>(std::time(nullptr)));
    }
    std::mutex mtx;
    int getMemory(size_t anSize) {
        memory = new char[static_cast<size_t>(anSize)];
        iCurCMD = 0;
        iCurData = anSize;
        if(!memory) {
            return 1;
        } else {
            return 0;
        }
    }
    void readRequest() {
        if(iCurCMD){
            void (*pFunc)(int, char**);

            std::cout << std::endl;
            std::cout << "Server " << std::endl;
            for(size_t i = 0; i < SIZE_BLOCK_MESSAGE; i++) {
                int bTmp = memory[i]&0xFF;
                std::cout << std::hex << bTmp << " "; //
            }
            std::cout << std::endl;

            long pfunc = 0;
            int nArgs = 0;
            long addressData = 0;
            GET_P_FUNK_FROM_MEM(memory, pfunc)
            GET_N_ARGS_FROM_MEM(memory, nArgs)
            GET_P_DADA_FROM_MEM(memory, addressData)
            char **pTmp = reinterpret_cast<char**>(addressData);
            pFunc = reinterpret_cast<void(*)(int, char**)>(pfunc);

            pFunc(nArgs, pTmp);
        }

//        std::cout << "pFunc: " << std::hex << (long long)pchRet << std::endl;
//        std::cout << "p " << std::hex << (long long)ppchTmp << std::endl;
//        std::cout << "addressDataFunk " << std::hex << addressDataFunk << std::endl;
// 0x40a2a6 <call(int, char**)>
    }

    int createSession(size_t anNumReq = 0, int anArgs = 3) {
        mtx.lock();
        if((iCurCMD + sizeBlockMessage) < iCurData && iCurCMD == 0) {
            static size_t marker = 0;
            memcpy(memory, &api[0], SIZE_PFUNC_API);             // функция
            marker += SIZE_PFUNC_API;
            memcpy(memory + marker, &anArgs, SIZE_NARGS);    // количество параметров
            marker += SIZE_NARGS;
            char **ppchTmp = new char*[static_cast<size_t>(anArgs)] {
                    reinterpret_cast<char*>(new int(0x00)),
                    reinterpret_cast<char*>(new size_t(anNumReq)),
                    new char('a')
            };
            memcpy(memory + marker, &ppchTmp, sizeof ppchTmp);
            marker += sizeof ppchTmp;

            std::cout << std::endl << "Client" << std::endl;
            for(size_t i = 0; i < sizeof SIZE_BLOCK_MESSAGE; i++) {
                int bTmp = memory[i]&0xFF;
                std::cout << std::hex << bTmp << ' '; //
            }
            std::cout << std::endl;
            iCurCMD++;
            //memcpy(memory, aCmd, sizeof aCmd);
        }
        //int length = sizePFuncAPI + sizeof(int) +  sizeof(int);
//        std::cout << std::endl;
//        size_t size = sizeof aCmd / sizeof(*aCmd);
//        std::cout << "memory: " << std::dec << size << std::endl;
//        for(size_t i = 0; i < size; i++) {
//            int bTmp = memory[i]&0xFF;
//            std::cout << std::hex << bTmp << " "; //              <-- продолжить, отображение не правильное
//        }

//        std::cout << std::endl;
        mtx.unlock();
        return 0;
    }
    std::atomic<size_t> &getIndexCmd() {

        return iCurCMD;
    }
    void (*api[COUNT_FIELD])(int, char**);
//    char *api = nullptr;
    char *memory = nullptr;

    unsigned int sizePFuncAPI;
    unsigned int sizeAPITable;
    size_t sizeBlockMessage;

    int tmp;
    std::atomic<size_t> iCurCMD;
    std::atomic<size_t> iCurData;
    size_t sizeMemory;


} sysinfo;

#endif // SHARED_MEMORY_H
