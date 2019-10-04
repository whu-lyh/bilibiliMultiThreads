#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include <future>

/*
	1. future的其他成员函数wait_for返回一个枚举类型的变量：future_status，可以用来解决async的不确定性问题
	enum class future_status{
		ready,
		timeout,
	    deferred
	};
	使用：
		std::future<int> ret = std::async(std::launch::deferred,myThread, 10);
		std::future_status stus = ret.wait_for(std::chrono::seconds(6)); //等待子线程的时间
		timeout:表示子线程的执行时间，超过主线程等待子线程的时间设定值，就会触发timeout。但是即便如此,std::async函数模板创建的子线程依然会在主线程返回前执行完。
		ready:表示线程成功返回。
		deferred：在使用std::async函数模板创建子线程，并且第一个参数设置为std::launch::deferred时，这个deferred才会有效。同时，子线程也是等到std::future<T>obj.get()才会执行，并且是在主线程中执行。
	2. 由于future.get()只能调用一次,所以要想实现不同线程之间通过future实现数据共享，那么怎么办？使用std::shared_future<T> ，它也是个类模板
		std::shared_future<int> ret(std::move(ret_1));
		这样ret就可以反复的回去线程的返回值，顾名思义，share_future<T>是将返回值通过复制的方获取，所以很安全，可以多次使用。
	3. 	3. std::atomic 原子操作,对单个变量操作，不适用互斥量，而且效率更高
		情况：
		有两个线程，一个线程对一个变量进行读取操作，另一个线程对一个变量进去写操作，如果任由两个线程自由进行操作，最终读取到的值，可能就不是当前值，也不是写线程操作之后的值，或许是一个不可预料的中间值。
			
			需要通过互斥量mutex，来保持两个线程的有序进行，如果不加锁输出的值不可预料，不是理想的结果。
			现在提供一次新的技术，即原子操作技术，不需要加锁也能保证多个线程对同一块数据进行有序访问，
			要么访问到的是其余线程没有对这个数据进行操作前的值（即原来的值），或者是被别的线程改动后的值，而不是中间值。
		可以把原子操作理解成不需要用互斥量枷锁的技术的多线程并发编程方式，原子操作：是指在多线程操作中不会被打断的程序执行片段
		原子操作比互斥量的效率上更胜一筹，
				注意了，原子操作针对的是单个变量，而不是大段的代码段，大段的代码还是需要mutex实现。
				一般指不可分割的操作，原子操作状态：要么是完成的，要么是未完成的，不可能出现半中间状态。
		原子操作支持的变量操作：
	****		++、--，+=，-=，&=,之类的,不支持 var = var+1;之类			****
			常用于计数或者统计
	*/

//int commVar = 0;
//使用原子操作之后就变成了如下的定义方式std::atomic<int>
std::atomic<int> commVar(10);
std::atomic<bool> g_bifelse = false;
//std::mutex mtx;
void write() 
{
	for (size_t i = 0; i < 1000000; ++i) {
		//mtx.lock();
		commVar++; //原子操作，不会被打断
		//mtx.unlock();
	}
	return;
}

int myThreadproj5(int parse)
{
	std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread start.\n";
	std::cout << "parse parameter: " << parse << std::endl;

	//模拟线程正在执行一个耗时的程序
	std::chrono::milliseconds time(5000); //=5s
	std::this_thread::sleep_for(time);

	std::cout << "subThread id: " << std::this_thread::get_id() << "...subThread end.\n";
	return 5; //线程函数必须要返回值，get()会一直堵塞等待返回值
}

int myThreadproj52(std::shared_future<int> &sfparse)
{
	std::cout << "subThread2 id: " << std::this_thread::get_id() << "...subThread start.\n";
	auto result = sfparse.get(); //获取值，但是只能执行一次，因为是移动语义，所以移动完之后就没有了，再调用就为空了，
	//shared_future


	std::cout << "thread2 result" << result << std::endl;
	std::cout << "subThread2 id: " << std::this_thread::get_id() << "...subThread end.\n";
	return 5; //线程函数必须要返回值，get()会一直堵塞等待返回值
}

int main5(int argc, char const *argv[])
{
	std::cout << "main Thread id: " << std::this_thread::get_id() << "...run...\n";

	//future<T>T就是将来子线程要返回的值,deferred延迟调用
	std::future<int> retVal = std::async(std::launch::deferred, myThreadproj5, 10); //线程开始执行，虽然线程函数会延迟5s,但是不会卡在这儿
	for (size_t i = 0; i < 10; ++i)
		std::cout << "test..." << i << std::endl;

	//get得到future的返回值
	//std::cout << "subThread return value: " << retVal.get() << std::endl;  //如果线程函数没有执行结束，而是会卡在这里，因为这里需要这个线程函数的返回结果(这里必须等到5才会继续往下运行)
	std::future_status fstatus = retVal.wait_for(std::chrono::seconds(6)); //0s-->c++11的新语法，10min表示10分钟
	if (std::future_status::timeout == fstatus) //预计等待子线程1s，但是子线程可能需要很久的时间，如果执行到下面的时候还是没有执行完，就返回timeout
	{
		std::cout << "subthread hasn't finished, time out occurs" << std::endl;
	}
	else if (std::future_status::ready == fstatus) //执行到这里的时候子线程已经结束了，因此是ready
	{
		std::cout << "subthread finished, continue process" << std::endl;
		std::cout << retVal.get() << std::endl;
	}
	else if (std::future_status::deferred == fstatus) //表示async创建子线程的时候是采用std::launch::deferred的形式创建的异步任务，并不是真正的异线程。
	{
		std::cout << "subthread is called by deferred" << std::endl;
		std::cout << retVal.get() << std::endl;
	}

	std::packaged_task<int(int)> packagesmask(myThreadproj5);
	std::thread myThreadObj(std::ref(packagesmask), 1);
	myThreadObj.join();

	std::future<int> result = packagesmask.get_future();
	auto resultcopy = result.get(); //future对象只可以get一次，并且必须是在move之前才行，
									//resultcopy = result.get();
	std::shared_future <int>result_shared = std::move(result);
	//std::shared_future <int>result_shared(result.share()); //这种方式也行

	//甚至可以直接使用get_future得到的future构造shared_future对象
	//std::shared_future <int>result_shared (packagesmask.get_future());

	bool resultbool = result.valid(); //验证是否有效的成员函数
	bool result_sharedbool = result_shared.valid();


	auto result_sharedcopy = result_shared.get();
	result_sharedcopy = result_shared.get(); //shared_ future对象可以get好多次

	std::thread myThreadObj2(myThreadproj52,std::ref(result_shared));
	myThreadObj2.join();

	//atomic操作程序段
	std::thread atomicthread(write);
	std::thread atomicthread2(write);

	atomicthread.join();
	atomicthread2.join();
	std::cout << "atomic variable is " << commVar << std::endl;

	std::cout << "this is main thread" << std::endl;
	system("pause");
	return 0;
}
