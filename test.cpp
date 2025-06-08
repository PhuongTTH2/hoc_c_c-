#include <iostream>

namespace ns_1
{
   void func()
   {
      std::cout << " call 1" << std::endl;
   }
}

namespace ns_2
{
   void func()
   {
      std::cout << " call 2" << std::endl;
   }
}

int main()
{
   ns_1::func();
   ns_2::func();

   return 0;
}

#include <iostream>

class human1
{
public:
   int height;
   int weight;
   int get_height() const
   {
      //   height++; //error
      return height;
   }
   int get_weight() const
   {
      return weight;
   }
};

int main()
{
   human1 a;
   a.height = 100;
   a.weight = 200;
   std::cout << a.get_height() << std::endl;
   std::cout << a.get_weight() << std::endl;

   return 0;
}

#include <iostream>

class Rectangle
{
private:
   int width;
   int height;

public:
   Rectangle(int w, int h) : width(w), height(h) {}

   // Phương thức hằng, không thay đổi trạng thái của đối tượng
   int get_area() const
   {
      return width * height;
   }
   void set(int w, int h)
   {
      width = w;
      height = h;
   }
   // Phương thức không hằng, có thể thay đổi trạng thái của đối tượng
   void scale(int factor)
   {
      width *= factor;
      height *= factor;
   }
};

int main()
{
   Rectangle rect(5, 10);
   rect.set(1, 2); // ok
   std::cout << "Area: " << rect.get_area() << std::endl;

   // Vì phương thức get_area được đánh dấu là const, nó có thể được gọi trên một đối tượng const
   const Rectangle const_rect(3, 6);
   // const_rect.set(1, 2);// error
   std::cout << "Area of const rectangle: " << const_rect.get_area() << std::endl;

   return 0;
}

// Polymorphism
#include <iostream>

class human
{
public:
   int height;
   float weight;
   human(int h, int w)
   {
      height = h;
      weight = w;
   }
};

int print(int x)
{
   std::cout << x << std::endl;
}

int print(float x)
{
   std::cout << x << std::endl;
}

int main()
{
   human john(180, 220);
   print(john.height);
   print(john.weight);
   return 0;
}

// Virtual Functions
#include <iostream>

class human2
{
public:
   int height;
   int weight;
   human2(int h, int w)
   {
      height = h;
      weight = w;
   }
   virtual void print_all() = 0;
};

class b : public human2
{
public:
   b(int h, int w) : human2(h, w){};
   void print_all()
   {
      std::cout << height << std::endl;
      std::cout << weight << std::endl;
   }
};
int main()
{
   b c(100, 200);
   c.print_all();
   return 0;
}

// Friend Functions
#include <iostream>

class human
{
public:
   int weight;
   human(int h)
   {
      weight = h;
   }

private:
   friend int get_weight(human h);
};

int get_weigth(human h)
{
   return h.weight;
}

int main()
{
   human c(100);
   std::cout << get_weigth(c) << std::endl;
   return 0;
}

#include <iostream>

template <class T>
T a(T n1, T n2)
{
   return n1 > n2 ? n1 : n2;
};
struct Point
{
   int x;
   int y;
};
int main()
{

   int x1 = a(1, 2);
   int x2 = a(2, 1);
   printf("%d-%d\n", x1, x2);

   Point p = {1, 2};
   auto [x, y] = p;
   printf("%d-%d", x, y);
   return 0;
}

#include <cstdio>
#include <functional>

class LibraryClass {
public:
	  void passACallbackToMe(int (*callback)(int num1, int num2)) {
	      // Now invoke (call) the callback
        int o = callback(1, 2);
        printf("Value: %i\n", o); // We might be on an embedded system, use printf() and not std::cout
	  }
};

class MyClass {
public:
      int methodToCallback(int num1, int num2) {
          return num1 + num2;
      }
};

// Global pointer to an instance of our class so the C style callback
// wrapper can invoke the callback on a particular instance (yuck!)
MyClass * myClassPtr;
int cStyleWrapper(int num1, int num2) {
    return myClassPtr->methodToCallback(num1, num2);
}

int main()
{
    MyClass myClass;
    // Make the global variable point to our new instance. Obviously, this
    // way does not scale well, as you have to make global variable and C-style
    // function for every instance (and what if you don't know how many instances you will
    // need!?!)
    myClassPtr = &myClass; 
    
    LibraryClass libraryClass;
    libraryClass.passACallbackToMe(&cStyleWrapper);
}


#include <stdio.h>
#include <functional>

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
   template <typename... Args> 
   static Ret callback(Args... args) {                    
      return func(args...);  
   }
   static std::function<Ret(Params...)> func; 
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

// C-style API which just wants a standard function for callback
void c_function_which_wants_callback(int (*func)(int num1, int num2)) {
   int o = func(1, 2);
   printf("Value: %i\n", o);
}

class ClassWithCallback {
   public:
      int method_to_callback(int num1, int num2) {
          return num1 + num2;
      }
};

typedef int (*callback_t)(int,int);

int main() {
    ClassWithCallback my_class;
    Callback<int(int,int)>::func = std::bind(&ClassWithCallback::method_to_callback, &my_class, std::placeholders::_1, std::placeholders::_2);
    callback_t func = static_cast<callback_t>(Callback<int(int,int)>::callback);

    // Now we can pass this function to a C API which just wants a standard function callback    
    c_function_which_wants_callback(func);      
}


#include <iostream>
 
struct A
{
    virtual void foo();
    void bar();
    virtual ~A();
};
 
// member functions definitions of struct A:
void A::foo() { std::cout << "A::foo();\n"; }
A::~A() { std::cout << "A::~A();\n"; }
 
struct B : A
{
//  void foo() const override; // Error: B::foo does not override A::foo
                               // (signature mismatch)
    void foo() override; // OK: B::foo overrides A::foo
//  void bar() override; // Error: A::bar is not virtual
    ~B() override; // OK: `override` can also be applied to virtual
                   // special member functions, e.g. destructors
    void override(); // OK, member function name, not a reserved keyword
};
 
// member functions definitions of struct B:
void B::foo() { std::cout << "B::foo();\n"; }
B::~B() { std::cout << "B::~B();\n"; }
void B::override() { std::cout << "B::override();\n"; }
 
int main()
{
    B b;
    b.foo();
    b.override(); // OK, invokes the member function `override()`
    int override{42}; // OK, defines an integer variable
    std::cout << "override: " << override << '\n';
}
// B::foo();
// B::override();
// override: 42
// B::~B();

//AA//bb