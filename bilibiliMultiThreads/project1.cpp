#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>

/*
|	1.传递临时对象作为线程参数
|	把数值变量i传递到子线程，是从主线程复制到了子线程，是值传递，因此即使主线程和子线程之间为detach，主线程结束，子线程仍可以继续运行完毕
|	但是把字符串char传入子线程是采用的引用（指针）对应的地址是相同的。通过shift+F9可以查看变量在内存中的地址，一旦detach则肯定回报错
|	2.如何可以正确将char转换到子线程myprint函数
|	总结
|	若传递int这种简单类型参数，建议直接采用值传递的方式
|	若传递类对象，则尽量避隐式传递，在创建线程的时候就构建出临时对象，然后在函数形参中用引用来接，否则会使用拷贝构造函数多次，效率变低
|	终极结论
|	不适用detach 使用join
|	线程id
|	id是对应于每个线程的，每个线程对应不同的id，std::this_thread::get_id()
|	3.传递类对象，智能指针作为线程参数
|	std::ref函数
|	4.传递智能指针
|	5.用成员函数指针做线程函数
*/

//#define lessen4
//#define lessen5
//#define lessen6

#ifdef lessen6
class A
{
public:
	//mutable int m_i; // mutable即使是const的变量也可以修改，但是显然在子线程修改了成员变量的值，子线程也是用的是引用传参，但是返回到主线程的时候仍然没有修改原来的变量
	int m_i;
	//类型转换构造函数？int --> class A
	A (int a) :m_i (a) { std::cout << "constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	A (const A &a) :m_i (a.m_i) { std::cout << "copy constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	~A () { std::cout << "destory func run here!" << this << std::endl; }

	void thread_work (int num)
	{
		m_i = 0.02;
		std::cout << "sub thread member func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl;
	}

	void operator()(int num)
	{
		std::cout << "sub thread () func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl;
	}
};

void myprint2 (std::unique_ptr<int> ppp)
{
	std::cout << "sub main pA's location is: " << &ppp << "thread id: " << std::this_thread::get_id () << std::endl;
}
#endif // lessen6

#ifdef lessen5
class A
{
public:
	//mutable int m_i; // mutable即使是const的变量也可以修改，但是显然在子线程修改了成员变量的值，子线程也是用的是引用传参，但是返回到主线程的时候仍然没有修改原来的变量
	int m_i;
	//类型转换构造函数？int --> class A
	A (int a) :m_i (a) { std::cout << "constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	A (const A &a) :m_i (a.m_i) { std::cout << "copy constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	~A () { std::cout << "destory func run here!" << this << std::endl; }
};

void myprint (const int i,const A &pA) //这里无论使用什么进行参数传递，值传递或者引用传递类对象的时候，编译器总是按照拷贝构造函数的形式进行传递
{
	std::cout << &pA << std::endl;
}

//void myprint2 (const A &pA) //这里即使是引用也无法修改m的值(这一行对应mutable)
void myprint2 (A &pA)
{
	pA.m_i = 200;
	std::cout << "sub main pA's location is: " <<&pA << "thread id: " << std::this_thread::get_id () << std::endl;
}
#endif // lessen5

#ifdef lessen4
//void myprint (const int &i,char *pmychar)
/*
|	这里进行了一些小的修改，期望通过将char*转换为const string，实现从主线程到子线程之间的数据传递，
|	容易出现的问题是：存在现象存在主线程已经执行完了，系统才开始将char*转换为const string的可能性。
*/
void myprint (const int &i, const std::string &pmychar)
{
	std::cout << "this is sub thread number: " << i << std::endl;
	std::cout << pmychar << std::endl;
}
#endif //lessen4

int main1 () 
{
#ifdef lessen4
	int num = 1;
	int &num2 = num;
	char tempchar[] = "this is qianqian";
	std::thread mythreadobj (myprint,num,std::string(tempchar)); // 直接在传递的时候生成临时string对象
	mythreadobj.join ();
#endif //lessen4

#ifdef lessen5

	std::cout << "main thread id: " << std::this_thread::get_id () << std::endl;

	int num = 1;
	int num2 = 12;
	std::thread mythreadobj (myprint, num, A(num2)); //创建线程的同时构造临时对象进行参数传递时可行的（最安全）
	//std::thread mythreadobj2 (myprint2, A(num)); //temp A object,如果是这种形式，必须在myprint中使用const，否则会出现invoke错误

	A myAobj (10);
	std::thread mythreadobj3 (myprint2 , std::ref(myAobj)); //使用了std::ref的才是真正的引用
	std::cout << myAobj.m_i << std::endl; //can't be changed
	mythreadobj.join ();
	//mythreadobj2.join ();
	mythreadobj3.join ();
#endif //lessen5

#ifdef lessen6
	std::cout << "main thread id: " << std::this_thread::get_id () << std::endl;

	std::unique_ptr<int> myunique (new int(100));
	std::thread mythreadobj (myprint2, std::move(myunique)); //使用了std::ref的才是真正的引用
	//std::cout << myunique.m_i << std::endl; //can't be changed
	mythreadobj.join ();//ref的情况下，一定不能使用detach

	A myAobj(100);
	//std::thread mythreadobj2 (&A::thread_work, myAobj, 200);
	std::thread mythreadobj2 (&A::thread_work, std::ref(myAobj), 200); //这里竟然出现一个invoke的错误，在vs2015上面，再测试一下vs2017是不是同样会出现该错误。
	mythreadobj2.join ();
	//mythreadobj2.detach ();//ref下不可以使用

	A myAobj2 (100); //这里使用类型转换构造函数
	std::thread mythreadobj3 (myAobj, 200);
	//std::thread mythreadobj3 (std::ref (myAobj), 200);//这里时不调用拷贝构造函数的，后面如果使用detach是不安全的。
	mythreadobj3.join (); //如果前面的类对象在传入到子线程时使用的时拷贝构造函数，则使用detach就不安全了，而join还是安全的。

#endif // lessen6

	std::cout << "this is main thread!" << std::endl;

	system ("pause");
	return 0;
}