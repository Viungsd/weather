# C++ Tuple元组实现 Metaprogramming

    template<typename ... Tn>struct Tuple;
    
    template<typename T0,typename ... Tn>
    struct Tuple<T0,Tn...>{///特化
        T0 head;
        Tuple<Tn...>tail;///递归调用
        constexpr static int size = sizeof...(Tn)+1;
        Tuple():head(0){
        }
        
        Tuple(const T0 &h,const Tuple<Tn...> &t):head(h),tail(t){
        }
        
        Tuple(const Tuple<T0,Tn...> & a):head(a.head),tail(a.tail){
        }
       
        template<typename _T0,typename ... _Tn,typename =std::enable_if_t<sizeof...(Tn)==sizeof...(_Tn)>>
        Tuple( _T0&& a, _Tn && ...  n):head(std::forward<_T0>(a)),tail(std::forward<_Tn>(n)...){
          
        }
    };
    
    template<typename T>
    struct Tuple<T>{
        constexpr static int size = 1;
        T head;
        
        Tuple():head(0){
        }
        template<typename _T>
        Tuple( _T&& a):head(a){
        }
    };

## 1、类型搜索

从Tuple中搜索某个类型，只要存在一个，则返回true,否则返回false 例如：Tuple<char,short,double> 类型中是否包含 某个类型（short）？只要包含返回true,否则返回false

```

template<typename Patten,typename ...T>struct Search;
    
template<typename Patten,typename H,typename ...T>
struct Search<Patten,Tuple<H,T...>>: std::conditional_t<std::is_same_v<Patten, H>,std::true_type, Search<Patten, Tuple<T...>>> {
};

template<typename Patten>
struct Search<Patten,Tuple<>>: std::false_type {
};

///Test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double> a('s',5,6.8);
    
    bool x0x = Search<short, decltype(a)>::value;/// true
    bool x2x = Search<double, decltype(a)>::value;/// true
    bool x3x = Search<unsigned short, decltype(a)>::value;/// false
    bool x4x = Search<int, decltype(a)>::value;/// false
   
    return 0;
}
```

## 2、类型搜索（并统计个数）

或者，我们可以更近一步，从Tuple中搜索某个类型，并且统计出有多少个该类型？

```
template<typename Patten,typename ...T>struct Count;
    
template<typename Patten,typename H,typename ...T>
struct Count<Patten,Tuple<H,T...>>:std::integral_constant<int, std::is_same_v<Patten, H> + Count<Patten, Tuple<T...>>::value > {
};

template<typename Patten>
struct Count<Patten,Tuple<>>: std::integral_constant<int, 0> {
};

///Test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double,short,int,double,double> a;
    
    int c0 = Count<char, decltype(a)>::value;///1
    int c1 = Count<short, decltype(a)>::value;///2
    int c2 = Count<double, decltype(a)>::value;///3
    int c3 = Count<bool, decltype(a)>::value;///0
    int c4 = Count<unsigned char, decltype(a)>::value;///0
    int c5 = Count<int, decltype(a)>::value;///1
    
    return 0;
}
```

有了上述统计类型个数的方法，判断Tuple中是否包含某个类型还可以修改成如下：

原理：该类型个数大于0返回true,否则返回false，不过效率相对低点，因为不管是否包含都会完全遍历完整个类型列表。

```
///判断是否存在某个类型？
template<typename Patten,typename ...T>
using search_t = std::conditional_t< (Count<Patten, T...>::value > 0) , std::true_type, std::false_type>;

///Test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double,short,double,double> a;
    
    bool x0x = search_t<short, decltype(a)>::value;/// true
    bool x2x = search_t<double, decltype(a)>::value;/// true
    bool x3x = search_t<unsigned short, decltype(a)>::value;/// false
    bool x4x = search_t<int, decltype(a)>::value;/// false
    
    return 0;
}
```

## 3、类型搜索（返回首次匹配到该类型时的位置）

或者，我们可以更更再近一步，从Tuple中搜索某个类型，并返回第一次匹配到该类型时的位置（从1开始），如果没有匹配到返回0

```
template<typename Patten,typename ...T>struct SearchHIdx;
  
template<typename Patten,typename H,typename ...T>
struct SearchHIdx<Patten,Tuple<H,T...>>: std::integral_constant<int,(std::is_same_v<Patten, H> ? sizeof...(T) :  SearchHIdx<Patten, Tuple<T...>>::value )>{
};

template<typename Patten>
struct SearchHIdx<Patten,Tuple<>>: std::integral_constant<int, 0> {
};

template<typename Patten,typename ...T>struct SearchH;

template<typename Patten,typename ...T>
struct SearchH<Patten,Tuple<T...>> :std::integral_constant<int,((SearchHIdx<Patten, Tuple<T...>>::value == 0) ? 0 : (sizeof...(T) - SearchHIdx<Patten, Tuple<T...>>::value))>{
};

///Test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double,short,double,double> a;///6
    
    int idx0 = SearchH<char, decltype(a)>::value;///1
    int idx1 = SearchH<short, decltype(a)>::value;///2
    int idx2 = SearchH<double, decltype(a)>::value;///3
    int idx3 = SearchH<int, decltype(a)>::value;///0， Not found

    return 0;
}
```

## 4、萃取Tuple指定位置的类型

我们可以从Tuple中萃取指定位置的类型，例如第一个、最后一个、或者第n个...

可以例如模版函数声明，代码如下：

