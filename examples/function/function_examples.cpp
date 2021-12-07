#include <iostream>
#include <functional>
#include <memory>
#include <thread>
#include <cstring>
#include <string_view>

#include <function.hpp>

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
		std::cout << "method 'foo' got: " << a << std::endl;
	}

	void foo(int a) const
	{
		std::cout << "method 'foo const' got: " << a << std::endl;
	}

	void foo1(int a) const
	{
		std::cout << "method 'foo1' got const: " << a << std::endl;
		func(a + 1);
	}

	function_t<void(int)const> func = function_t<void(int)const>::bind<A, &A::foo>(this);
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
	std::cout << "void foo(std::string_view s): " << s.data() << std::endl;
}


struct class_example_1 {

    static inline constexpr char ctx [] = " / here is class context" ;

	std::string_view class_ctx{ctx};
	//unsigned char data[8];

    void foo_for_std_bind(std::string_view s)
	{
		std::cout << "void class_example_1::foo(std::string_view s): " << s.data() << class_ctx.data() << std::endl;
	}

	void foo(std::string_view s)
	{
		std::cout << "void class_example_1::foo(std::string_view s): " << s.data() << class_ctx.data() << std::endl;
	}

    void foo(std::string_view s) const
	{
		std::cout << "void class_example_1::foo(std::string_view s) const : " << s.data() << class_ctx.data() << std::endl;
	}

	void foo(std::string_view s) const volatile
	{
		std::cout << "void class_example_1::foo(std::string_view s) const volatile : " << s.data() << ctx << std::endl;
	}

	void foo_const_lvalue_ref_qualifier_noexcept(std::string_view s) const& noexcept
	{
		std::cout << "void class_example_1::foo(std::string_view s) const& volatile noexcept: " << s.data() << ctx << std::endl;
	}

	void foo_lvalue_ref_qualifier_noexcept(std::string_view s) & noexcept
	{
		std::cout << "void class_example_1::foo(std::string_view s) & noexcept : " << s.data() << ctx << std::endl;
	}

	void foo_rvalue_ref_qualifier_noexcept(std::string_view s) && noexcept
	{
		std::cout << "void class_example_1::foo(std::string_view s) && noexcept : " << s.data() << ctx << std::endl;
	}

	void foo_const_lvalue_ref_qualifier(std::string_view s) const&
	{
		std::cout << "void class_example_1::foo(std::string_view s) const& : " << s.data() << ctx << std::endl;
	}

	void foo_lvalue_ref_qualifier(std::string_view s) &
	{
		std::cout << "void class_example_1::foo(std::string_view s) & : " << s.data() << ctx << std::endl;
	}

	void foo_rvalue_ref_qualifier(std::string_view s) &&
	{
		std::cout << "void class_example_1::foo(std::string_view s) && : " << s.data() << ctx << std::endl;
	}



	void foo_const(std::string_view s) const
	{
		std::cout << "void class_example_1::foo(std::string_view s) const: " << s.data() << class_ctx.data() << std::endl;
	}

	void foo_int_inc_by_ref(int& i)
	{
		std::cout << "void class_example_1::foo_int_ref(int& i): " << ++i << std::endl;
	}

	bool operator==(const class_example_1& t){
		return class_ctx == t.class_ctx;
	}

	void operator()(std::string_view s) const {
		std::cout << "oid operator()(std::string_view s) const : " << s.data() << " - with context: " << class_ctx.data() << std::endl;
	}


// 	class_example_1() noexcept = default;
//   ~class_example_1() noexcept = default;
//   class_example_1(class_example_1 const &s) {};
//   class_example_1 &operator=(class_example_1 const &s) {};
//   class_example_1(class_example_1 &&s) noexcept = default;
//   class_example_1 &operator=(class_example_1 &&s) noexcept = default;
};


struct class_example_2 {

    static inline constexpr char ctx [] = " / here is class context" ;

	std::string_view class_ctx{ctx};

	bool operator==(const class_example_2& t){
		std::cout << "---------------------------> bool operator==(class_example_1&& t)" << std::endl;
		return class_ctx == t.class_ctx;
	}

