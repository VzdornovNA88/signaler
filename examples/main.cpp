#include <iostream>
#include <functional>

#include <function.hpp>
#include <signal.hpp>

using namespace signaler;

struct A
{
  ~A() {

    std::cout << "~A()" << std::endl;
  }

  A() {

    std::cout << "A()" << std::endl;
  }

  void foo(int a)
  {
    std::cout << "method got: " << a << std::endl;
  }

  void foo1(int a) const
  {
    std::cout << "method got const: " << a << std::endl;
    func(a+1);
  }

   function_t<void (int)> func = function_t<void (int)>::bind<A, &A::foo>(this);
};

void foo(int a)
{
  std::cout << "function got: " << a << std::endl;
}

struct B {
	unsigned char mas[256];
  void operator()(int a) const {
    std::cout << "B::operator(): " << a << std::endl;
  }
};

::signal_t<void(int)> signal_int2;

int main(int argc, char* argv[])
{
  auto d1(function_t<void (int)>::bind<foo>());
 
  std::cout << "A create start" << std::endl;
  A a;
  B b;
  auto dB(function_t<void(int)>::bind<B,&B::operator()>(&b));
  //auto d2(function_t<void (int)>::bind<A, &A::foo1>(&a));
  auto d2(function_t<void(int)>::bind(&a, &A::foo1));
  auto d3(function_t<void (int)>{foo});

  {
	  signal_t<void(int)> signalB;
	  signalB.connect(std::move(b));

	  function_t<void(int)> d7(std::move(signalB));

	  d7(444);
  }
  
  d1(1);
  d2(2);
  d3(3);

{
  int b(2);

  std::cout << "dx create start" << std::endl;
  auto dx(function_t<void ()>(
    [&,b](){std::cout << "hello world: " << b << std::endl;}));

  dx();


  auto d4 = function_t<void()>{ [&,b]() {std::cout << "hello world d4: " << b << std::endl; } };
  d4();

std::cout << "size dx: " << sizeof(dx) << std::endl;

std::cout << "1" << std::endl;

dx = [](){}; 

std::cout << "2" << std::endl;

 dx = [a]()mutable{a.foo(777);};

std::cout << "3" << std::endl;

dx();

std::cout << "4" << std::endl;

auto  dx_copy = dx;

std::cout << "5" << std::endl;

dx_copy();
dx_copy = 61651615;
if( dx_copy == 0 ) 
  std::cout << "6" << std::endl;
}

std::cout << "7" << std::endl;

  signal_t<void(std::string, int)> signal;

  // attach a slot
  signal.connect([](std::string arg1, int arg2)mutable {
      std::cout << arg1.c_str() << " " << arg2 << std::endl;
  });

  signal("The answer:", 42);


  ::signal_t<void(int)> signal_int;

  signal_int.connect<A,&A::foo>( &a );

  signal_int(333);

  ::signal_t<void(int)> signal_to_signal,signal_to_signal1,signal_to_signal2;

  
  B object_b;

  signal_to_signal1.connect( signal_to_signal );
  signal_to_signal2.connect( signal_to_signal1 );
  signal_to_signal(1111111);

  signal_to_signal2.disconnect( signal_to_signal1 );

  auto connection = signal_to_signal2.connect<foo>();
  signal_to_signal2(66666666);
  
  
  std::cout << "start copy signal" << std::endl;
  //signal_int = std::move(signal_to_signal2);
  std::cout << "end copy signal" << std::endl;

  signal_to_signal2.disconnect(connection);

  signal_int(88888888);

  std::function< void (int) > std_func = foo;

  {
  ::signal_t<void(int)> signal_int10;

  signal_int10.connect<A,&A::foo1>( &a );
  signal_t<void(int)>::connection_t* con = signal_int10.connect<A,&A::foo> ( &a );

  signal_int10(1010101010);
  //std::cout << "signal_int10 = " << con->signal_result() << std::endl;
  signal_int10.disconnect( con );
  }

  signal_t<int(int)> signal_int_int_0;
  auto connecion = signal_int_int_0.connect([](int ret) { return ret; });
  signal_int_int_0(23232323);
  std::cout << "signal_int_int_0 = " << connecion->signal_result() << std::endl;
  signal_int_int_0.disconnect(connecion);


  std::cout << "d1 = " << sizeof( d1 ) << std::endl;
  std::cout << "std_func = " << sizeof( std_func ) << std::endl;
  std::cout << "sizeof( signal_to_signal2 ) = " << sizeof( signal_to_signal2 ) << std::endl;




// function_t<void (int)> func_object(signal_int2);
// func_object(45454);

  return 0;
}