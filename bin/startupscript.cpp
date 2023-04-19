#include <stdio.h>
#include <cstdlib>

#include <unordered_map>
#include <mutex>

struct RefCountedMemory {
    int refCount;
    void *memory;
};

std::unordered_map<void *, RefCountedMemory *> memoryMap;
std::mutex memoryMapMutex;

extern "C" void* myMalloc(int size)
{
    RefCountedMemory *rcMem = (RefCountedMemory *)malloc(sizeof(RefCountedMemory));
    rcMem->refCount = 1;
    rcMem->memory = malloc(size);

    std::unique_lock<std::mutex> lock(memoryMapMutex);
    memoryMap[rcMem->memory] = rcMem;
    lock.unlock();

    return rcMem->memory;
}

extern "C" void incrementRefCount(void *ptr)
{
    std::unique_lock<std::mutex> lock(memoryMapMutex);
    RefCountedMemory *rcMem = memoryMap[ptr];
    rcMem->refCount++;
    lock.unlock();
}

extern "C" void myFree(void* ptr)
{
   free(ptr);
}

extern "C" void decrementRefCount(void *ptr)
{
    std::unique_lock<std::mutex> lock(memoryMapMutex);
    RefCountedMemory *rcMem = memoryMap[ptr];
    rcMem->refCount--;
    if (rcMem->refCount == 0) {
        myFree(rcMem->memory);
        memoryMap.erase(ptr);
        delete rcMem;
    }

    lock.unlock();
}

extern "C"
{
    void start();
}

extern "C" void printchar(int ascii)
{
    putchar((char)ascii);
}

extern "C" void printint(int val)
{
    printf("%d\n", val);
}

extern "C" void printbool(bool val)
{
    printf("%d\n", val);
}

extern "C" void nextLine()
{
    putchar('\n');
}

extern "C" void indexOutOfBounds(){
    printf("Array Index Out of Bounds!");
    exit(-1);
}

int main()
{
    start();
    std::unique_lock<std::mutex> lock(memoryMapMutex);
    if (memoryMap.empty()) {
        // printf("All memory resources deallocated\n");
    } else {
        printf("Memory resources not fully deallocated\n");
    }
    lock.unlock();
    return 0;
}