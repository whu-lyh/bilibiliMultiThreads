#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <stdlib.h>

/*
	1、设计模式简介
		程序灵活，维护起来可能很方便，但是别人接管，阅读代码比较困难05年左右比较流行的一本书<<head first>>
	2、单例设计模式single instance model (配置文件读写)
	3、单例模式共享数据问题分析和解决
		在主线程创建单例类最好
		但是如果是在多个子线程中创建单例类的对象，这是就需要getinstance函数互斥
	4、std::call_once()也是c++11中的函数模板，第二个参数是一个函数名a()
		功能是保证函数a只能被调用一次, 具备互斥量能力，而却比互斥量消耗的资源更少
		需要和std::once_flag这个标记结合使用，once_flag是一个结构
		call_once()执行成功后once_flag标记为以调用，后续线程就不会再调用a()了
		
*/

#define CALLONCE

std::mutex g_mutex;
std::once_flag g_flag;

class singleInstanceclass
{
private:
	singleInstanceclass() {};

public:
	static singleInstanceclass *getInstance()
	{
#ifndef CALLONCE
		if (m_instance == NULL) //双重锁定（双重检查）会提高效率
								// A) 如果m_instance ！= NULL成立，则肯定表示m_instance已经new了
								// B) 如果m_instance == NULL，不代表m_isntance一定没被new过（还有可能是被加锁了？需要阻塞了）
		{//上面的那一行其实也只会执行一次，后面的时候全是！=null
			std::unique_lock<std::mutex> mymutex(g_mutex); // 自动加锁
			if (m_instance == NULL)
			{
				m_instance = new singleInstanceclass();
				static garbageRecycle recycle1; // 静态类，生命周期是整个程序执行的时候，一旦程序退出，会执行静态类的析构函数，从而将new的对象delete掉
			}
		}
#endif
		std::call_once(g_flag, createInstance); //g_flag相当于是一把锁mutex
		std::cout << "call once func run done" << std::endl;
		return m_instance;
	}
	class garbageRecycle // 类中嵌套类，用来释放对象
	{
	public:
		~garbageRecycle()
		{
			if (singleInstanceclass::m_instance)
			{
				delete singleInstanceclass::m_instance;
				singleInstanceclass::m_instance = NULL;
			}
		}
	};

	void testFunc()
	{
		std::cout << "test" << std::endl;
	}
#ifdef CALLONCE
private:
	static void createInstance()
	{
		std::chrono::microseconds timedura(2000);
		std::this_thread::sleep_for(timedura);
		std::cout << "this func is only run for once" << std::endl;
		m_instance = new singleInstanceclass();
		static garbageRecycle recycle1;
	}
#endif

private:
	static singleInstanceclass *m_instance; //静态成员变量
};

//成员变量初始化，但是为什么指针成员的初始化是这样写的？直接在：：后面写不行？
singleInstanceclass *singleInstanceclass::m_instance = NULL;

//线程入口函数
void myThreadFunc()
{
	std::cout << "my thread func begin here" << std::endl;
	singleInstanceclass *p_single = singleInstanceclass::getInstance();
	p_single->testFunc();
	std::cout << "my thread func end here" << std::endl;
	return;
}

int main3()
{
	singleInstanceclass *p_singleclassA = singleInstanceclass::getInstance(); //创建一个对象，并返回该类（singleInstanceclass）对象的指针
	singleInstanceclass *p_singleclassB = singleInstanceclass::getInstance(); //返回的对象指针是相同的。
	p_singleclassA->testFunc();
	p_singleclassB->testFunc();
	singleInstanceclass::getInstance()->testFunc();

	std::thread mythreadObj1(myThreadFunc);
	std::thread mythreadObj2(myThreadFunc);
	mythreadObj1.join();
	mythreadObj2.join();

	system("pause");
	return 0;
}