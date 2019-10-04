#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <future>

//#define CONDITION_VARIABLE_LESSEN
#define ASYNC_FUTURE_LESSEN

#ifdef CONDITION_VARIABLE_LESSEN
/*
std::condition_variable条件变量，实际是一个类,需要和一个互斥量配对使用
notify_one不一定能够唤醒wait()因为wait（）所在线程可能正在执行某个很耗时的部分，而不是卡在wait()处
notify_one一次只会激活一个线程，如果两个线程如下面的outMsgRecvQueue函数（两个线程）中都有wiai()等待唤醒，则需要notify_all()
notify_all
*/
#endif // CONDITION_VARIABLE_LESSEN

#ifdef ASYNC_FUTURE_LESSEN
/*
	后台创建后台任务并返回值，async是函数模板，是用来启动异步任务，返回std::future类模板对象，
	future对象含有成员函数get()
	1. std::asysnc、std::future<T>：希望线程返回一个结果
		std::asysnc：函数模板，启动一个异步任务，它返回一个std::future类模板对象。
		std::future<T>:类模板，常用函数,T是返回结果的类型
			get()       //等待，直到获取子线程返回值，解除堵塞。///注意只能调用一次///,   因为get()是用移动语义实现的,使用future.get()后，再次使用future.get()将变成nullptr。
			wait()；    //等待，直到子线程结束，不需要返回值，解除堵塞
	启动一个异步任务，就是自动创建一个线程并且开始执行对应的线程入口函数，它返回一个std::future类模板对象。在这个对象里，含有线程入口函数的返回结果，即线程返回的结果，我们可以通过std::future对象的成员函数get()获取结果。
	future, 提供了一种访问异步操作结果的机制。
	2. std::asysnc，第一个参数:std::launch类型，是一个枚举类型，来实现特殊的目的：
	/// Launch code for futures
	    enum class launch
		    {
			        async = 1,
					deferred = 2
			};
		std::future<int> retVal = std::async(std::launch::deferred,&Async::myThread, &ayc, val);
		1）std::launch::deferred：//延迟调用
	the task is executed on the calling thread the first time its result is requested (lazy evaluation)。
	即，这个标志位下，子线程根本不会被创建，直到get()/wait()相应的入口函数才会被执行；没有get()/wait()入口函数就不会被执行。子线程也不会被创建
	而且，在有get()/wait()时，线程的入口函数是在调用get()/wait()所在线程执行的。==>延迟调用是不会创建子线程的，还是会再本线程（主线程）中调用mythread函数入口。
		2）std::launch::asysnc（默认使用的就是它）
	a new thread is launched to execute the task asynchronously。
	即，显式的创建一个新的线程执行线程入口函数。并且不需要get也会开始创建子线程执行子线程函数，
		3）std::launch::deferred | std::launch::asysnc
	if both the std::launch::async and std::launch::deferred flags are set in policy, it is up to the implementation whether to perform asynchronous execution or lazy evaluation.
	系统的默认的标志位，即当第一个参数不传入的时候，和显示的设置std::launch::deferred | std::launch::async效果一样,即取决于自己的实现，执行哪个。
	此时可能创建新的线程执行异步任务（std::launch::async），也有可能在调用异步任务返回值的线程里直接调用这个异步任务函数而不创建新的线程(std::launch::deferred)。因此可以配合future_status一起使用，来确定这个异步任务执行情况。
	3. 	std::packaged_task
		std::packaged_task是个类模板，模板参数是各种可调用对象，通过std::packaged_task把各种可调用对象包装起来，以作为线程入口函数。
		可调用对象指的是？函数？成员函数？
		std::packaged_task 对象将线程入口函数myThread进行封装，然后传入std::thread 对象，和普通线程一样，调用join()，当再次调用ret.get()就不会再等待，前面join()已经
		等待子线程执行结束了。ret.get()就可以直接获取值。如果不加join()，直接ret.get()，会报错。
		package_task包装起来的对象可以直接调用
	4. 	4. std::promise:类模板
		能够在某个线程中给它赋值，然后在其他线程中，把这个值取出来。std::promise<T> prom,可以作为一个线程的参数，在这个线程里进行某些运算，运算结果，
		保存在std::promise<T>对象之中，在另一个线程中，再将这个值取出来。当然，得确保这个std::promise<T>对象是同一个对象，因此需要使用引用传递参数。
		与取值有关的函数是std::future<T>obj,通过prom.get_futrue();就可以取出furture对象，然后future.get()取出这个值。
	5. 小结
		std::asysnc
		std::packaged_task,
		std::promise：更像是一个传递数据的变量，通过它在线程之间传递数据。
			他们都可以和std::future<T>配合使用，通过std::future<T>来取出线程中自己需要的值，方式有所不同。
				std::async是函数模板，函数返回值就是std::future<T>对象
				std::packaged_task和std::promise
					他们的对象有get_future方法，可以得到std::future<T>对象，再获取相应的值。其中T是获取的值类型。
					他们都需要结合std::thread 进行使用，然后别忘记 join()。
		std::asysnc
			async如果创建子线程，如果子线程还没结束，主线程任务已经执行结束，那么主线程会在结束前将子线程任务执行结束再返回。
			sync更加准确的叫法是，创建一个异步任务，但并不一定创建一个子线程，
				std::launch:deferred传入时，谁调用get就当前线程里创建异步任务，并没有创建新的子线程。
				std::launch::async传入时，强制创建子线程执行异步任务。
			std::asysnc与std::thread区别
				thread在系统资源紧张时，可能会创建线程失败，而async会强制创建一个新的线程
				thread如果想获取线程的返回值，或者一些自己需要的中间值，不容易实现。但是async返回的时std::future<T>对象，或者std::share_futute<T>对象，可以方便的获取返回值。
			可以直接使用promise在thread创建的子线程之间传递参数，async则可以通过get_future() & get()返回或捕捉线程中的参数
	*/
