#include"LogWrite.h"
#include<iostream>
int main(int argc,char **argv)
{
    CWriteLog log("Log2","test");
    log.Log(InfoLog,"hi greate to");
    int i;
    std::cout<<"please input a number"<<std::endl;
    std::cin>>i;
    return 0;
}
