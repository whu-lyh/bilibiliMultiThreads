#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>

//#define __WINDOWSCODING_
#ifdef __WINDOWSCODING_
#include <Windows.h>
#endif
/*
	1、windows临界区,和c++中互斥量的使用时一致的，只是需要初始化，进入临界区就等于加锁，离开临界区就相当于解锁
	１. 临界区基本用法，类似mutex的lock()、unlock()
		EnterCriticalSection(&winsSec);
		msg.push_back(i);
		LeaveCriticalSection(&winsSec);
	２. windows临界区，允许同一个临界区进入两次，muxtex对象不允许同一个对象lock()两次
		EnterCriticalSection(&winsSec); //ok
		EnterCriticalSection(&winsSec);
		msg.push_back(i);
		LeaveCriticalSection(&winsSec);
		LeaveCriticalSection(&winsSec);
	３. 自动析构技术
		windows临界区实现mutex的自动lock()和unlock()操作。RAII(resource aquisition is initialization)类，在构造函数里进行初始化，在析构函数里进行释放。
		class uniLockWins
		{
		private:
		   CRITICAL_SECTION *_critical_sec;
		public:
		   uniLockWins(CRITICAL_SECTION *sec){
		        _critical_sec = sec;
		        EnterCriticalSection(_critical_sec);
		   }
		   ~uniLockWins(){LeaveCriticalSection(_critical_sec);}
		};
	４. recursive_mutex:递归独占互斥量，允许同一个线程同一个互斥量多次lock()/unlock()
		就是用recursive_mutex直接代替mutex即可，和正常的mutex用法一致，此时允许mutex多次加锁，递归次数有限制，但是没试过。
		效率更低
		应该考虑重构代码，进行优化
	5、 std::timed_mutex、std::recursive_timed_mutex
		带超时功能的独占互斥量
		try_lock_for();等待一段时间，如果拿到lock返回true，则可以操作数据，如过没拿到lock则返回false，看是继续等待还是做其他操作
		try_lock_until();参数是未来时间点，在未来时间没到的时间内，如果拿到锁就走下来，如果没有拿到锁则继续进行后面的操作，而不会卡住
		
		std::chrono::steady_clock::now
	6、 线程池，循环利用线程，并统一管理的方式称为线程池，线程池在程序运行的时候就创建好了线程，
		不会再在临时使用的时候才申请创建，可以提升稳定性和效率,降低内存的消耗
		2000个线程基本上是极限了。创建线程的数量 = cpu数量，cpu*2 .etc
*/
#ifdef __WINDOWSCODING_
	//用于自动释放windows下的临界区，防止忘记LeaveCriticalSection导致死锁情况的发生
	//RAII (resource aquisition is initialization)类，在构造函数里进行初始化，在析构函数里进行释放。
	//智能指针，容器都属于RAII类，自动析构技术（自动释放）
class unLockWins
{
public:
	unLockWins (CRITICAL_SECTION *sec) {
		_critical_sec = sec;
		EnterCriticalSection (_critical_sec); //在构造函数中初始化临界区
	}
	~unLockWins () { LeaveCriticalSection (_critical_sec); } //在析构函数中释放临界区
private:
	CRITICAL_SECTION *_critical_sec;
};
#endif

class B6
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
#ifdef __WINDOWSCODING_
			EnterCriticalSection (&my_winsec);
			EnterCriticalSection (&my_winsec); //同一个线程中重复进入相同的临界区变量
			//下面的这行就是用来
			unLockWins unlockwindows (&my_winsec);
			m_queue.emplace_back (i);
			LeaveCriticalSection (&my_winsec);
			LeaveCriticalSection (&my_winsec); //重复离开临界区
#else
			my_mutex.lock (); //但是c++中的互斥量不允许同一个线程中同一个互斥量lock多次
			m_queue.emplace_back (i);
			my_mutex.unlock ();
#endif
		}
	}

	bool outMsgfunc(int & command)
	{
#ifdef __WINDOWSCODING_
		EnterCriticalSection (&my_winsec);
		if (!m_queue.empty ())
		{
			command = m_queue.front ();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front (); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			LeaveCriticalSection (&my_winsec); //所有分支都要有解锁
			return true;
		}
		LeaveCriticalSection (&my_winsec);
#else
		//my_mutex.lock ();
		std::lock_guard<std::mutex> lockguard (my_mutex); //不需要用户unlock
		if (!m_queue.empty ())
		{
			command = m_queue.front ();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front (); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			//my_mutex.unlock (); //所有分支都要有解锁
			std::chrono::microseconds dura(1);
			std::this_thread::sleep_for (dura);
			return true;
		}
		//my_mutex.unlock ();
#endif
		return false;
	}

	//把收到的消息删除一个队列的线程
	void outMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			int command = 0;
			if (m_queue.empty())
			{
				std::cout << "current message queue is empty: " << i << std::endl;
			}
			else
			{
				outMsgfunc(i);

			}
		}
		std::cout << "finish" << std::endl;
	}

#ifdef __WINDOWSCODING_
	InitializeCriticalSection(&my_winsec); //临界区初始化 TODO？？这里报错，尚未解决
#endif
protected:
private:
	std::list<int> m_queue; //容器（消息队列），共享数据
	std::mutex my_mutex;
#ifdef __WINDOWSCODING_
	CRITICAL_SECTION my_winsec; //windows中的临界区，非常类似于c++中的互斥量，但是临界区必须要初始化
#endif
};

int main()
{
	B6 myBobj;
	std::thread msgOutobj(&B6::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgInobj(&B6::inMsgRecvQueue, &myBobj);
	msgInobj.join();
	msgOutobj.join();

	std::cout << "this is the main thread!" << std::endl;

	system("pause");
	return 0;
}