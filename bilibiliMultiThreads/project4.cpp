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
	std::condition_variable����������ʵ����һ����,��Ҫ��һ�����������ʹ��
	notify_one��һ���ܹ�����wait()��Ϊwait���������߳̿�������ִ��ĳ���ܺ�ʱ�Ĳ��֣������ǿ���wait()��
	notify_oneһ��ֻ�ἤ��һ���̣߳���������߳��������outMsgRecvQueue�����������̣߳��ж���wiai()�ȴ����ѣ�����Ҫnotify_all()
	notify_all
	*/

#ifdef CONDITION_VARIABLE_LESSEN
class FF
{
public:
	//���յ�����Ϣ���뵽һ�����е��߳�
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 10000; i++)
		{
			std::cout << "sub thread inMsgRecvQueue() func begin here!	insert a member" << i << std::endl;
			//try_to_lock
			std::unique_lock<std::mutex> uniqueguardA(my_mutexA);

			m_queue.emplace_back(i);
			my_cond.notify_one(); //call wait() func�����ǲ�һ��notify֮���ִ�е�����̣߳��п��ܻ���inMsgrecvQueue��ȡ��uniqueguardA�����»�����ܶ���Ϣ��������һ���߳�һ���߳�����run
		}
		return;
	}

	//���յ�����Ϣɾ��һ�����е��߳�
	void outMsgRecvQueue()
	{
		while (true)
		{
			int command = 0;
			std::unique_lock<std::mutex> uniquelock(my_mutexA);
			/*
				wait()������һ��������wait�����൱���ǳ�Ա��������һ��������һ�����������ڶ���������һ��lambda���ʽ
				�������false��m_queue�ǿյģ���ôwait����������para1���������������ڱ��У�ֱ��ĳ���̵߳���notify_one��Ա���������»���wait()��Ϊֹ��
				�������true ��m_queue���ǿյģ���ôwaitֱ��run����û�б�Ĳ�����
				���waitû�еڶ���������para2������Ĭ��para2����false��������������������ֱ�Ӷ���
				���wait��notify_one���Ѻ�wait���³��Ի�ȡ���������������ȡ���������ǻῨ��wait��������˳��ͨ����
				��ȡ����֮�����¼�����
			*/
			my_cond.wait(uniquelock, [this] { //һ��lambda����һ���ɵ��ö��󣨺�����
				if (!m_queue.empty())
					return true;
				return false;
			});
			//�������ִ�е������ô������һ�������ŵ�
			command = m_queue.front();//���ص�һ��Ԫ�ص��ǲ���Ԫ���Ƿ����
			m_queue.pop_front(); //�Ƴ���һ��Ԫ��
			uniquelock.unlock();//unique_lock������ʱ����
			std::cout << "current command popped out: " << command << std::endl;

			std::cout << "finish" << std::endl;
		}//end while
	}
protected:
private:
	std::list<int> m_queue; //��������Ϣ���У�����������
	std::mutex my_mutexA;
	std::condition_variable my_cond;
};
#endif

int main()
{
	FF myBobj;
	std::thread msgOutobj(&FF::outMsgRecvQueue, &myBobj); //�ڶ������������ã��������ܱ�֤�߳���ʹ�õ���ͬ������󣬵��ǾͲ���ʹ��detach��
	std::thread msgOutobj2(&FF::outMsgRecvQueue, &myBobj);//����wait����
	std::thread msgInobj(&FF::inMsgRecvQueue, &myBobj);
	msgInobj.join();
	msgOutobj.join();
	msgOutobj2.join();

	std::cout << "this is the main thread!" << std::endl;

	system("pause");
	return 0;
}