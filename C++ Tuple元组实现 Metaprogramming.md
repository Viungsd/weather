# C++ Tuple 元组实现 Metaprogramming

核心玩法：模板递归、偏特化

    template<typename ... Tn>struct Tuple;
    
    template<typename T0,typename ... Tn>
    struct Tuple<T0,Tn...>{
        T0 head;
        Tuple<Tn...>tail;
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

通过Supscript可以萃取指定位置的类型，代码如下：

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

template <typename H,typename ...T>
struct Supscript<0,Tuple<H,T...>>{
    using type = H;
};

///为何不能这样写？
///template <typename ...T>
///using last_t = typename Supscript<sizeof...(T)-1, T...>::type;

template<typename ...T>struct LastType;

template<typename ...T>
struct LastType<Tuple<T...>>{
    using type = typename Supscript<sizeof...(T)-1, Tuple<T...>>::type;
};

int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a;
    Tuple<short,double> av;
    Tuple<char> ab;
    
    Supscript<0, decltype(a)>::type xx4;
    Supscript<1, decltype(a)>::type xx43;
    Supscript<2, decltype(a)>::type x4x4;
    Supscript<3, decltype(a)>::type xx554;
    Supscript<0, decltype(av)>::type xx444;
    Supscript<1, decltype(av)>::type xx334;
    
    LastType<decltype(a)>::type xaa2a;///bool xaa2a;
    LastType<decltype(av)>::type xaa3a;///double xaa3a;
    LastType<decltype(ab)>::type xa4aa;///char xa4aa;
    
    return 0;
}
```

上述代码，也可以用另外一种写法实现【好像换汤不换药，逻辑是一样的】，修改后如下：

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

template <int idx, typename ...T>
using supscript_t = typename Supscript<idx,T...>::type;

///test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a;
    Tuple<short,double> av;
    Tuple<char> ab;
    
    supscript_t<0, decltype(a)> xxx0;/// char xxx0;
    supscript_t<1, decltype(a)> xxx1;/// short xxx1;
    supscript_t<2, decltype(a)> xxx2;/// double xxx2;
    supscript_t<3, decltype(a)> xxx3;///bool xxx3;
    supscript_t<0, decltype(av)> xxx4;///short xxx4
    supscript_t<1, decltype(av)> xxx5;///double xxx5;
    supscript_t<0, decltype(ab)> xxx6;/// char xxx6;
    supscript_t<1, decltype(ab)> xxx7;/// Error
    
     return 0;
}
```

经过上述代码改进后，Supscript依然可以萃取到任意指定位置的类型。

## 5、翻转Tuple的类型列表

借助上面PushBackC这个方法可以轻松实现Tuple类型反转。

```
template <int idx, typename ...T>struct Reverse;

template <int idx,typename H, typename ...T>
struct Reverse<idx,Tuple<H,T...>>{
    using type =  typename PushBackC<H,typename Reverse<idx-1,Tuple<T...>>::type>::type;
};

template <typename H, typename ...T>
struct Reverse<0,Tuple<H,T...>>{
    using type = Tuple<H>;
};

template <int idx, typename ...T>
using reverse_t = typename Reverse<idx, T...>::type;

int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a;
    Tuple<short,double> av;
    Tuple<char> ab;
    
    reverse_t<0, decltype(a)> xfdas0;/// Tuple<char> xfdas0;
    reverse_t<1, decltype(a)> xfdas1;/// Tuple<short,char> xfdas1;
    reverse_t<2, decltype(a)> xfdas2;/// Tuple<double,short,char> xfdas2;
    reverse_t<3, decltype(a)> xfdas3;/// Tuple<bool,double,short,char> xfdas3;
    reverse_t<0, decltype(av)> xfdas4;/// Tuple<short> xfdas4;
    reverse_t<1, decltype(av)> xfdas5;/// Tuple<double,short> xfdas5;
    
     return 0;
}
```

## 6、两个Tuple合并

可以想办法把两个Tuple内部的类型合并成到一个Tuple中：

