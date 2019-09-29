#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>

//#define lessen7
//#define lessen8
#define lessen9

/*
	1、多线程的执行顺序是乱的，与操作系统内部对线程的运行调度机制有关
	2、多线程之间的数据共享问题：
		2.1只读数据是安全稳定的
		2.2有读有写数据（两个线程写，两个线程读）
	3、解决共享数据保护的问题，互斥量（一个互斥量一把锁）
			把共享数据锁住，操作数据，解锁。
			mutex
			就是一个类对象，多个线程用lock()加锁，只有一个线程可以锁住，
			mutex要选择正确的保护部分
			步骤：lock->操作->unlock
			要成对使用，one-by-one
	4、c++11为了防止忘记unlock，提出了lock_guard()类模板，直接取代lock()&unlock()
		lock_guard的构造函数里面执行了mutex::lock()
		在lock_guard析构函数中执行了mutex::unlock()
	5、死锁（产生的前提是有两个互斥量，分别产生两个锁）
		有两把锁lockA，lockB：线程A执行的时候锁住lockA，正要锁lockB的时候，系统时间片出现了上下文切换，
		线程B执行了，先锁lockB，由于lockB还没有被锁住，因此lockB成功锁住，再去锁lockA，此时产生死锁。
	（出现的问题就是两个锁上锁的顺序不一致导致的），那么解决方案即：不同线程之间的锁的上锁顺序保持一致
	6、std::lock()函数模板-->用于处理出现多个互斥量的情况
		{
			std::lock(lockA,lockB);
			lockA.unlock();
			lockB.unlock();
		}
		同时锁住两个以上的锁，至少两个，但是不会出现导致死锁的风险
		会使得所有的互斥量全部锁住才会执行，不然就释放当前已上锁的lock
	7、6中出现的lock模板函数总是需要手动解锁，因此，可以采用与lock_guard函数结合的方式完成自动解锁
		std::lock(my_mutexA,my_mutexB);
		std::lock_guard<std::mutex> lockguardA(my_mutexA,std::adopt_lock);
		std::lock_guard<std::mutex> lockguardB(my_mutexB,std::adopt_lock);
		adopt_lock是一个结构体对象，起到标记作用
*/

std::vector<int> gv = { 1,2,3 };

#ifdef lessen7
class B
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			my_mutex.lock ();
			m_queue.emplace_back (i);
			my_mutex.unlock ();
		}
	}

	bool outMsgfunc (int & command)
	{
		my_mutex.lock ();
		if (!m_queue.empty ())
		{
			command = m_queue.front ();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front (); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			my_mutex.unlock (); //所有分支都要有解锁
			return true;
		}
		my_mutex.unlock ();
		return false;
	}

	//把收到的消息删除一个队列的线程
	void outMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			int command = 0;
			if (m_queue.empty ())
			{
				std::cout << "current message queue is empty: " << i << std::endl;
			}
			else
			{
				outMsgfunc (i);

			}
		}
		std::cout << "finish" << std::endl;
	}
protected:
private:
	std::list<int> m_queue; //容器（消息队列），共享数据
	std::mutex my_mutex;
};
#endif // lessen7

#ifdef lessen8
class B
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			my_mutexA.lock ();
			//其他操作//
			my_mutexB.lock ();
			m_queue.emplace_back (i);
			my_mutexA.unlock ();
			my_mutexB.unlock ();
		}
	}
	
	bool outMsgfunc (int & command)
	{
		std::lock_guard<std::mutex> lockguard (my_mutexA);
		my_mutexB.lock ();
		my_mutexA.lock ();
		if (!m_queue.empty ())
		{
			command = m_queue.front ();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front (); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			//my_mutex.unlock (); //所有分支都要有解锁
			return true;
		}
		my_mutexA.unlock ();
		my_mutexB.unlock ();
		return false;
	}

	//把收到的消息删除一个队列的线程
	void outMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			int command = 0;
			if (m_queue.empty ())
			{
				std::cout << "current message queue is empty: " << i << std::endl;
			}
			else
			{
				outMsgfunc (i);

			}
		}
		std::cout << "finish" << std::endl;
	}
protected:
private:
	std::list<int> m_queue; //容器（消息队列），共享数据
	std::mutex my_mutexA;
	std::mutex my_mutexB;
};

void myprint (const int i)
{
	std::cout << "sub thread begin here!	num=" << i << std::endl;
	std::cout << "sub-thread id: " << std::this_thread::get_id () << std::endl;
	std::cout << gv [0] << "," << gv [1] << "," << gv [3] << std::endl;
}
#endif // lessen8

#ifdef lessen9
class B
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			my_mutexA.lock ();
			//其他操作//
			my_mutexB.lock ();
			m_queue.emplace_back (i);
			my_mutexA.unlock ();
			my_mutexB.unlock ();
		}
	}

	bool outMsgfunc (int & command)
	{
		std::lock (my_mutexA, my_mutexB);
		std::lock_guard<std::mutex> lockguardA (my_mutexA, std::adopt_lock);
		std::lock_guard<std::mutex> lockguardB (my_mutexB, std::adopt_lock);
		if (!m_queue.empty ())
		{
			command = m_queue.front ();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front (); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			return true;
		}
		return false;
	}

	//把收到的消息删除一个队列的线程
	void outMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			int command = 0;
			if (m_queue.empty ())
			{
				std::cout << "current message queue is empty: " << i << std::endl;
			}
			else
			{
				outMsgfunc (i);

			}
		}
		std::cout << "finish" << std::endl;
	}
protected:
private:
	std::list<int> m_queue; //容器（消息队列），共享数据
	std::mutex my_mutexA;
	std::mutex my_mutexB;
};

void myprint (const int i)
{
	std::cout << "sub thread begin here!	num=" << i << std::endl;
	std::cout << "sub-thread id: " << std::this_thread::get_id () << std::endl;
	std::cout << gv [0] << "," << gv [1] << "," << gv [3] << std::endl;
}
#endif

int main ()
{
	std::vector<std::thread> mythreads;
	for (int i = 0; i < 10; i++)
	{
		mythreads.emplace_back (std::thread (myprint, i));
	}
	for (auto iter = mythreads.begin();iter != mythreads.end();iter++)
	{
		iter->join ();
	}

#ifdef lessen7
	B myBobj;
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();
#endif // lessen7

#ifdef lessen8
	B myBobj;
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();
#endif

#ifdef lessen9
	B myBobj;
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();
#endif
	std::cout << "this is the main thread!" << std::endl;

	system ("pause");
	return 0;
}