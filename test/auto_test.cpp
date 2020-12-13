//#include "simple_gc.h"
#include "auto_alloc.h"

#include <cstdio>

class MyClass
{
public:
    int a;
public:
    MyClass(int t=52):a(t){}
    ~MyClass(){}
    void Print()
    {
        printf("%d\n",a);
    }
};


int main()
{

//    SimpleAlloc alloc;
    AutoAlloc alloc;    
    int count = 5; 

    int* intArray = NewArray<int>(count, alloc);
 
    for(int i=0;i<count;i++)
    {
        intArray[i] = i+1;
    }
    for(int i=0;i<count;i++)
    {
        printf("%d ",intArray[i]);
    }
    printf("\n");

    MyClass* obj = New<MyClass>(alloc);
    
    obj->Print();

    int arg1 = 1024;
    MyClass* objWithArg = New<MyClass>(arg1, alloc);

     objWithArg->Print();

    alloc.Clear();

//    MyClass* objArray = NewArray<MyClass>(count, alloc);

    return 0;
}