#endif //ASYNC_FUTURE_LESSEN


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

int main4()
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
#endif / CONDITION_VARIABLE_LESSEN

#ifdef ASYNC_FUTURE_LESSEN

class D
{
public:
	int myThread(int parse)
	{
		std::cout << parse << std::endl;
		std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread start.\n";

		//模拟线程正在执行一个耗时的程序
		std::chrono::milliseconds time(5000); //=5s
		std::this_thread::sleep_for(time);

		std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread end.\n";
		return 5; //线程函数必须要返回值，get()会一直堵塞等待返回值
	}
protected:
private:
};

int myThread(int parse) 
{
	std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread start.\n";
	std::cout << "parse parameter: " << parse << std::endl;
	//模拟线程正在执行一个耗时的程序
	std::chrono::milliseconds time(5000); //=5s
	std::this_thread::sleep_for(time);

	std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread end.\n";
	return 5; //线程函数必须要返回值，get()会一直堵塞等待返回值
}

//这个线程计算
void mythreadCalcu(std::promise<int>& prom, int val) 
{
	val++;
	//假设这个线程花了2s, 得到了运算结果
	std::chrono::milliseconds time(2000); //=5s
	std::this_thread::sleep_for(time);
	prom.set_value(val); //把val存在了prom中，以便其他的线程使用。
	return;
}

//这个线程使用上面那个线程的计算结果
void myotherThread(std::future<int>& future) 
{
	int ret = future.get();
	std::cout << "myotherThread val: " << ret << std::endl;
	return;
}

std::vector<std::packaged_task<int(int)>> mypackagetasks;

int main(int argc, char const *argv[]) 
{
	std::cout << "main Thread id: " << std::this_thread::get_id() << "...run...\n";

	//future<T>T就是将来子线程要返回的值
	std::future<int> retVal = std::async(myThread,10);//线程开始执行，虽然线程函数会延迟5s,但是不会卡在这儿
	for (size_t i = 0; i < 10; ++i) 
		std::cout << "test..." << i << std::endl;

	//get得到future的返回值
	std::cout << "subThread return value: " << retVal.get() << std::endl;  //如果线程函数没有执行结束，而是会卡在这里，因为这里需要这个线程函数的返回结果(这里必须等到5才会继续往下运行)
	
	//用类的成员函数作为子线程函数入口
	D dobj;
	int parse = 12;
	std::future<int> retVal2= std::async(&D::myThread, &dobj, parse);
	
	std::cout << "subThread return value2: " << retVal2.get() << std::endl;

	//package_tasks & future
	//std::packaged_task<int(int)> packagetask(myThread); //myThread用package_task再次封装一次

	//lambda expression to replace the normal thread function 封装seal method
	std::packaged_task<int(int)> packagetask([](int parse) {
		std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread start.\n";
		std::cout << "parse parameter: " << parse << std::endl;
		//模拟线程正在执行一个耗时的程序
		std::chrono::milliseconds time(500); //=5s
		std::this_thread::sleep_for(time);

		std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread end.\n";
		return 6; //线程函数必须要返回值，get()会一直堵塞等待返回值
	});

	mypackagetasks.emplace_back(std::move(packagetask)); //这里使用了vector容器以及move移动语义，移动之后packagetask就为空了
	std::packaged_task<int(int)> packagetask1;
	auto iter = mypackagetasks.begin();
	packagetask1 = std::move(std::move(*iter)); // move到packagetask1之后mypackagetasks的size仍然等于1，所以必须erase掉iter
	mypackagetasks.erase(iter); //后面也就不可以再用iter了
	packagetask1(520); //直接调用lambda表达式

	//std::thread trd(std::ref(packagetask), 100); //线程直接开始运行，para2为线程入口函数的参数
	//trd.join();

	std::future<int> ret = packagetask1.get_future(); //packagetask的返回值ret就包含在future对象中了
	std::cout << ret.get() << std::endl;

	//promise class tamplate function used to trans a variable
	std::promise<int> mypromisePara; //int 为保存的数据类型，通过这个mypromisePara实现两个线程的数据交互
	std::thread calculaThread(mythreadCalcu,std::ref(mypromisePara),521); //注意必须传入引用
	std::future<int> myfuture = mypromisePara.get_future();
	std::thread furtherThered(myotherThread, std::ref(myfuture));
	calculaThread.join();
	furtherThered.join();

	std::cout << "this is main thread" << std::endl;
	system("pause");
	return 0;
}

#endif //ASYNC_FUTURE_LESSEN