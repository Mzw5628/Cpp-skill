#include<iostream>
#include<thread>
#include<chrono>

int add(int a,int b){
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return a + b;
}

int main(){
    std::thread t1(add(5, 10));
    std::thread t2(add(6, 11));
    t1.join();
    t2.join();
    return 0;
}