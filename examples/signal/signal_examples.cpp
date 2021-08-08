#include <iostream>
#include <functional>
#include <memory>
#include <thread>
#include <cstring>

#include <signal.hpp>

using namespace signaler;

struct A
{
	~A() {

		std::cout << "~A()" << std::endl;
	}

	A() noexcept {

		std::cout << "A()" << std::endl;
	}

	void foo(int a)
	{
		std::cout << "method got: " << a << std::endl;
	}

	void foo1(int a) const
	{
		std::cout << "method got const: " << a << std::endl;
		func(a + 1);
	}

	function_t<void(int)> func = function_t<void(int)>::bind<A, &A::foo>(this);

	signal_t<void(std::string_view)> signal;
};

struct B {
	unsigned char mas[256];
	int b__ = 0;

	B(int b_) : b__(b_) {};
	B() noexcept {};

	B& operator = ([[maybe_unused]]const B& s) noexcept {

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
		auto res_ = b__ + a;
		std::cout << "B::operator(): " << res_ << std::endl;
		return res_;
	}
};

void foo(std::string_view s)
{
	std::cout << "void foo(std::string_view s): " << s.data() << " ; in thread id: " << std::this_thread::get_id() << std::endl;
}


struct class_example_1 {

	std::string_view class_ctx{ " / here is class context" };

	void foo(std::string_view s)
	{
		std::cout << "void class_example_1::foo(std::string_view s): " << s.data() << class_ctx.data() << std::endl;
	}

	void foo_const(std::string_view s) const
	{
		std::cout << "void class_example_1::foo(std::string_view s) const: " << s.data() << class_ctx.data() << std::endl;
	}

	void foo_int_ref(int& i)
	{
		std::cout << "void class_example_1::foo_int_ref(int& i): " << ++i << std::endl;
	}
};



signaler::context_t<> context_worker_thread;
using object_in_worker_thread_t = signaler::object_t<&context_worker_thread>;

struct class_example_2 : object_in_worker_thread_t/*signaler::object_t<&context_worker_thread>*/ {

	std::string class_ctx{ " / here is context of class_example_2 " };

	void foo_const(std::string_view s) const
	{
		std::cout << "void class_example_2::foo(std::string_view s) const: " << s.data() << class_ctx.data() << std::endl;
	}

	int foo(int i)
	{
		std::cout << "{ 'int class_example_2::foo(int i)' } : input parameter(i) = "<< i 
			      << " in thread id: " << std::this_thread::get_id() << " with result = ";
		return ++i;
	}

	void foo_int_ref(int& i)
	{
		std::cout << "{ 'void class_example_2::foo(int& i)' } : input parameter(++i) = " << ++i
			<< " in thread id: " << std::this_thread::get_id();
	}

	void foo_B(B b)
	{
		auto res = b(2);
		std::cout << "{ 'void class_example_2::void foo_B(B b)' } : input parameter = " << res
			<< " in thread id: " << std::this_thread::get_id();
	}
};

