#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <stdlib.h>

/*
	1�����ģʽ���
		������ά���������ܷܺ��㣬���Ǳ��˽ӹܣ��Ķ�����Ƚ�����05�����ұȽ����е�һ����<<head first>>
	2���������ģʽsingle instance model (�����ļ���д)
	3������ģʽ����������������ͽ��
		�����̴߳������������
		����������ڶ�����߳��д���������Ķ������Ǿ���Ҫgetinstance��������
	4��std::call_once()Ҳ��c++11�еĺ���ģ�壬�ڶ���������һ��������a()
		�����Ǳ�֤����aֻ�ܱ�����һ��, �߱���������������ȴ�Ȼ��������ĵ���Դ����
		��Ҫ��std::once_flag�����ǽ��ʹ�ã�once_flag��һ���ṹ
		call_once()ִ�гɹ���once_flag���Ϊ�Ե��ã������߳̾Ͳ����ٵ���a()��
		
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
		if (m_instance == NULL) //˫��������˫�ؼ�飩�����Ч��
								// A) ���m_instance ��= NULL��������϶���ʾm_instance�Ѿ�new��
								// B) ���m_instance == NULL��������m_isntanceһ��û��new�������п����Ǳ������ˣ���Ҫ�����ˣ�
		{//�������һ����ʵҲֻ��ִ��һ�Σ������ʱ��ȫ�ǣ�=null
			std::unique_lock<std::mutex> mymutex(g_mutex); // �Զ�����
			if (m_instance == NULL)
			{
				m_instance = new singleInstanceclass();
				static garbageRecycle recycle1; // ��̬�࣬������������������ִ�е�ʱ��һ�������˳�����ִ�о�̬��������������Ӷ���new�Ķ���delete��
			}
		}
#endif
		std::call_once(g_flag, createInstance); //g_flag�൱����һ����mutex
		std::cout << "call once func run done" << std::endl;
		return m_instance;
	}
	class garbageRecycle // ����Ƕ���࣬�����ͷŶ���
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
	static singleInstanceclass *m_instance; //��̬��Ա����
};

//��Ա������ʼ��������Ϊʲôָ���Ա�ĳ�ʼ��������д�ģ�ֱ���ڣ�������д���У�
singleInstanceclass *singleInstanceclass::m_instance = NULL;

//�߳���ں���
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
	singleInstanceclass *p_singleclassA = singleInstanceclass::getInstance(); //����һ�����󣬲����ظ��ࣨsingleInstanceclass�������ָ��
	singleInstanceclass *p_singleclassB = singleInstanceclass::getInstance(); //���صĶ���ָ������ͬ�ġ�
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