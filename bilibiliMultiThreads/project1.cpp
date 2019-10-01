#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>

/*
|	1.������ʱ������Ϊ�̲߳���
|	����ֵ����i���ݵ����̣߳��Ǵ����̸߳��Ƶ������̣߳���ֵ���ݣ���˼�ʹ���̺߳����߳�֮��Ϊdetach�����߳̽��������߳��Կ��Լ����������
|	���ǰ��ַ���char�������߳��ǲ��õ����ã�ָ�룩��Ӧ�ĵ�ַ����ͬ�ġ�ͨ��shift+F9���Բ鿴�������ڴ��еĵ�ַ��һ��detach��϶��ر���
|	2.��ο�����ȷ��charת�������߳�myprint����
|	�ܽ�
|	������int���ּ����Ͳ���������ֱ�Ӳ���ֵ���ݵķ�ʽ
|	�������������������ʽ���ݣ��ڴ����̵߳�ʱ��͹�������ʱ����Ȼ���ں����β������������ӣ������ʹ�ÿ������캯����Σ�Ч�ʱ��
|	�ռ�����
|	������detach ʹ��join
|	�߳�id
|	id�Ƕ�Ӧ��ÿ���̵߳ģ�ÿ���̶߳�Ӧ��ͬ��id��std::this_thread::get_id()
|	3.�������������ָ����Ϊ�̲߳���
|	std::ref����
|	4.��������ָ��
|	5.�ó�Ա����ָ�����̺߳���
*/

//#define lessen4
//#define lessen5
//#define lessen6

#ifdef lessen6
class A
{
public:
	//mutable int m_i; // mutable��ʹ��const�ı���Ҳ�����޸ģ�������Ȼ�����߳��޸��˳�Ա������ֵ�����߳�Ҳ���õ������ô��Σ����Ƿ��ص����̵߳�ʱ����Ȼû���޸�ԭ���ı���
	int m_i;
	//����ת�����캯����int --> class A
	A (int a) :m_i (a) { std::cout << "constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	A (const A &a) :m_i (a.m_i) { std::cout << "copy constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	~A () { std::cout << "destory func run here!" << this << std::endl; }

	void thread_work (int num)
	{
		m_i = 0.02;
		std::cout << "sub thread member func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl;
	}

	void operator()(int num)
	{
		std::cout << "sub thread () func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl;
	}
};

void myprint2 (std::unique_ptr<int> ppp)
{
	std::cout << "sub main pA's location is: " << &ppp << "thread id: " << std::this_thread::get_id () << std::endl;
}
#endif // lessen6

#ifdef lessen5
class A
{
public:
	//mutable int m_i; // mutable��ʹ��const�ı���Ҳ�����޸ģ�������Ȼ�����߳��޸��˳�Ա������ֵ�����߳�Ҳ���õ������ô��Σ����Ƿ��ص����̵߳�ʱ����Ȼû���޸�ԭ���ı���
	int m_i;
	//����ת�����캯����int --> class A
	A (int a) :m_i (a) { std::cout << "constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	A (const A &a) :m_i (a.m_i) { std::cout << "copy constructor func run here!" << this << "thread id: " << std::this_thread::get_id () << std::endl; }
	~A () { std::cout << "destory func run here!" << this << std::endl; }
};

void myprint (const int i,const A &pA) //��������ʹ��ʲô���в������ݣ�ֵ���ݻ������ô���������ʱ�򣬱��������ǰ��տ������캯������ʽ���д���
{
	std::cout << &pA << std::endl;
}

//void myprint2 (const A &pA) //���Ｔʹ������Ҳ�޷��޸�m��ֵ(��һ�ж�Ӧmutable)
void myprint2 (A &pA)
{
	pA.m_i = 200;
	std::cout << "sub main pA's location is: " <<&pA << "thread id: " << std::this_thread::get_id () << std::endl;
}
#endif // lessen5

#ifdef lessen4
//void myprint (const int &i,char *pmychar)
/*
|	���������һЩС���޸ģ�����ͨ����char*ת��Ϊconst string��ʵ�ִ����̵߳����߳�֮������ݴ��ݣ�
|	���׳��ֵ������ǣ���������������߳��Ѿ�ִ�����ˣ�ϵͳ�ſ�ʼ��char*ת��Ϊconst string�Ŀ����ԡ�
*/
void myprint (const int &i, const std::string &pmychar)
{
	std::cout << "this is sub thread number: " << i << std::endl;
	std::cout << pmychar << std::endl;
}
#endif //lessen4

int main1 () 
{
#ifdef lessen4
	int num = 1;
	int &num2 = num;
	char tempchar[] = "this is qianqian";
	std::thread mythreadobj (myprint,num,std::string(tempchar)); // ֱ���ڴ��ݵ�ʱ��������ʱstring����
	mythreadobj.join ();
#endif //lessen4

#ifdef lessen5

	std::cout << "main thread id: " << std::this_thread::get_id () << std::endl;

	int num = 1;
	int num2 = 12;
	std::thread mythreadobj (myprint, num, A(num2)); //�����̵߳�ͬʱ������ʱ������в�������ʱ���еģ��ȫ��
	//std::thread mythreadobj2 (myprint2, A(num)); //temp A object,�����������ʽ��������myprint��ʹ��const����������invoke����

	A myAobj (10);
	std::thread mythreadobj3 (myprint2 , std::ref(myAobj)); //ʹ����std::ref�Ĳ�������������
	std::cout << myAobj.m_i << std::endl; //can't be changed
	mythreadobj.join ();
	//mythreadobj2.join ();
	mythreadobj3.join ();
#endif //lessen5

#ifdef lessen6
	std::cout << "main thread id: " << std::this_thread::get_id () << std::endl;

	std::unique_ptr<int> myunique (new int(100));
	std::thread mythreadobj (myprint2, std::move(myunique)); //ʹ����std::ref�Ĳ�������������
	//std::cout << myunique.m_i << std::endl; //can't be changed
	mythreadobj.join ();//ref������£�һ������ʹ��detach

	A myAobj(100);
	//std::thread mythreadobj2 (&A::thread_work, myAobj, 200);
	std::thread mythreadobj2 (&A::thread_work, std::ref(myAobj), 200); //���ﾹȻ����һ��invoke�Ĵ�����vs2015���棬�ٲ���һ��vs2017�ǲ���ͬ������ָô���
	mythreadobj2.join ();
	//mythreadobj2.detach ();//ref�²�����ʹ��

	A myAobj2 (100); //����ʹ������ת�����캯��
	std::thread mythreadobj3 (myAobj, 200);
	//std::thread mythreadobj3 (std::ref (myAobj), 200);//����ʱ�����ÿ������캯���ģ��������ʹ��detach�ǲ���ȫ�ġ�
	mythreadobj3.join (); //���ǰ���������ڴ��뵽���߳�ʱʹ�õ�ʱ�������캯������ʹ��detach�Ͳ���ȫ�ˣ���join���ǰ�ȫ�ġ�

#endif // lessen6

	std::cout << "this is main thread!" << std::endl;

	system ("pause");
	return 0;
}