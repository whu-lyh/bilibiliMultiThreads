#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>

class B6
{
public:
	//把收到的消息加入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			my_mutex.lock();
			m_queue.emplace_back(i);
			my_mutex.unlock();
		}
	}

	bool outMsgfunc(int & command)
	{
		my_mutex.lock();
		if (!m_queue.empty())
		{
			command = m_queue.front();//返回第一个元素但是不管元素是否存在
			m_queue.pop_front(); //移除第一个元素
			std::cout << "current command popped out: " << command << std::endl;
			my_mutex.unlock(); //所有分支都要有解锁
			return true;
		}
		my_mutex.unlock();
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
	std::mutex my_mutex;
};

std::vector<int> gv6 = { 1,2,3 };

void myprintproj6(const int i)
{
	std::cout << "sub thread begin here!	num=" << i << std::endl;
	std::cout << "sub-thread id: " << std::this_thread::get_id() << std::endl;
	std::cout << gv6[0] << "," << gv6[1] << "," << gv6[3] << std::endl;
}


int main()
{
	std::vector<std::thread> mythreads;
	for (int i = 0; i < 10; i++)
	{
		mythreads.emplace_back(std::thread(myprintproj6, i));
	}
	for (auto iter = mythreads.begin(); iter != mythreads.end(); iter++)
	{
		iter->join();
	}

	B6 myBobj;
	std::thread msgOutobj(&B6::outMsgRecvQueue, &myBobj); //第二个参数是引用，这样才能保证线程里使用的是同意个对象，但是就不能使用detach了
	std::thread msgInobj(&B6::inMsgRecvQueue, &myBobj);
	msgInobj.join();
	msgOutobj.join();

	std::cout << "this is the main thread!" << std::endl;

	system("pause");
	return 0;
}