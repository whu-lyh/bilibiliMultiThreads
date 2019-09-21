#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>

/************************************************************************/
/*	生成进程的同时主线程自动执行,主线程执行完毕了，则整个进程执行完毕   */
/*		join主线程结束，但是子线程没结束，这种是不合格的程序			*/
/*		c++11中有detech()分离，主线程与子线程不汇合，主线程和子线程分离	*/
/*	detach之后主线程与子线程失去联系，子线程驻留再后台运行，子线程结束之*/
/*	后由c++运行时库帮助释放该线程相关的资源（linux下成为守护线程）		*/
/*	joinable()判断是否可以成功使用join和detach							*/
/************************************************************************/

#define lessen1		0		// 通过一般函数创建线程
#define lessen2		0		// 通过类对象创建线程
#define lessen3		0		// 通过lambda函数创建线程
#define lessen4		1

// 通过类对象创建线程
class Cthreadtest 
{
	/*
	在主线程中创建了该类，一旦主线程结束，会使得主线程中的类对象释放，但是在子线程中是复制了一个新的对象
	因此没有引用，指针之类的话，就不会出现什么大的问题
	*/

public:

	int &m_i; //由于这里是引用，因此如果主线程中使用了detach，导致主线程中的int变量提前释放，会使得子线程中m_i错误。

	Cthreadtest (int &i) :m_i (i) 
	{
		std::cout << "构造函数执行" << std::endl;
	}

	Cthreadtest (const Cthreadtest &test) :m_i (test.m_i)
	{
		std::cout << "拷贝构造函数执行" << std::endl;
	}

	~Cthreadtest ()
	{
		std::cout << "构造函数结束" << std::endl;
	}

	void operator()() //不带参数的运算符重载
	{
		std::cout << "sub thread operator begin here" << std::endl;
		std::cout << "m_i的值为：" << m_i << std::endl;
		std::cout << "sub thread operator end here" << std::endl;
	}
};

// 通过一般函数创建线程
void myprint () 
{
	std::cout << "sub thread begin here" << std::endl;

	std::cout << "sub thread end here" << std::endl;
}

int main () 
{
	std::cout << "Main thread begin here" << std::endl;

	if (lessen1)
	{
		std::cout << "茜茜最好看！" << std::endl;

		std::thread mythreadobj (myprint); //创建一个子线程，子线程的入口就是myprint函数，myprint执行完了之后子线程结束
		mythreadobj.join (); //主线程到这里之后开始阻塞，等待子线程结束，这里必须等待子线程

		//mythreadobj.detach (); //detach使子线程脱离自己的控制，一旦detach就无法再join了。

		// if (mythreadobj.joinable ())

			std::cout << "茜茜你是！" << std::endl;
	}
	if (lessen2)
	{
		int i = 6;
		Cthreadtest test(i);
		std::thread mythreadobj (test);
		mythreadobj.join ();
	}
	if (lessen3)
	{
		auto mylambdafunc = []
		{
			std::cout << "sub thread begin here" << std::endl;

			std::cout << "sub thread end here" << std::endl;
		};

		std::thread mythreadobj (mylambdafunc);
		mythreadobj.join ();
	}

	system ("pause");
	return 0;
}