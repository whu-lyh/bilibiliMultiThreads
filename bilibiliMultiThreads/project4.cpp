#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <stdlib.h>

#define CONDITION_VARIABLE_LESSEN
/*
	std::condition_variable条件变量，实际是一个类,需要和一个互斥量配对使用
	notify_one不一定能够唤醒wait()因为wait（）所在线程可能正在执行某个很耗时的部分，而不是卡在wait()处
	notify_one一次只会激活一个线程，如果两个线程如下面的outMsgRecvQueue函数（两个线程）中都有wiai()等待唤醒，则需要notify_all()
	notify_all
	*/

#ifdef CONDITION_VARIABLE_LESSEN
class FF
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			//try_to_lock
			std::unique_lock<std::mutex> uniqueguardA(my_mutexA);

			m_queue.emplace_back(i);
			my_cond.notify_one(); //call wait() func，但是不一定notify之后会执行到别的线程，有可能还是inMsgrecvQueue获取到uniqueguardA，导致会输入很多消息，并不是一个线程一个线程轮着run
		}
		return;
	}

	//把收到的消息删除一个队列的线程
	void outMsgRecvQueue()
	{
		while (true)
		{
			int command = 0;
			std::unique_lock<std::mutex> uniquelock(my_mutexA);
			/*
				wait()用来等一个东西，wait（）相当于是成员函数，第一个参数是一个互斥量，第二个参数是一个lambda表达式
				如果返回false（m_queue是空的）那么wait将互斥量（para1）解锁，并阻塞在本行，直到某个线程调用notify_one成员函数（重新唤醒wait()）为止。
				如果返回true （m_queue不是空的）那么wait直接run过，没有别的操作。
				如果wait没有第二个参数（para2），则默认para2返回false，不是条件堵塞，而是直接堵塞
				如果wait被notify_one唤醒后，wait重新尝试获取互斥量锁，如果获取不到，还是会卡在wait处，否则顺利通过。
				获取到锁之后重新加锁。
			*/
			my_cond.wait(uniquelock, [this] { //一个lambda就是一个可调用对象（函数）
				if (!m_queue.empty())
					return true;
				return false;
			});
			//如果程序执行到这里，那么互斥锁一定是锁着的
			command = m_queue.front();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front(); //移除第一个元素
			uniquelock.unlock();//unique_lock可以随时解锁
			std::cout << "current command popped out: " << command << std::endl;

			std::cout << "finish" << std::endl;
		}//end while
	}
protected:
private:
	std::list<int> m_queue; //容器（消息队列），共享数据
	std::mutex my_mutexA;
	std::condition_variable my_cond;
};
#endif

int main()
{
	FF myBobj;
	std::thread msgOutobj(&FF::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgOutobj2(&FF::outMsgRecvQueue, &myBobj);//两个wait函数
	std::thread msgInobj(&FF::inMsgRecvQueue, &myBobj);
	msgInobj.join();
	msgOutobj.join();
	msgOutobj2.join();

	std::cout << "this is the main thread!" << std::endl;

	system("pause");
	return 0;
}