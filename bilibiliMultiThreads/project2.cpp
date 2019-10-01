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
	8��//unique_lock��һ��ģ���࣬�����Ƽ�ʹ��lock_guard?lock_guardquȡ����mutex��lock&ublock����
		//unique_lockЧ�ʲ�һ�㣬�ڴ�ռ�ö�һЩ,���Ǹ�����why
		ֱ�Ӱ�lock_guard����unque_lock����
	9��std::unique_lock<std::mutex> uniquelockA(my_mutexA,std::adopt_lock);
		std::adopt_lock��ʾ�û������Ѿ���lock�ˣ���mutex���뱻��ǰlock������ᱨ��
		std::adopt_lock��ǵ�Ч�����ǣ�������÷��߳��Ѿ�ӵ���˻��������Ȩ������lock_guard����unique_lock�����������оͲ���������ublock�����ˡ�
	10��һ�����lockһ����ĳ���߳���lock�����ڴ��߳��У��������к�ʱ�ܾã��������߳̾��޷�lock��
		std::try_to_lock�᳢����mutex��lock������mutex�����û�������ɹ���Ҳ���������أ���������
		���try_to_lock��ǰ����֮ǰ�������ֶ�lock
		try_to_lock()���Ա�������һ���߳���ס��ʱ��������������飬������Ϊ������ֹͣ��
	11��std::defer_lock()��ǰ��Ҳ�ǲ����Լ���lock����unique_lock�ĵ�����������
		��ʹ��mutex��������ʱ�򲻻����������ǳ�ʼ��Ϊһ��δ������mutex
	12��unique_lock����Ҫ��Ա������
		lock()
		unlock()
		defer_lock()
		try_lock() ���Ը�����������������ò��������򷵻�false������õ�����������true�����������������
		release() �������������mutex����ָ�룬���ͷ�����Ȩ��unique_lock��mutex�������й�ϵ
		ע������unlock��release֮�������
		���ԭ���Ķ����ڼ���״̬���������θ��������release���ص���ԭʼmutex��ָ��
		std::mutex *ptx=my_mutex1.release()
		///���ɲ���
		ptx.unlock()
	13������ס�������Ķ��ٳ�Ϊ�������ȣ�һ���ô�ϸ����ʾ��ѡ����ʵ�������Ҫ����ѡ��
			a)��ס�Ĵ����٣�����Ϊϸ��ִ��Ч�ʸ�
			b)��ס�Ĵ���࣬����Ϊ�֣�ִ��Ч�ʵ�
	14��һ��unique_lock��һ��mutex��bind��һ��Ҳ����һ����һ��һ�Ĺ�ϵ��
		����Ȩ���һ��unique_lockӵ��һ��mutex������Ȩ
		unique_lock���Խ����Ӧ��mutex����Ȩ�ǿ���ת�Ƶģ����ǲ��ܸ��ơ�
		e.g.
		std::unique_lock<std::mutex> uniqeuguardA(my_mutexA);
		std::unique_lock<std::mutex> uniqeuguardB(std::move(uniqeuguardA));
		��uniqeuguardB��my_mutexA��bind��һ��uniqeuguardAָ��գ�uniqeuguardBָ��my_mutexA
		����Ȩת�Ƶ�ʱ������״̬���������������ǲ���ġ� 
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
				std::cout << "inMsgRecvQueue ִ�е���û���õ�����" <<i<< std::endl;
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
#endif

#ifdef LESSEN_UNIQUE_LOCK
class B
{
public:
	std::unique_lock<std::mutex> rtn_unique_lock()
	{
		std::unique_lock<std::mutex>tmpguard(my_mutexA);
		return tmpguard;
		//�����ķ���ֵ��һ���ֲ���unique_lock����tmpguard��������ʱunique_lock���󣬲�����unique_lock���ƶ����캯��
	}

	//���յ�����Ϣ���뵽һ�����е��߳�
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::unique_lock<std::mutex> uniqeuguardA(my_mutexA, std::defer_lock);
			//{
			//	uniqeuguardA.lock();//ͬ������Ҫ�ֶ�unlock
			//	//���������

			//	uniqeuguardA.unlock();
			//	//��ʱ��������ʱ���Դ���ǹ������

			//	uniqeuguardA.lock();
			//	//��ʱ�ֿ��Դ����������
			//}

			//lockת�ƴ�
			std::unique_lock<std::mutex> uniqueguardAmoved = rtn_unique_lock();
			if (uniqeuguardA.try_lock() == true)
			{
				m_queue.emplace_back(i);
			} 
			else
			{
				std::cout << "try_lock û�õ���" << std::endl;
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
			command = m_queue.front();//���ص�һ��Ԫ�ص��ǲ���Ԫ���Ƿ����
			m_queue.pop_front(); //�Ƴ���һ��Ԫ��
			std::cout << "current command popped out: " << command << std::endl;
			return true;
		}
		return false;
	}

	//���յ�����Ϣɾ��һ�����е��߳�
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
	std::list<int> m_queue; //��������Ϣ���У�����������
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
	std::thread msgOutobj (&B::outMsgRecvQueue, &myBobj); //�ڶ������������ã��������ܱ�֤�߳���ʹ�õ���ͬ������󣬵��ǾͲ���ʹ��detach��
	std::thread msgInobj (&B::inMsgRecvQueue, &myBobj);
	msgInobj.join ();
	msgOutobj.join ();

	std::cout << "this is the main thread!" << std::endl;

	//system("pause");
	return 0;
}