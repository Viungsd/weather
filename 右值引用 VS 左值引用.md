## 右值引用 VS 左值引用？

**1、左、右引用定义变量后，它自身都是左值（可以取地址、有名字），只是初始化时接受的类型不同，初始化的本质都引用相同的地址（不会触发copy）。**

**2、如果右值引用 作为函数参数使用，实参必须时右值（临时值），参数传递过程**

**3、右值引用作为函数返回值，不能返回函数退出后就会被释放的局部变量。且函数值降作为右值使用。**



|     Expression     |                        Value category                        |
| :----------------: | :----------------------------------------------------------: |
|         4          |                            rvalue                            |
|         i          |                            lvalue                            |
|        a+b         |                            rvalue                            |
|         &a         |                            rvalue                            |
|         *p         |                            lvalue                            |
|        ++i         |           lvalue(前缀自增运算符直接在原变量上自增)           |
|        i++         | rvalue(后缀自增运算符先拷贝一份变量，自增后再重新赋值给原变量) |
| std::string(“oye”) |                            rvalue                            |
|     str1+str2      | rvalue(重载的`+`运算符返回的是一个临时的`std::string`对象而不是引用) |
|       vec[0]       |           lvalue(重载的`[]`运算符返回类型为`int&`)           |
|         m          |             lvalue(引用了一个右值，但本身是左值)             |

# Universal reference

C++ 11中引入了右值引用(rvalue reference)用于表示移动语义（绑定了右值）。在C++ 11中，类型`T`的右值引用表示为`T&&`。然而并不是所有的形如`T&&`的类型都为rvalue reference。形如`T&&`的类型有两种情况：一种是普通的rvalue reference，另一种则被称为 **universal reference**，它既可能表示lvalue reference，也可能表示rvalue reference。那么如何区分这两种引用呢？根据 *Effective Modern C++, Item 24* 总结一下：

- 如果一个函数模板参数的类型为`T&&`，其中`T`是需要推导的类型，那么此处的`T&&`指代universal reference。若不严格满足`T&&`类型或没有发生类型推导，则`T&&`指代rvalue reference。比如下面的`param`参数即为universal reference：

```
template<typename T>
void f(T&& param);
```

上面有两个要点：类型为`T&&`和参与类型推导。也就是说`T&&`加上cv qualifiers就不是universal reference了，比如下面的`param`参数为rvalue reference而不是universal reference：

```
template<typename T>
void f(const T&& param);
```

另外一点就是要参与类型推导。比如以下`vector`类中的`push_back`函数，虽然参数类型为`T&&`，但是并未参与类型推导。因为`vector`模板实例化的时候，`T`的类型是已经推导出来的(如`std::vector<T>, T = int`)，因此参数仍然为rvalue reference:

```
template<class T, class Allocator = allocator<T>>
class vector {
public:
	void push_back(T&& x);
	// ...
};
```

而`emplace_back`模板函数则需要进行类型推导，因此对应的参数`Args&&`为universal reference(为方便起见，将此处的parameter pack看作是一个type parameter)：

```
template<class T, class Allocator = allocator<T>>
class vector {
public:
	template <class... Args>
	void emplace_back(Args&&... args);
	// ...
};
```

- 如果一个对象的类型被定义为`auto&&`，则此对象为universal reference，比如`auto&& f = g()`

那么如何确定universal reference的引用类型呢？有如下规则：如果universal reference是通过rvalue初始化的，那么它就对应一个rvalue reference；如果universal reference是通过lvalue初始化的，那么它就对应一个lvalue reference。

确定了universal reference的引用类型后，编译器需要推导出`T&&`中的`T`的真实类型：若传入的参数是一个左值，则`T`会被推导为左值引用；而如果传入的参数是一个右值，则`T`会被推导为原生类型（非引用类型）。这里面会涉及到编译器的 **reference collapsing** 规则，下面来总结一下。

# Reference collapsing

考虑以下代码：

```
class A {};

template<typename T>
void f(T&& param);

A a;
f(a); // T = A&
```

根据上面总结的规则，`T = A&, T&& = A&`。然而将`T&&`展开后我们发现这玩意像是一个reference to reference：

```
void f(A& && param);
```

如果我们直接表示这样的多层引用，编译器会报错；而这里编译器却允许在一定的情况下进行隐含的多层引用推导，这就是 **reference collapsing** (引用折叠)。C++中有两种引用（左值引用和右值引用），因此引用折叠就有四种组合。引用折叠的规则：

> **如果两个引用中至少其中一个引用是左值引用，那么折叠结果就是左值引用；否则折叠结果就是右值引用。**

直观表示：

- `T& &` = `T&`
- `T&& &` = `T&`
- `T& &&` = `T&`
- `T&& &&` = `T&&`

