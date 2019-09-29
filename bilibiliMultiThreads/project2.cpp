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
	1�����̵߳�ִ��˳�����ҵģ������ϵͳ�ڲ����̵߳����е��Ȼ����й�
	2�����߳�֮������ݹ������⣺
		2.1ֻ�������ǰ�ȫ�ȶ���
		2.2�ж���д���ݣ������߳�д�������̶߳���
	3������������ݱ��������⣬��������һ��������һ������
			�ѹ���������ס���������ݣ�������
			mutex
			����һ������󣬶���߳���lock()������ֻ��һ���߳̿�����ס��
			mutexҪѡ����ȷ�ı�������
			���裺lock->����->unlock
			Ҫ�ɶ�ʹ�ã�one-by-one
	4��c++11Ϊ�˷�ֹ����unlock�������lock_guard()��ģ�壬ֱ��ȡ��lock()&unlock()
		lock_guard�Ĺ��캯������ִ����mutex::lock()
		��lock_guard����������ִ����mutex::unlock()
	5��������������ǰ�������������������ֱ������������
		��������lockA��lockB���߳�Aִ�е�ʱ����סlockA����Ҫ��lockB��ʱ��ϵͳʱ��Ƭ�������������л���
		�߳�Bִ���ˣ�����lockB������lockB��û�б���ס�����lockB�ɹ���ס����ȥ��lockA����ʱ����������
	�����ֵ��������������������˳��һ�µ��µģ�����ô�������������ͬ�߳�֮�����������˳�򱣳�һ��
	6��std::lock()����ģ��-->���ڴ�����ֶ�������������
		{
			std::lock(lockA,lockB);
			lockA.unlock();
			lockB.unlock();
		}
		ͬʱ��ס�������ϵ������������������ǲ�����ֵ��������ķ���
		��ʹ�����еĻ�����ȫ����ס�Ż�ִ�У���Ȼ���ͷŵ�ǰ��������lock
	7��6�г��ֵ�lockģ�庯��������Ҫ�ֶ���������ˣ����Բ�����lock_guard������ϵķ�ʽ����Զ�����
		std::lock(my_mutexA,my_mutexB);
		std::lock_guard<std::mutex> lockguardA(my_mutexA,std::adopt_lock);
		std::lock_guard<std::mutex> lockguardB(my_mutexB,std::adopt_lock);
		adopt_lock��һ���ṹ������𵽱������
*/

std::vector<int> gv = { 1,2,3 };

#ifdef lessen7
class B
{
public:
	//���յ�����Ϣ���뵽һ�����е��߳�
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
			command = m_queue.front ();//���ص�һ��Ԫ�ص��ǲ���Ԫ���Ƿ����
			m_queue.pop_front (); //�Ƴ���һ��Ԫ��
			std::cout << "current command popped out: " << command << std::endl;
			my_mutex.unlock (); //���з�֧��Ҫ�н���
			return true;
		}
		my_mutex.unlock ();
		return false;
	}

	//���յ�����Ϣɾ��һ�����е��߳�
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
	std::list<int> m_queue; //��������Ϣ���У�����������
	std::mutex my_mutex;
};
#endif // lessen7

#ifdef lessen8
class B
{
public:
	//���յ�����Ϣ���뵽һ�����е��߳�
	void inMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			my_mutexA.lock ();
			//��������//
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
			command = m_queue.front ();//���ص�һ��Ԫ�ص��ǲ���Ԫ���Ƿ����
			m_queue.pop_front (); //�Ƴ���һ��Ԫ��
			std::cout << "current command popped out: " << command << std::endl;
			//my_mutex.unlock (); //���з�֧��Ҫ�н���
			return true;
		}
		my_mutexA.unlock ();
		my_mutexB.unlock ();
		return false;
	}

	//���յ�����Ϣɾ��һ�����е��߳�
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
	std::list<int> m_queue; //��������Ϣ���У�����������
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
	//���յ�����Ϣ���뵽һ�����е��߳�
	void inMsgRecvQueue ()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			my_mutexA.lock ();
			//��������//
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
			command = m_queue.front ();//���ص�һ��Ԫ�ص��ǲ���Ԫ���Ƿ����
			m_queue.pop_front (); //�Ƴ���һ��Ԫ��
			std::cout << "current command popped out: " << command << std::endl;
			return true;
		}
		return false;
	}

	//���յ�����Ϣɾ��һ�����е��߳�
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
	std::list<int> m_queue; //��������Ϣ���У�����������
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
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //�ڶ������������ã��������ܱ�֤�߳���ʹ�õ���ͬ������󣬵��ǾͲ���ʹ��detach��
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();
#endif // lessen7

#ifdef lessen8
	B myBobj;
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //�ڶ������������ã��������ܱ�֤�߳���ʹ�õ���ͬ������󣬵��ǾͲ���ʹ��detach��
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();
#endif

#ifdef lessen9
	B myBobj;
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //�ڶ������������ã��������ܱ�֤�߳���ʹ�õ���ͬ������󣬵��ǾͲ���ʹ��detach��
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();
#endif
	std::cout << "this is the main thread!" << std::endl;

	system ("pause");
	return 0;
}