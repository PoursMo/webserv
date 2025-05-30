#include <string>
#include <iostream>

void exec(void (*func)())
{
    (*this.*func)();
}

class A
{
protected:
    virtual void sayName() = 0;
};

class B : public A
{
private:
    std::string name;

public:
    B(std::string name) : name(name) {}

    void sayName();
    void run();
    
};

void B::sayName()
{
    std::cout << "COUCOU " << name << std::endl;
}

void B::run()
{
	exec(this->sayName);
}

int main()
{
    B a("pipi");
    B b("prout");
    b.run();
}

