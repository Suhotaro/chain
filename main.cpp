#include <memory>
#include <mutex>

#include "task.h"

int main() 
{
	const int size = 100;
	
	threadsafe_queue<int> q1;
	threadsafe_queue<int> q2;
	threadsafe_queue<int> q3;

	for (int i = 0; i < size; i++)
		q1.push(i);

	std::shared_ptr<Task<int> > mul(new TaskMulTwo<int>(q1, q2));
	std::shared_ptr<Task<int> > mul2(new TaskMulFore<int>(q2, q3));

	mul->start();
	mul2->start();
	std::this_thread::sleep_for (std::chrono::seconds(1));

	for (int i = 0; i < size; i++)
	{
		int tmp = 0;
		q3.wait_and_pop(tmp);		
		printf("new_val: %d\n", tmp);
	}
	
	mul->stop();
	mul2->stop();
	
	q1.push(0);
	q2.push(0);

	return 0;
}

/*
 	printf("wait: 1\n");
	std::this_thread::sleep_for (std::chrono::seconds(2));
	printf("continue: 1\n");
	
	mul->pause();
	
	printf("wait: 2\n");
	std::this_thread::sleep_for (std::chrono::seconds(4));
	printf("continue: 2\n");

	mul->start();

	printf("wait: 3\n");
	std::this_thread::sleep_for (std::chrono::seconds(2));
	printf("continue: 3\n");

	mul->pause();

	printf("wait: 4\n");
	std::this_thread::sleep_for (std::chrono::seconds(4));
	printf("continue: 4\n");

	mul->start();

	printf("wait: 5\n");
	std::this_thread::sleep_for (std::chrono::seconds(2));
	printf("continue: 5\n"); 
 */