```
template<typename T,typename ...T1> struct Merge;

template<typename T,typename T1,typename ...T2>
struct Merge<T,Tuple<T1,T2...>>{
private:
    using Head = decltype(PushBackT(T(),T1()));
    using Trail = Tuple<T2...>;
public:
    using type = typename Merge<Head,Trail>::type;
};

template<typename T,typename T1>
struct Merge<T,Tuple<T1>>{
public:
    using type = decltype(PushBackT(T(),T1()));
};

template<typename T,typename ...T1>
using merge_t = typename Merge<T, T1...>::type;

int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a;
    Tuple<short,double> av;
    Tuple<char> ab;
    
    merge_t<decltype(ab), decltype(av)> x11;///Tuple<char,short,double> x11;
    merge_t<decltype(av), decltype(av)> x12;/// Tuple<short,double,short,double> x12;
    merge_t<decltype(av), decltype(ab)> x13;/// Tuple<short,double,char> x13;
    
     return 0;
}
```

## 7、判断两个Tuple的类型列表中是否有相同的类型

利用上面的Search方法可以判断Tuple中是否包含特定类型，因而我们可以再进一步判断两个Tuple是否有相同的类型，有则返回true，否则返回false，代码参考如下：

```
template<typename T1,typename ...T2>struct ContainSame;

template<typename T1,typename Tn1,typename ...Tn2>
struct ContainSame<T1,Tuple<Tn1,Tn2...>> : std::conditional_t<Search<Tn1,T1>::value, std::true_type, ContainSame<T1, Tuple<Tn2...>>>{
};

template<typename T1,typename Tn1>
struct ContainSame<T1,Tuple<Tn1>> :Search<Tn1,T1>{///从Tuple:T1中搜索是否包含类型Tn1
};

template<typename T1,typename ...T2>
bool contain_same_v = ContainSame<T1,T2...>::value;

int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a ('a',34,6.8,true);
    Tuple<short,double> av(90,88.9);
    Tuple<char> ab;
    
    bool xx0 = contain_same_v<decltype(a), decltype(av)>;///true
    bool xx1 = contain_same_v<decltype(a), decltype(ab)>;///true
    bool xx2 = contain_same_v<decltype(ab), decltype(av)>;///false
    bool xx3 = contain_same_v<decltype(a), decltype(a)>;///true
        
     return 0;
}
```



# Tuple对象的变换

如果说刚才都是Tuple类型列表变换，那我们要讨论一下对象如何变换？也就是形如一个Tuple<char>对象，如何变换到其他类型的对象？增加一个类型？删除一个类型？或者翻转类型？

#### 问题1:如何获取、修改Tuple对象指定位置的元素值？

```
template<int idx,typename ...T>
auto& getTuple(Tuple<T...>& tuple){///确保参数只接受左值，函数返回左值，所以可以同时支持读、写
    if constexpr (0 == idx) {///利用了constexpr编译时特性，Since C++17，否则就只能使用模板递归实现了
        return  tuple.head;
    }else{
        return getTuple<idx-1>(tuple.tail);
    }
}

///Test
int main(int argc, const char * argv[]) {
    Tuple<char,short,double,bool> a ('a',34,6.8,true);
    Tuple<short,double> av(90,88.9);
    Tuple<char> ab;
    
    auto xxx0 =  getTuple<0>(a);///copy -> 'a'
    auto &xxx = getTuple<0>(a);///lvalue reference
    xxx = '9';///modify
    getTuple<3>(a) = false;///modify
    auto xxx1 =  getTuple<1>(a);///34
    auto xxx2 =  getTuple<2>(a);///6.8
    auto xxx3 =  getTuple<3>(a);///true
    auto xxx4 =  getTuple<0>(av);///90
    
     return 0;
}
```

上述getTuple利用了constexpr编译时特性，C++17之前只能使用模板递归实现，参考如下代码，由于函数模板不支持偏特化，所以需要函数外面套上一层类TP，借助TP类的模板偏特化来实现递归结束：

```
template<int idx,typename ...T>struct TP;

template<int idx,typename H,typename ...T>
struct TP<idx,Tuple<H,T...>>{
    static auto& fun(Tuple<H,T...>& tp){
        return TP<idx-1,Tuple<T...>>::fun(tp.tail);
    }
};

template<typename H,typename ...T>
struct TP<0,Tuple<H,T...>>{
    static auto& fun(Tuple<H,T...>& tp){
        return tp.head;
    }
};

template<int idx,typename ...T>
auto& getTuple(Tuple<T...>& tuple){
    return TP<idx,Tuple<T...>>::fun(tuple);
}
```



```
template<typename Head,typename ...Trail>
inline auto pushFront(const Head& h,const Tuple<Trail...> &trail){///把对象h放到Tuple的首部，生成一个新的Tuple
    using T =  decltype(PushForntT(h, trail));
    return T(h,trail);
}
```