```
template<typename Head,typename ...Trail>
Head ForntT(Tuple<Head,Trail...>);///获取第一个类型

///template<typename ...Head,typename Trail>///获取最后一个类型
///Trail BackT(Tuple<Head...,Trail>);///【无法使用该模板】：因为Tuple<Head...,Trail>类型作为函数参数编译器推断不出来

template<typename Head,typename ...Trail>
Tuple<Trail...> PopForntT(Tuple<Head,Trail...>);///移除第一个类型

template<typename Head,typename ...Trail>
Tuple<Head,Trail...> PushForntT(Head ,Tuple<Trail...>);///插入类型到Tuple的首部

template<typename ...Head,typename Trail>
Tuple<Head...,Trail> PushBackT(Tuple<Head...>,Trail);///插入类型到Tuple的尾部

///Test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double> a;
   
    decltype(ForntT(a)) x1;/// char x1;
    decltype(PopForntT(a)) x2;///   Tuple<short,double> x2;
    decltype(PushBackT(a,false)) x3;///  Tuple<char,short,double,bool> x3;
    
    return 0;
}
```

通过上述函数模板的声明，可以利用编译器推断出来Tuple的部分位置的情况，但是上述方法无法推断出Tuple的最后一个类型，或者移除最后一个类型，我们得想其他办法。我们尝试用类模板偏特化试试看：

```
template<typename T>struct FrontC;

template<typename Head,typename ...Trail>
struct FrontC<Tuple<Head,Trail...>>{///获取第一个类型
    using type = Head;
};

//template<typename T>struct BackC;
//
//template<typename ...Head,typename Trail>
//struct BackC<Tuple<Head...,Trail>>{///获取最后一个类型，无法编译通过
//    using type = Trail;
//};


template<typename T>struct PopFrontC;

template<typename Head,typename ...Trail>
struct PopFrontC<Tuple<Head,Trail...>>{///移除第一个类型
    using type = Tuple<Trail...>;
};

template<typename Head,typename ...Trail>struct PushFrontC;

template<typename Head,typename ...Trail>
struct PushFrontC<Head,Tuple<Trail...>>{///插入类型到Tuple首部
    using type = Tuple<Head,Trail...>;
};


template<typename Trail,typename ...Head>struct PushBackC;

template<typename Trail,typename ...Head>
struct PushBackC<Trail,Tuple<Head...>>{///插入类型到Tuple尾部
    using type = Tuple<Head...,Trail>;
};

int main(int argc, const char * argv[]) {
    Tuple<char,short,double> a;
   
    FrontC<decltype(a)>::type x1;/// char x1;
    PopFrontC<decltype(a)>::type x2;/// Tuple<short,double> x2;
    PushFrontC<bool, decltype(a)>::type x3;/// Tuple<bool,char,short,double> x3;
    PushBackC<int, decltype(a)>::type x4;/// Tuple<char,short,double,int> x4;
    
    return 0;
}
```

发现类模板也无法萃取出Tuple的最后一个类型，或者移除最后一个类型。因为Tuple<Head...,Trail>这样的类型，无法被偏特化。

### 萃取Tuple的指定位置的类型

本来准备写一个可以萃取Tuple任意位置类型的方法，结果失败了，阴错阳差，这个“错误”刚好而且“只能”能把Tuple的最后一个类型萃取出来。

```
template <int idx, typename ...T>
struct Supscript;

template <int idx,typename H, typename ...T>
struct Supscript<idx,Tuple<H,T...>>{
private:
    using last = typename Supscript<idx-1,Tuple<T...>>::type;
public:
    using type = std::conditional_t<idx==0, H,last>;
};

template <typename T>
struct Supscript<0,Tuple<T>>{
    using type = T;
};

template<typename ...T>struct LastType;

template<typename ...T>
struct LastType<Tuple<T...>>{
    using type = typename Supscript<sizeof...(T)-1, Tuple<T...>>::type;
};

int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a;
    Tuple<short,double> av;
    Tuple<char> ab;
    
    LastType<decltype(a)>::type xaa2a;///bool xaa2a;
    LastType<decltype(av)>::type xaa3a;///double xaa3a;
    LastType<decltype(ab)>::type xa4aa;///char xa4aa;
    
    return 0;
}
```

分析，上述代码中的Supscript，只能萃取到最后一个元素，萃取其他指定位置会报错，我们还得修改一下，使得它最好能萃取到任意指定位置。修改后如下：

```
template <int idx, typename ...T>
struct Supscript;

template <int idx,typename H, typename ...T>
struct Supscript<idx,Tuple<H,T...>>{
    using type = typename  Supscript<idx-1,Tuple<T...>>::type;
};

template <typename H, typename ...T>
struct Supscript<0,Tuple<H,T...>>{
    using type = H;
};

///为何不能这样使用？如下方式只能萃取到第一个元素
///template <int idx, typename ...T>
///using supscript_t = typename Supscript<idx, Tuple<T...>>::type;

///test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a;
    Tuple<short,double> av;
    Tuple<char> ab;
    
    Supscript<0, decltype(a)>::type xxx0;/// char xxx0;
    Supscript<1, decltype(a)>::type xxx1;/// short xxx1;
    Supscript<2, decltype(a)>::type xxx2;/// double xxx2;
    Supscript<3, decltype(a)>::type xxx3;///bool xxx3;
    Supscript<0, decltype(av)>::type xxx4;///short xxx4
    Supscript<1, decltype(av)>::type xxx5;///double xxx5;
    Supscript<0, decltype(ab)>::type xxx6;/// char xxx6;
    
     return 0;
}
```

经过上述代码改进后，Supscript可以萃取到任意指定位置的类型。

### 反转Tuple的类型列表

有了上述的Supscript，我们可以轻松在编译阶段萃取到任意位置的类型，借助这个方法可以轻松实现Tuple类型反转。

