#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <stdlib.h>

//#define lessen7
//#define lessen8
//#define lessen9
#define LESSEN_UNIQUE_LOCK

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
	8、//unique_lock是一个模板类，但是推荐使用lock_guard?lock_guardqu取代了mutex的lock&ublock（）
		//unique_lock效率差一点，内存占用多一些,但是更加灵活？why
		直接把lock_guard换成unque_lock即可
	9、std::unique_lock<std::mutex> uniquelockA(my_mutexA,std::adopt_lock);
		std::adopt_lock表示该互斥量已经被lock了（该mutex必须被提前lock，否则会报错）
		std::adopt_lock标记的效果就是，假设调用方线程已经拥有了互斥的所有权，则在lock_guard或者unique_lock的析构函数中就不会再条用ublock函数了。
	10、一般的锁lock一旦在某个线程中lock掉，在此线程中（假设运行耗时很久），其他线程就无法lock了
		std::try_to_lock会尝试用mutex的lock锁定该mutex，如果没有锁定成功，也会立即返回，不会阻塞
		因此try_to_lock的前提是之前不可以手动lock
		try_to_lock()可以避免在另一个线程锁住的时候仍能做别的事情，不会因为卡主而停止。
	11、std::defer_lock()的前提也是不能自己先lock，是unique_lock的第三个参数，
		在使用mutex互斥量的时候不会上锁，而是初始化为一个未上锁的mutex
	12、unique_lock的重要成员函数：
		lock()
		unlock()
		defer_lock()
		try_lock() 尝试给互斥量加锁，如果拿不到锁，则返回false，如果拿到了锁，返回true，这个函数不会阻塞
		release() 返回他所管理的mutex对象指针，并释放所有权，unique_lock和mutex对象不再有关系
		注意区分unlock和release之间的区别
		如果原来的对象处于枷锁状态，你有责任负责解锁，release返回的是原始mutex的指针
		std::mutex *ptx=my_mutex1.release()
		///自由操作
		ptx.unlock()
	13、锁锁住代码量的多少成为锁的粒度，一般用粗细来表示。选择合适的粒度需要谨慎选择。
			a)锁住的代码少，粒度为细，执行效率高
			b)锁住的代码多，粒度为粗，执行效率低
	14、一个unique_lock和一个mutex绑定bind在一起，也就是一般是一对一的关系。
		所有权概念：一个unique_lock拥有一个mutex的所有权
		unique_lock可以将其对应的mutex所有权是可以转移的，但是不能复制。
		e.g.
		std::unique_lock<std::mutex> uniqeuguardA(my_mutexA);
		std::unique_lock<std::mutex> uniqeuguardB(std::move(uniqeuguardA));
		将uniqeuguardB与my_mutexA绑定bind到一起，uniqeuguardA指向空，uniqeuguardB指向my_mutexA
		所有权转移的时候锁的状态（加锁、解锁）是不变的。 
*/

std::vector<int> gv = { 1,2,3 };

void myprint(const int i)
{
	std::cout << "sub thread begin here!	num=" << i << std::endl;
	std::cout << "sub-thread id: " << std::this_thread::get_id() << std::endl;
	std::cout << gv[0] << "," << gv[1] << "," << gv[3] << std::endl;
}

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
			//try_to_lock
			std::unique_lock<std::mutex> uniqueguardA(my_mutexA, std::try_to_lock);
			my_mutexB.lock();

			if (uniqueguardA.owns_lock())
			{
					m_queue.emplace_back(i);
					//other processes
			} 
			else
			{
				std::cout << "inMsgRecvQueue 执行但是没有拿到锁。" <<i<< std::endl;
			}
			my_mutexB.unlock ();
		}
		return;
	}

	bool outMsgfunc (int & command)
	{
		//unique_lock
		std::lock (my_mutexA,my_mutexB);
		std::unique_lock<std::mutex> lockguardA (my_mutexA, std::adopt_lock);

		std::lock_guard<std::mutex> lockguardB (my_mutexB, std::adopt_lock);

		std::chrono::microseconds dura(200000);
		std::this_thread::sleep_for(dura);

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
#endif

#ifdef LESSEN_UNIQUE_LOCK
class B
{
public:
	std::unique_lock<std::mutex> rtn_unique_lock()
	{
		std::unique_lock<std::mutex>tmpguard(my_mutexA);
		return tmpguard;
		//函数的返回值是一个局部的unique_lock对象，tmpguard会生成临时unique_lock对象，并调用unique_lock的移动构造函数
	}

	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::unique_lock<std::mutex> uniqeuguardA(my_mutexA, std::defer_lock);
			//{
			//	uniqeuguardA.lock();//同样不需要手动unlock
			//	//处理共享代码

			//	uniqeuguardA.unlock();
			//	//临时解锁，此时可以处理非共享代码

			//	uniqeuguardA.lock();
			//	//此时又可以处理共享代码了
			//}

			//lock转移大法
			std::unique_lock<std::mutex> uniqueguardAmoved = rtn_unique_lock();
			if (uniqeuguardA.try_lock() == true)
			{
				m_queue.emplace_back(i);
			} 
			else
			{
				std::cout << "try_lock 没拿到锁" << std::endl;
			}
		}
		return;
	}

	bool outMsgfunc(int & command)
	{
		//unique_lock
		std::lock(my_mutexA, my_mutexB);
		std::unique_lock<std::mutex> lockguardA(my_mutexA, std::adopt_lock);
		std::lock_guard<std::mutex> lockguardB(my_mutexB, std::adopt_lock);

		if (!m_queue.empty())
		{
			command = m_queue.front();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front(); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			return true;
		}
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
protected:
private:
	std::list<int> m_queue; //容器（消息队列），共享数据
	std::mutex my_mutexA;
	std::mutex my_mutexB;
};

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

	B myBobj;
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();

	std::cout << "this is the main thread!" << std::endl;

	//system("pause");
	return 0;
}