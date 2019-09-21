#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>

/************************************************************************/
/*	���ɽ��̵�ͬʱ���߳��Զ�ִ��,���߳�ִ������ˣ�����������ִ�����   */
/*		join���߳̽������������߳�û�����������ǲ��ϸ�ĳ���			*/
/*		c++11����detech()���룬���߳������̲߳���ϣ����̺߳����̷߳���	*/
/*	detach֮�����߳������߳�ʧȥ��ϵ�����߳�פ���ٺ�̨���У����߳̽���֮*/
/*	����c++����ʱ������ͷŸ��߳���ص���Դ��linux�³�Ϊ�ػ��̣߳�		*/
/*	joinable()�ж��Ƿ���Գɹ�ʹ��join��detach							*/
/************************************************************************/

#define lessen1		0		// ͨ��һ�㺯�������߳�
#define lessen2		0		// ͨ������󴴽��߳�
#define lessen3		0		// ͨ��lambda���������߳�
#define lessen4		1

// ͨ������󴴽��߳�
class Cthreadtest 
{
	/*
	�����߳��д����˸��࣬һ�����߳̽�������ʹ�����߳��е�������ͷţ����������߳����Ǹ�����һ���µĶ���
	���û�����ã�ָ��֮��Ļ����Ͳ������ʲô�������
	*/

public:

	int &m_i; //�������������ã����������߳���ʹ����detach���������߳��е�int������ǰ�ͷţ���ʹ�����߳���m_i����

	Cthreadtest (int &i) :m_i (i) 
	{
		std::cout << "���캯��ִ��" << std::endl;
	}

	Cthreadtest (const Cthreadtest &test) :m_i (test.m_i)
	{
		std::cout << "�������캯��ִ��" << std::endl;
	}

	~Cthreadtest ()
	{
		std::cout << "���캯������" << std::endl;
	}

	void operator()() //�������������������
	{
		std::cout << "sub thread operator begin here" << std::endl;
		std::cout << "m_i��ֵΪ��" << m_i << std::endl;
		std::cout << "sub thread operator end here" << std::endl;
	}
};

// ͨ��һ�㺯�������߳�
void myprint () 
{
	std::cout << "sub thread begin here" << std::endl;

	std::cout << "sub thread end here" << std::endl;
}

int main () 
{
	std::cout << "Main thread begin here" << std::endl;

	if (lessen1)
	{
		std::cout << "������ÿ���" << std::endl;

		std::thread mythreadobj (myprint); //����һ�����̣߳����̵߳���ھ���myprint������myprintִ������֮�����߳̽���
		mythreadobj.join (); //���̵߳�����֮��ʼ�������ȴ����߳̽������������ȴ����߳�

		//mythreadobj.detach (); //detachʹ���߳������Լ��Ŀ��ƣ�һ��detach���޷���join�ˡ�

		// if (mythreadobj.joinable ())

			std::cout << "�������ǣ�" << std::endl;
	}
	if (lessen2)
	{
		int i = 6;
		Cthreadtest test(i);
		std::thread mythreadobj (test);
		mythreadobj.join ();
	}
	if (lessen3)
	{
		auto mylambdafunc = []
		{
			std::cout << "sub thread begin here" << std::endl;

			std::cout << "sub thread end here" << std::endl;
		};

		std::thread mythreadobj (mylambdafunc);
		mythreadobj.join ();
	}

	system ("pause");
	return 0;
}