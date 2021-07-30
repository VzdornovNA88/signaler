#include <iostream>
#include <functional>
#include <memory>
#include <thread>
#include <cstring>

#include <function.hpp>
#include <signal.hpp>

#include <chrono>
#include <numeric>
#include <iomanip>

using namespace std::chrono;
using namespace signaler;

struct A {
	virtual void foo_virtual(std::string_view  s) = 0;
};

struct B : A {
	unsigned char mas[256];
	int b__ = 0;

	B(int b_) : b__(b_) {};
	B() noexcept {};

	B& operator = (const B& s) noexcept {

		std::cout << "-----------------------> B& operator = (const B& s) noexcept: " << std::endl;

		return *this;
	}

	B(const B& s) noexcept {

		b__ = s.b__;
		std::cout << "-----------------------> B(const B& s): " << std::endl;
	}

	B& operator = (B&& s) noexcept {

		b__ = s.b__;
		std::cout << "-----------------------> B& operator = (B&& s) noexcept: " << std::endl;

		return *this;
	}

	B(B&& s) noexcept {

		b__ = s.b__;
		std::cout << "-----------------------> B(B&& s): " << std::endl;
	}

	int operator()(int a) const {
		/*auto res_ = b__ + a;
		std::cout << "B::operator(): " << res_ << std::endl;*/
		return /*res_*/0;
	}

	virtual void foo_virtual(std::string_view) {};
};

void foo(std::string_view s)
{
	//std::cout << s.c_str() << " in thread id: " << std::this_thread::get_id();
}


struct class_example_1 {

	std::string_view class_ctx{ " / here is class context" };

	void foo(std::string_view s)
	{
		//::foo(s + class_ctx);
	}

	void foo_const(std::string_view s) const
	{
		//::foo(s + class_ctx + " const ");
	}
};



signaler::context_t<> context_worker_thread;
//using object_in_worker_thread_t = signaler::object_t<&context_worker_thread>;

struct class_example_2 : /*object_in_worker_thread_t*/signaler::object_t<&context_worker_thread> {

	std::string_view class_ctx{ " / here is context of class_example_2 " };

	void foo_const(std::string_view s) const
	{
		//::foo(s + class_ctx + " const ");
	}

	int foo(int i)
	{
		/*std::cout << "{ 'int class_example_2::foo(int i)' } : input parameter(i) = "<< i 
			      << " in thread id: " << std::this_thread::get_id() << " with result = ";*/
		return ++i;
	}

	void foo_int_ref(int& i)
	{
		/*std::cout << "{ 'void class_example_2::foo(int& i)' } : input parameter(++i) = " << ++i
			<< " in thread id: " << std::this_thread::get_id();*/
	}

	void foo_B(B b)
	{
		//auto res = b(2);
		/*std::cout << "{ 'void class_example_2::void foo_B(B b)' } : input parameter = " << res
			<< " in thread id: " << std::this_thread::get_id();*/
	}
};


#define SHOW_TIME_BEFORE_OP(time) time = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());  \
std::cout << std::endl;  \
std::cout << "TIME(ns) BEFORE emit signal ----> " << time.count() << std::endl;  \
std::cout << std::endl

#define SHOW_TIME_AFTER_OP(time) time = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());  \
std::cout << std::endl;  \
std::cout << "TIME(ns) AFTER emit signal ----> " << time.count() << std::endl;  \
std::cout << std::endl

#define SHOW_DURATION_OP(name,op,acc,cnt) for (int i = 0; i < cnt; i++) {\
\
		nanoseconds ns_time_before;\
		nanoseconds ns_time_after;\
\
		ns_time_before = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());\
		op;\
		ns_time_after = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());\
\
		acc.push_back((ns_time_after - ns_time_before).count());\
}\
std::cout << std::fixed << std::setprecision(9) << std::left;\
std::cout << name << " : " << "duration(ns) ---> " << ((unsigned long long)std::accumulate(acc_op.begin(), acc_op.end(), (unsigned long long)0)) / cnt << std::endl;\
acc.clear();\
std::cout << std::endl