int main([[maybe_unused]]int argc, [[maybe_unused]]char* argv[])
{
	std::cout << "Begine examples : " << std::endl;

	std::cout << "Main thread ID : " << std::this_thread::get_id() << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from global function ----------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	signal_t<void(std::string_view)> foo_string_1;
	auto connection_foo_string_1 = foo_string_1.connect<foo>();

	foo_string_1("1. signal { 'foo_string_1' ; 'foo' } signal_t connect to global function (conection created by static connect");

	std::cout << std::endl;

	auto result_foo_string_1 = connection_foo_string_1.get_result();

	std::cout << "2. Getting the result of signal foo_string_1 ---> " << result_foo_string_1.status().message().c_str() << std::endl;

	std::cout << std::endl;

	std::cout << "3. Compare other connection with the same slot as initial connection" << std::endl;

	auto other_connection_foo_string_1 = foo_string_1.connect<foo>();

	if (other_connection_foo_string_1 == connection_foo_string_1)
		std::cout << "other connection of foo_string_1 is the same as connection of this signal" << std::endl;

	std::cout << std::endl;

	std::cout << "4. Compare connection from other signal with the same slot as initial connection" << std::endl;

	signal_t<void(std::string_view)> foo_string_2;
	auto connection_foo_string_2 = foo_string_2.connect<foo>();

	if (connection_foo_string_2 != connection_foo_string_1)
		std::cout << "connection of foo_string_2 is not equal connection of foo_string_1 with the same slot" << std::endl;

	std::cout << "5. Disconnect connection from foo_string_2 signal" << std::endl;

	foo_string_2.disconnect<foo>();

	foo_string_2("signal AFTER DISCONNECT { 'foo_string_2' ; 'foo' } signal_t connect to global function (conection created by static connect)");

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------ Creating from class member -----------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	class_example_1 obj_example_1;

	signal_t<void(std::string_view)> foo_string_3;
	auto connection_foo_string_3 = foo_string_3.connect<class_example_1, &class_example_1::foo>(&obj_example_1);

	foo_string_3("1. signal { 'foo_string_3' ; 'class_example_1::foo' } signal_t connect to class function (conection created by static connect)");

	std::cout << std::endl;

	auto result_foo_string_3 = connection_foo_string_3.get_result();

	std::cout << "2. Getting the result of signal foo_string_3 ---> " << result_foo_string_3.status().message().c_str() << std::endl;

	std::cout << std::endl;

	std::cout << "3. Compare other connection with the same slot as initial connection" << std::endl;

	auto other_connection_foo_string_3 = foo_string_3.connect<class_example_1, &class_example_1::foo>(&obj_example_1);

	if (other_connection_foo_string_3 == connection_foo_string_3)
		std::cout << "other connection of foo_string_3 is the same as connection of this signal" << std::endl;

	std::cout << std::endl;

	std::cout << "4. Compare connection from other signal with the same slot as initial connection" << std::endl;

	signal_t<void(std::string_view)> foo_string_4;
	auto connection_foo_string_4 = foo_string_4.connect<class_example_1, &class_example_1::foo>(&obj_example_1);

	if (connection_foo_string_4 != connection_foo_string_3)
		std::cout << "connection of foo_string_4 is not equal connection of foo_string_3 with the same slot" << std::endl;

	std::cout << "5. Disconnect connection from foo_string_4 signal" << std::endl;

	foo_string_4.disconnect<class_example_1, &class_example_1::foo>(&obj_example_1);

	foo_string_4("signal AFTER DISCONNECT { 'foo_string_4' ; 'foo' } signal_t connect to class function (conection created by static connect");

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "--------------- Creating from class const member --------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	class_example_1 obj_example_2;

	signal_t<void(std::string_view)> foo_string_5;
	auto connection_foo_string_5 = foo_string_5.connect<class_example_1, &class_example_1::foo_const>(&obj_example_2);

	foo_string_5("1. signal { 'foo_string_5' ; 'class_example_1::foo_const' } signal_t connect to class const function (conection created by static connect)");

	std::cout << std::endl;

	auto result_foo_string_5 = connection_foo_string_5.get_result();

	std::cout << "2. Getting the result of signal foo_string_5 ---> " << result_foo_string_5.status().message().c_str() << std::endl;

	std::cout << std::endl;

	std::cout << "3. Compare other connection with the same slot as initial connection" << std::endl;

	auto other_connection_foo_string_5 = foo_string_5.connect<class_example_1, &class_example_1::foo_const>(&obj_example_2);

	if (other_connection_foo_string_5 == connection_foo_string_5)
		std::cout << "other connection of foo_string_5 is the same as connection of this signal" << std::endl;

	std::cout << std::endl;

	std::cout << "4. Compare connection from other signal with the same slot as initial connection" << std::endl;

	signal_t<void(std::string_view)> foo_string_6;
	auto connection_foo_string_6 = foo_string_6.connect<class_example_1, &class_example_1::foo_const>(&obj_example_2);

	if (connection_foo_string_6 != connection_foo_string_5)
		std::cout << "connection of foo_string_6 is not equal connection of foo_string_5 with the same slot" << std::endl;

	std::cout << "5. Disconnect connection from foo_string_6 signal" << std::endl;

	foo_string_6.disconnect<class_example_1, &class_example_1::foo_const>(&obj_example_2);

	foo_string_6("signal AFTER DISCONNECT { 'foo_string_6' ; 'foo' } signal_t connect to class const function (conection created by static connect");

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from lambda expression --------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	signal_t<void(std::string_view)> foo_string_7;
	auto connection_foo_string_7 = foo_string_7.connect([](std::string_view s) {
		foo(s);
	});

	foo_string_7("1. signal { 'foo_string_7' ; '[](std::string_view s) {foo(s);}' } signal_t connect to lambda expression without context");

	std::cout << std::endl;

	auto result_foo_string_7 = connection_foo_string_7.get_result();

	std::cout << "2. Getting the result of signal foo_string_7 ---> " << result_foo_string_7.status().message().c_str() << std::endl;

	std::cout << std::endl;

	std::cout << "3. Compare other connection with the same slot as initial connection" << std::endl;

	auto other_connection_foo_string_7 = foo_string_7.connect([](std::string_view s) {
		foo(s);
	});

	if (other_connection_foo_string_7 != connection_foo_string_7)
		std::cout << "other connection of foo_string_7 is not equal first connection of this signal" << std::endl;

	std::cout << std::endl;

	std::cout << "4. Compare connection from other signal with the same slot as initial connection" << std::endl;

	signal_t<void(std::string_view)> foo_string_8;
	auto connection_foo_string_8 = foo_string_8.connect([](std::string_view s) {
		foo(s);
	});

	if (connection_foo_string_8 != connection_foo_string_7)
		std::cout << "connection of foo_string_8 is not equal connection of foo_string_7 with the same slot" << std::endl;

	std::cout << "5. Disconnect connection from foo_string_8 signal" << std::endl;

	foo_string_8.disconnect(connection_foo_string_8);

	foo_string_8("signal AFTER DISCONNECT { 'foo_string_8' ; '[](std::string_view s) {foo(s);}' } signal_t connect to lambda expression without context");

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "--------------------------- Move semantic ---------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	// initialized by lambda expression without context
	signal_t<void(std::string_view)> foo_string_9;

	auto connection_foo_string_9 = foo_string_9.connect([](std::string_view s) {
		foo(s);
	});

	foo_string_9("1. signal { 'foo_string_9 BEFORE std::move' ; '[](std::string_view s) {foo(s);}' }");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto temp = std::move(foo_string_9);

	foo_string_9("");

	std::cout << "2. signal { 'foo_string_9 AFTER std::move' ; '[](std::string_view s) {foo(s);}' } ----> "
		<< connection_foo_string_9.get_result().status().message().c_str() << std::endl;

	temp("3. signal { 'TEMP of foo_string_9 AFTER std::move' ; '[](std::string s) {foo(s);}' }");

	std::cout << "---------------------------------------------------------------" << std::endl;

	foo_string_9 = std::move(temp);

	foo_string_9("4. signal { 'foo_string_9 AFTER std::move of TEMP' ; '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------------- Signal to signal --------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	signal_t<void(std::string_view)> foo_string_10,foo_string_10_1, foo_string_10_2, foo_string_10_3;

	// initialized by lambda expression without context
	signal_t<void(std::string_view)> foo_string_10_4;

	auto con_foo_string_10   = foo_string_10  .connect(foo_string_10_1);
	auto con_foo_string_10_1 = foo_string_10_1.connect(foo_string_10_2);
	auto con_foo_string_10_2 = foo_string_10_2.connect(foo_string_10_3);
	auto con_foo_string_10_3 = foo_string_10_3.connect(foo_string_10_4);
	auto con_foo_string_10_4 = foo_string_10_4.connect([](std::string_view s) {
		foo(s);
	});

	foo_string_10("1. signal { 'foo_string_10 -> foo_string_10_1 -> foo_string_10_2 -> foo_string_10_3 -> foo_string_10_4' ; '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;

    std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << "2. foo_string_10.disconnect(foo_string_10_1);" << std::endl;

	std::cout << std::endl;

	foo_string_10.disconnect(foo_string_10_1);

	foo_string_10("signal { 'foo_string_10 -> foo_string_10_1 -> foo_string_10_2 -> foo_string_10_3 -> foo_string_10_4' foo_string_10_1 BEFORE disconnect(); '[](std::string_view s) {foo(s);}' }");

	foo_string_10_1("signal { 'foo_string_10_1 -> foo_string_10_2 -> foo_string_10_3 -> foo_string_10_4' foo_string_10_1 AFTER disconnect(); '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;

    std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << "3. foo_string_10_1.disconnect(foo_string_10_2);" << std::endl;
	foo_string_10_1.disconnect(foo_string_10_2);

	std::cout << std::endl;

	foo_string_10_2("signal { 'foo_string_10_2 -> foo_string_10_3 -> foo_string_10_4' foo_string_10_2 AFTER disconnect(); '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;

    std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << "4. foo_string_10_2.disconnect(foo_string_10_3);" << std::endl;
	foo_string_10_2.disconnect(foo_string_10_3);

	std::cout << std::endl;

	foo_string_10_3("signal { 'foo_string_10_3 -> foo_string_10_4' foo_string_10_3 AFTER disconnect(); '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << "5. foo_string_10_3.disconnect(foo_string_10_4);" << std::endl;
	foo_string_10_3.disconnect(foo_string_10_4);

	std::cout << std::endl;

	foo_string_10_4("signal { 'foo_string_10_4' foo_string_10_4 AFTER disconnect()(); '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << "6. foo_string_10_4.disconnect();" << std::endl;
	foo_string_10_4.disconnect();

	std::cout << std::endl;

	foo_string_10_4("signal { 'AFTER diconnect() all; '[](std::string_view s) {foo(s);}' }");

	std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << "7. connect to signal like a class member." << std::endl;

	std::cout << std::endl;

	A a;

	auto con_signal_of_A = a.signal.connect([](std::string_view s) {
		foo(s);
	});

	auto con_foo_string_10_to_A =  foo_string_10.connect(&a, a.signal);

	foo_string_10("signal { 'foo_string_10 -> signal like a class member'; '[](std::string_view s) {foo(s);}' }");

	std::cout << std::endl;
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

	foo_string_11("signal { 'foo_string_11' ; 'class_example_2::foo_const' } signal_t connect to class const function (conection created by static connect)");

	std::this_thread::sleep_for(std::chrono::seconds(1));

	foo_string_11.disconnect();

	std::cout << std::endl;
	std::cout << std::endl;

    std::cout << "---------------------------------------------------------------" << std::endl;

	signal_t<int(int)> foo_int_1;
	auto connection_foo_int_1 = foo_int_1.connect<class_example_2, &class_example_2::foo>(&obj_example_3);

	foo_int_1(1);

	std::cout << connection_foo_int_1.get_result().value() << std::endl;

	std::cout << std::endl;

	std::cout << "foo_int_1.disconnect()" << std::endl;

	foo_int_1.disconnect();

	std::cout << std::endl;

	std::cout << "AFTER disconnect() ----> connection_foo_int_1.get_result().status().message() : " << connection_foo_int_1.get_result().status().message().c_str() << std::endl;

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;

	signal_t<void(int&)> foo_int_ref_1;
	auto connection_foo_int_ref_1 = foo_int_ref_1.connect<class_example_2, &class_example_2::foo_int_ref>(&obj_example_3);
	int I = 1;
	foo_int_ref_1(I);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "AFTER call foo_int_ref_1 ----> I = " << I << std::endl;

	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;

	signal_t<void(B)> foo_B_1;
	auto connection_foo_B_1 = foo_B_1.connect<class_example_2, &class_example_2::foo_B>(&obj_example_3);
	B b{1};
	foo_B_1(std::move(b));

	std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}