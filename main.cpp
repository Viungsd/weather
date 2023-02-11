//
//  main.cpp
//  C++
//
//  Created by Mahoone on 2022/3/28.
//

#include <iostream>
#include <string>
#include <vector>
#include <thread>
using namespace std;

int pro(int a, int b){
    for (int i=0; i<a; i++) {
        cout<<"pro---i:"<<i << " b:" <<b <<endl;
    }
    return a+b;
}

class A{
  mutable  int i;
public:
  
    A(){
       // cout<<"A() " << hex << this <<endl;
    }
    A(const A&a){
       // cout<<"A(const A&a) from " << hex << &a <<" to:" << hex <<this <<endl;
    }
    A(A&& a){
       // cout<<"A(A&& a) from " << hex << &a <<" to:" << hex <<this <<endl;
    }
    
    ~A(){
       // cout<<"~A " << hex << this <<endl;
    }
    
    void  operator()(int a,char c){
        
    }
    virtual void print(){
        cout<<"A print "  <<endl;
    }
};

class scoped_thread{
   A t;
public:
    scoped_thread(const A& t_):t((A&&)(t_))
    {
    }

    int operator()(int a){
        return a + 10;;
    }
    ~scoped_thread(){

    }
};


A xxxtest(){
    return  A();
}


class B: public A{
public:
    
    B(){
        
    }
    
    void print() override{
        cout<<"B print "  <<endl;
    }
    
    ~B(){
        
    }
};

template<typename T>
struct func;

///只声明，不实现
template <typename >
struct is_mem_func;

///non const 类函数指针
template <typename RET,typename CLS,typename ...ARC >
struct is_mem_func<RET (CLS::*)(ARC...)> {
    using type_func = RET (ARC...);;
    
    RET operator()(ARC...){
        
    }
};

///const 类函数指针
template <typename RET,typename CLS,typename ...ARC >
struct is_mem_func<RET (CLS::*)(ARC...) const> {
    using type_func = RET (ARC...);;
    
    RET operator()(ARC...){
        
    }
};

///普通函数指针
template <typename RET,typename ...ARC >
struct is_mem_func<RET(ARC...)> {
    using type_func = RET (ARC...);;
    
    RET operator()(ARC... arc){
        using base_type = func<RET(ARC...)>;
        auto p = static_cast<base_type*>(this)->get_ptr();
        auto b = *(type_func**)p;
        return b(arc...);
    }
};


template<typename T>
struct func:is_mem_func<T>{
    using super_type = is_mem_func<T>;
    using type_func = typename super_type::type_func;
    static constexpr  auto  capacity = 10;
    func(){
        data._[capacity-1]= nullptr;
    }
    
    template<typename C>
    func(C arg){
        if (sizeof(C) <= sizeof(data.content)) {
            new(data.content) C(std::move(arg));
            data._[capacity-1] = (type_func*)data.content;
        }
    }
    
    bool is_local(){
        return true;
    }
    
    type_func* get_ptr(){
        return data._[capacity-1];
    }
private:
    union{
        type_func* content [(capacity-1)];
        type_func* _ [capacity];
    }data;
};


template<typename T>
func(T) -> func<typename is_mem_func<decltype(&T::operator())>::type_func>;

template<typename RET,typename ...ARG>
func(RET(ARG...)) ->func<RET(ARG...)>;


int main()
{
   // auto address = (long)pro;
    
    func yuuuuu = pro;
    int uuu = yuuuuu(1,5);
    
    
 
    return 0;

    
    func<void(int)> yyyaa;
    func<void(int,bool)> yyyaa000;
    
    yyyaa = yyyaa000;///why? different type
    
    func o0000 = [](int , int){
        return 0;
    };
    
    yuuuuu = o0000;
    o0000 = yuuuuu;

    A abc = A{};
    func uuu90 = abc;
//    abc.print();
//    abc.ttt();
    
    func lamddd = [](int,double,bool){};

   // lamddd = uuu90;

//    uu
   
//    decltype(uuu90);;
    uuu90(5,9);
    
    auto ppp = &A::operator();
    (abc.*ppp)(00,9);
    

   // is_mem_func<decltype(ppp)>::type_func;
    
    auto lamada = [](int a){ return a + 10;};
    

    auto pointerOpt = &decltype(lamada)::operator();
    
   // is_mem_func<decltype(pointerOpt)>::type_func;
    
   int u888 = (&lamada->*pointerOpt)(90);
    
   // &lamada->(*pointerOpt)(00);
    
  //  is_mem_func<int(B::*)(int)>::type_func;
//    is_mem_func<decltype(pointerOpt)>::ttt apppp;
    
   // uuu90 = lamada;
    
    
    
    std::function test =  [](int a,int b){ return a+b;};

    
    test = pro;
    
    
    
    is_mem_func<decltype(&B::print)>::type_func x00x;
    
//    auto pp = &B::~B;
    
    std::function<void(A*)> kk = &A::print;
    //is_member_function_pointer<int*>;
    int tttt;
    auto xx = [](A*)-> void{
       //int i = tttt + 1;
    };
    
    void (*pFun)(A *) = xx;
    
    
    A a;
    B b;
    
    kk(&b);
  
    
    return 0;
}