	void operator()(std::string_view s)  {
		std::cout << "oid operator()(std::string_view s) const : " << s.data() << " - with context: " << class_ctx.data() << std::endl;
	}
};


int main([[maybe_unused]]int argc, [[maybe_unused]]char* argv[])
{
	std::cout << "Begine examples : " << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from global function ----------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	// binding to global function 'foo' and initialized by function_t<void(std::string_view)>&&
	auto foo_string_1 ( function_t<void(std::string_view)>::bind<foo>() );

	foo_string_1("call { 'foo_string_1' ; 'foo' } function_t from global function (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// binding to global function 'foo' and initialized by function_t<void(std::string_view)>&&
	auto foo_string_2 = function_t<void(std::string_view)>::bind<foo>();

	foo_string_2("call { 'foo_string_2' ; 'foo' } function_t from global function (created by static bind function and copy assignable operator)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// creating from global function 'foo' and initialized by function_t<void(std::string_view)>&&
	auto foo_string_3(function_t<void(std::string_view)>{foo});

	foo_string_3("call { 'foo_string_3' ; 'foo' } function_t from global function (created by constructor of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// initialized by global value (foo)
	function_t<void(std::string_view)> foo_string_4(foo);

	foo_string_4("call { 'foo_string_4' ; 'foo' } function_t from global function (created by constructor of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// initialized by default value (nullptr)
	function_t<void(std::string_view)> foo_string_5; 
	// initialized by global value (foo)
	foo_string_5 = foo;

	foo_string_5("call { 'foo_string_5' ; 'foo' } function_t from global function (created by assignable operator of callable object)");

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------- Creating from lambda expression --------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	// initialized by lambda expression without context
	function_t<void(std::string_view)> foo_string_6 ( [](std::string_view s) {
		foo(s);
	} );

	foo_string_6("call { 'foo_string_6' ; '[](std::string_view s) {foo(s);}' } function_t from lambda expression without context (created by constructor of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// initialized by default value (nullptr)
	function_t<void(std::string_view)> foo_string_7;
	// initialized by lambda expression without context
	foo_string_7 = [](std::string_view s) { foo(s); };

	foo_string_7("call { 'foo_string_7' ; '[](std::string_view s) {foo(s);}' } function_t from lambda expression without context (created by assignable operator of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// initialized by default value (nullptr)
	function_t<void(std::string_view)> foo_string_8;

	{
		std::string ctx = " / here is lambda context";
		auto lambda = [ctx](std::string_view s) {
			std::string arg_{s.data()} ;
			auto c_str_ = (arg_ + ctx);
			foo(c_str_.c_str());
		};

		foo_string_8 = lambda;
	}

	foo_string_8("call { 'foo_string_8' ; '[ctx](std::string_view s) {foo(s);}' } function_t from lambda expression with context (initialized by assignable operator of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------ Creating from class member -----------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	// static binding to class function 'foo' and initialized by function_t<void(std::string_view)>&&

	class_example_1 obj_example_1;
	auto foo_string_9(function_t<void(std::string_view)>::bind<class_example_1,&class_example_1::foo>(obj_example_1));

	foo_string_9("call { 'foo_string_9' ; 'class_example_1::foo' } function_t from class function (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// static binding to class function 'foo_const' and initialized by function_t<void(std::string_view)>&&
	auto foo_string_10(function_t<void(std::string_view)const>::bind<class_example_1, &class_example_1::foo_const>(obj_example_1));

	foo_string_10("call { 'foo_string_10' ; 'class_example_1::foo_const' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto foo_string_10_1(function_t<void(std::string_view)const>::bind<class_example_1, &class_example_1::foo>(obj_example_1));

	foo_string_10_1("call { 'foo_string_10_1' ; 'class_example_1::foo const' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto foo_string_10_2(function_t<void(std::string_view)const volatile>::bind<class_example_1, &class_example_1::foo>(obj_example_1));

	foo_string_10_2("call { 'foo_string_10_2' ; 'class_example_1::foo const volatile' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto foo_string_10_3(function_t<void(std::string_view)const&>::bind<class_example_1, &class_example_1::foo_const_lvalue_ref_qualifier>(obj_example_1));

	foo_string_10_3("call { 'foo_string_10_3' ; 'class_example_1::foo_const_lvalue_ref_qualifier const&' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto foo_string_10_4(function_t<void(std::string_view)const& noexcept>::bind<class_example_1, &class_example_1::foo_const_lvalue_ref_qualifier_noexcept>(obj_example_1));

	foo_string_10_4("call { 'foo_string_10_4' ; 'class_example_1::foo_const_lvalue_ref_qualifier_noexcept const& noexcept' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto foo_string_10_5(function_t<void(std::string_view)& noexcept>::bind<class_example_1, &class_example_1::foo_lvalue_ref_qualifier_noexcept>(obj_example_1));

	foo_string_10_5("call { 'foo_string_10_5' ; 'class_example_1::foo_lvalue_ref_qualifier_noexcept & noexcept' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

    auto foo_string_10_6(function_t<void(std::string_view)&>::bind<class_example_1, &class_example_1::foo_lvalue_ref_qualifier>(obj_example_1));

	foo_string_10_6("call { 'foo_string_10_6' ; 'class_example_1::foo_lvalue_ref_qualifier &' } function_t from class function const (created by static bind function and copy constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	//! auto foo_string_10_6(function_t<void(std::string_view)&& noexcept>::bind<class_example_1, &class_example_1::foo_rvalue_ref_qualifier_noexcept>(obj_example_1));

	//! foo_string_10_6("call { 'foo_string_10_6' ; 'class_example_1::foo_rvalue_ref_qualifier_noexcept && noexcept' } function_t from class function const (created by static bind function and copy constructor)");

	//! std::cout << "---------------------------------------------------------------" << std::endl;
	
	//! dynamic binding to class function 'foo' and initialized by function_t<void(std::string_view)>&&
	auto foo_string_11 = function_t<void(std::string_view)>::bind(obj_example_1, &class_example_1::foo);

	foo_string_11("call { 'foo_string_11' ; 'class_example_1::foo' } function_t from class function (created by dynamic bind function and copy assignable operator)");

	std::cout << "---------------------------------------------------------------" << std::endl;
	
	// dynamic binding to class function 'foo_const' and initialized by function_t<void(std::string_view)>&&
	auto foo_string_12 = function_t<void(std::string_view)const>::bind(obj_example_1, &class_example_1::foo_const);

	foo_string_12("call { 'foo_string_12' ; 'class_example_1::foo_const' } function_t from class function const (created by dynamic bind function and copy assignable operator)");

	std::cout << "---------------------------------------------------------------" << std::endl;
	
	// construct to class function 'foo'
	function_t<void(std::string_view)> foo_string_11_construct(obj_example_1, &class_example_1::foo);

	foo_string_11_construct("call { 'foo_string_11_construct' ; 'class_example_1::foo' } function_t from class function (created by constructor)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	// construct to class function 'foo_const'
	class_example_1* obj = nullptr/*&obj_example_1*/;
	function_t<void(std::string_view)const> foo_string_12_construct (obj, &class_example_1::foo_const);

	auto result_of_foo_string_12_construct = foo_string_12_construct("call { 'foo_string_12_construct' ; 'class_example_1::foo_const' } function_t from class function const (created by constructor)");

	std::cout << "call { 'foo_string_12_construct' ; 'class_example_1::foo_const' } function_t from class function const (created by constructor) ----> "
		<< result_of_foo_string_12_construct.status().message().c_str() << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------------- Creating from nullptr ------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	// dynamic binding to nullptr and initialized by function_t<void(std::string_view)>&&

	auto foo_string_13(function_t<void(std::string_view)>::bind(nullptr));

	auto result_foo_string_13 = foo_string_13("");

	std::cout << "call { 'foo_string_13' ; 'nullptr' } function_t from nullptr (created by dynamic bind function and copy constructor) ----> " 
		<< result_foo_string_13.status().message().c_str() << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;
	
	function_t<void(std::string_view)> foo_string_14 = 0;

	auto result_foo_string_14 = foo_string_14("");

	std::cout << "call { 'foo_string_14' ; '0' } function_t from '0' (created by dynamic bind function and copy constructor) ----> "
		<< result_foo_string_14.status().message().c_str() << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;
	
	function_t<void(std::string_view)> foo_string_15;

	auto result_foo_string_15 = foo_string_15("");

	std::cout << "call { 'foo_string_15' ; 'default(nullptr)' } function_t from 'default(nullptr)' (created by default constructor) ----> "
		<< result_foo_string_15.status().message().c_str() << std::endl;

	std::cout << "---------------------------------------------------------------" << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "--------------------------- Move semantic ---------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	// initialized by lambda expression without context
	function_t<void(std::string_view)> foo_string_16([](std::string_view s) {
		foo(s);
	});

	foo_string_16("1. call { 'foo_string_16 BEFORE std::move' ; '[](std::string_view s) {foo(s);}' } function_t from lambda expression without context (created by constructor of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	auto temp = std::move(foo_string_16);

	auto result_foo_string_16 = foo_string_16("");

	std::cout << "2. call { 'foo_string_16 AFTER std::move' ; '[](std::string_view s) {foo(s);}' } function_t from lambda expression without context (created by constructor of callable object) ----> "
		<< result_foo_string_16.status().message().c_str() << std::endl;

	temp("3. call { 'TEMP of foo_string_16 AFTER std::move' ; '[](std::string_view s) {foo(s);}' } function_t from lambda expression without context (created by constructor of callable object)");

	std::cout << "---------------------------------------------------------------" << std::endl;

	foo_string_16 = std::move(temp);

	foo_string_16("4. call { 'foo_string_16 AFTER std::move of TEMP' ; '[](std::string_view s) {foo(s);}' } function_t from lambda expression without context (created by constructor of callable object)");

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "---------------------------- Comporators ----------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	auto foo_string_17 = function_t<void(std::string_view)>::bind<foo>();
	auto foo_string_18 = function_t<void(std::string_view)>::bind<foo>();

	if(foo_string_17 == foo_string_18)
		std::cout << "1. foo_string_17 == foo_string_18 with ( void foo(std::string_view s) )" << std::endl;

	foo_string_18 = foo_string_17;

	if (foo_string_17 == foo_string_18)
		std::cout << "2. foo_string_17 == foo_string_18 with ( void foo(std::string_view s) ) after assigning 'foo_string_18 = foo_string_17' " << std::endl;

	foo_string_18 = nullptr;

	if (foo_string_17 != foo_string_18)
		std::cout << "3. foo_string_17 != foo_string_18 with ( nullptr )" << std::endl;

	if (foo_string_18 == nullptr)
		std::cout << "4. foo_string_18 == nullptr" << std::endl;

	if (foo_string_17 != nullptr)
		std::cout << "5. foo_string_17 != nullptr with ( void foo(std::string_view s) )" << std::endl;

	foo_string_17 = foo_string_18;

	if (foo_string_17 == foo_string_18)
		std::cout << "6. foo_string_17 == foo_string_18 with ( nullptr )" << std::endl;

	foo_string_17 = [](std::string_view s) { foo(s); };
	foo_string_18 = [](std::string_view s) { foo(s); };

	if (foo_string_17 != foo_string_18)
		std::cout << "7. foo_string_17 != foo_string_18 with ( [](std::string_view s) { foo(s); }; )" << std::endl;

	auto lambda = [](std::string_view s) { foo(s); };

	foo_string_17 = lambda;
	foo_string_18 = lambda;

	if (foo_string_17 != foo_string_18)
		std::cout << "8. foo_string_17 != foo_string_18 with ( lambda )" << std::endl;

    foo_string_17 = class_example_1{};
	foo_string_18 = class_example_1{};

	if (foo_string_17 == foo_string_18)
		std::cout << "9. foo_string_17 == foo_string_18 with ( class_example_1{} )" << std::endl;

	foo_string_17 = class_example_1{};
	foo_string_18 = foo_string_17;

	if (foo_string_17 == foo_string_18)
		std::cout << "10. foo_string_17 == foo_string_18 with ( class_example_1{} ) after assigning 'foo_string_18 = foo_string_17'" << std::endl;

	foo_string_17 = class_example_1{};
	foo_string_18 = class_example_2{};

	if (foo_string_17 != foo_string_18)
		std::cout << "11. foo_string_17 != foo_string_18 with ( class_example_2{} )" << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------------- sizeof function_t -------------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	function_t<void(std::string_view)> foo_string_19 = [](std::string_view s) { foo(s); };

	std::cout << "sizeof(foo_string_19) = " << sizeof(foo_string_19) << std::endl;

	function_t<void(std::string_view),40> foo_string_20 = [](std::string_view s) { foo(s); };

	std::cout << "sizeof(foo_string_20) = " << sizeof(foo_string_20) << std::endl;

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "------------------- function_t with std::function -------------------" << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	std::function<void(std::string_view)> std_function_foo_string_21 = [](std::string_view s) { foo(s); };

	std_function_foo_string_21("call{ 'std_function_foo_string_21'; '[](std::string_view s) {foo(s);}' } std::function from lambda expression without context");

	std::cout << "---------------------------------------------------------------" << std::endl;

	function_t<void(std::string_view)> foo_string_21 = std_function_foo_string_21;

	foo_string_21("call{ 'foo_string_21'; 'std::function' } function_t from std::function");

	std::cout << "---------------------------------------------------------------" << std::endl;

	std_function_foo_string_21 = foo_string_21;

	std_function_foo_string_21("call{ 'std_function_foo_string_21'; 'function_t(foo_string_21)' } std::function from function_t");

	std::cout << "---------------------------------------------------------------" << std::endl;

	foo_string_21("AFTER call{ 'foo_string_21'; 'std::function' } function_t from std::function");

	std::cout << "---------------------------------------------------------------" << std::endl;

	using std::placeholders::_1;

	foo_string_21 = std::bind ( &class_example_1::foo_for_std_bind, obj_example_1, _1 );

	foo_string_21("call{ 'foo_string_21'; 'std::function from std::bind with &class_example_1::foo_for_std_bind' } function_t from std::function");

	std::cout << "---------------------------------------------------------------" << std::endl;

	std_function_foo_string_21 = function_t<void(std::string_view)>::bind(obj_example_1, &class_example_1::foo);

	std_function_foo_string_21("call{ 'std_function_foo_string_21'; 'function_t from function_t<void(std::string_view)>::bind with &class_example_1::foo' } std::function from function_t");

	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "Multi-Level nesting of combinations of function_t with std::function"  << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;

	std::function<void(std::string_view)> std_function_foo_string_22 = [](std::string_view s) { foo(s); };

	function_t<void(std::string_view)> foo_string_22 = std_function_foo_string_22;

	std::function<void(std::string_view)> std_function_foo_string_23 = foo_string_22;

	function_t<void(std::string_view)> foo_string_23 = std_function_foo_string_23;

	std::function<void(std::string_view)> std_function_foo_string_24 = foo_string_23;

	function_t<void(std::string_view)> foo_string_24 = std_function_foo_string_24;

	std::function<void(std::string_view)> std_function_foo_string_25 = foo_string_24;

	function_t<void(std::string_view)> foo_string_25 = std_function_foo_string_25;

	foo_string_25("call{ 'foo_string_25'; 'function_t from combination of std::function and function_t' } function_t from std::function");

    std::cout << std::endl;
	std::cout << std::endl;

    std::cout << "---------------------------------------------------------------------" << std::endl;
	std::cout << "- function_t binding to method of class with arg of reference type -"  << std::endl;
	std::cout << "---------------------------------------------------------------------" << std::endl;

	std::cout << std::endl;
	
	auto foo_int_ref_1 = function_t<void(int&)>::bind<class_example_1, &class_example_1::foo_int_inc_by_ref>(obj_example_1);
	int I = 2;
	foo_int_ref_1(I);

	std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}