int main(int argc, char* argv[])
{
	
	const size_t count_op = 1000000;
	std::vector<unsigned long long> acc_op;
	acc_op.reserve(count_op);

	std::cout << "Begine micro benchmarks : " << std::endl;

	std::cout << "Main thread ID : " << std::this_thread::get_id() << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	SHOW_DURATION_OP("sleep_for(...)", std::this_thread::sleep_for(std::chrono::seconds(2)), acc_op, 2);

	std::cout << std::endl;

	SHOW_DURATION_OP("foo(...)", foo("call"), acc_op, count_op);

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from global function ----------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	signal_t<void(std::string_view)> foo_string_1;
	foo_string_1.connect<foo>();

	SHOW_DURATION_OP("signal emit for ----> foo_string_1(...)", foo_string_1("call"); , acc_op, count_op);
	
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------ Creating from class member -----------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	class_example_1 obj_example_1;

	signal_t<void(std::string_view)> foo_string_2;
	foo_string_2.connect<class_example_1, &class_example_1::foo>(&obj_example_1);

	SHOW_DURATION_OP("signal emit for ----> foo_string_2(...)", foo_string_2("call");, acc_op, count_op);

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "--------------- Creating from class const member --------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	class_example_1 obj_example_2;

	signal_t<void(std::string_view)> foo_string_3;
	foo_string_3.connect<class_example_1, &class_example_1::foo_const>(&obj_example_2);

	SHOW_DURATION_OP("signal emit for ----> foo_string_3(...)", foo_string_3("call"); , acc_op, count_op);

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from class virtual member -----------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	B b;

	signal_t<void(std::string_view s)> foo_void_1;
	foo_void_1.connect<B, &B::foo_virtual>(&b);

	SHOW_DURATION_OP("signal emit for virtual method ----> foo_void_1(...)", foo_void_1("call");, acc_op, count_op);

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from lambda expression --------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	signal_t<void(std::string_view)> foo_string_4;
	foo_string_4.connect( [](std::string_view s){} );

	SHOW_DURATION_OP("signal emit for ----> foo_string_4(...)", foo_string_4("call");, acc_op, count_op);

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------------- Signal to signal --------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	signal_t<void(std::string_view)> foo_string_10,foo_string_10_1, foo_string_10_2, foo_string_10_3;

	// initialized by lambda expression without context
	signal_t<void(std::string_view)> foo_string_10_4;

	foo_string_10  .connect(foo_string_10_1);
	foo_string_10_1.connect(foo_string_10_2);
	foo_string_10_2.connect(foo_string_10_3);
	foo_string_10_3.connect(foo_string_10_4);
	foo_string_10_4.connect([](std::string_view s) {});

	SHOW_DURATION_OP("signal emit for ----> foo_string_10(...)", foo_string_10("call"); , acc_op, count_op);

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------ Signal to slot from object on other thread -------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;
	
	std::thread worker_thread ([]{ context_worker_thread(); });

	worker_thread.detach();

	class_example_2 obj_example_3;

	signal_t<void(std::string_view)> foo_string_11;
	auto connection_foo_string_11 = foo_string_11.connect<class_example_2, &class_example_2::foo_const>(&obj_example_3);

	SHOW_DURATION_OP("signal emit for ----> foo_string_11(...)", foo_string_11("call"), acc_op, count_op);

	std::this_thread::sleep_for(std::chrono::seconds(5));

	std::cout << std::endl;
	std::cout << std::endl;

	signal_t<int(int)> foo_int_1;
	auto connection_foo_int_1 = foo_int_1.connect<class_example_2, &class_example_2::foo>(&obj_example_3);

	SHOW_DURATION_OP("signal emit for ----> foo_int_1(...)", foo_int_1(1) , acc_op, count_op);

	std::cout << connection_foo_int_1.get_result().value() << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}