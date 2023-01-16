#include "reload/application.h"
#include <iostream>
#include <array>

class service {

public:
	template <class T> service(T&& obj) : object(std::make_unique<Model<T>>(std::forward<T>(obj))) {}

	void test() { object->test(); }
	void start() const { object->start(); object->test(); }
	void stop() const { object->stop(); }

	struct Concept {                                         
		virtual ~Concept() {}
		virtual void test() = 0;
		virtual void start() const = 0;
		virtual void stop() const = 0;
	};

	template< typename T >
	struct Model : Concept 
	{
		Model(const T& t) : object(t) {}

		void test() override {
			object.test();
		}

		void start() const override {
			object.start();
		}

		void stop() const override {
			object.stop();
		}

	private:
		T object;
	};

	std::unique_ptr<Concept> object;
};


struct sound 
{
	void test() { a = 10; }
	void start() const { printf("network::start %i\n", a); }
	void stop() const {printf("network::stop \n"); }

	int a;
};

struct network
{
	void test() { b = 20; }
	void start() const { printf("network::start %i\n", b); }
	void stop() const {printf("network::stop \n"); }
	int b;
};

int main(int argc, char** argv)
{

	std::array<service, 2> services{ service(sound()), service(network()) };

	for (auto service : services)
	{
		service.test();
		service.start();
		service.stop();
	}


	return ;
}