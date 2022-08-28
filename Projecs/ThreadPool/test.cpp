#include "threadpool.hpp"
#include <iostream>

void func_th0(int& sum)
{
    for (int i = 0; i < 100; i++)
    {
        ++sum;
        printf("func0 : %d\n", sum);
    }
}

void func_th1(int& sum)
{
    for (int i = 0; i < 100; i++)
    {
        ++sum;
        printf("func1 : %d\n", sum);
    }
}

int main()
{
    int sum0 = 0;
    int sum1 = 0;

    MoThreadUtils::ThreadPool m_thread_pool(4);

    m_thread_pool.AddTask([&sum0]()
    {
        func_th0(sum0);
    });
    m_thread_pool.AddTask([&sum0]()
    {
        func_th1(sum0);
    });

    while(true)
    {
        if (sum0 >= 200)
        {
            break;
        }
    }
    printf("sum0 : %d\n", sum0);
    printf("sum1 : %d\n", sum1);
    return 0;
}

