#pragma once
#ifndef _THREAD_POOL_HPP
#define _THREAD_POOL_HPP


#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>
#include <vector>



class ThreadPool
{
public:
	typedef std::function<void()> task_func;
public:
	ThreadPool(std::size_t _pool_size);
	~ThreadPool();

	void complete();

	template<class Task>
	void post(Task task)
	{
		{ // acquire lock
			std::unique_lock<std::mutex> lock(mQueueMutex);

			// add the task
			mTasks.push_back(task_func(task));

			++mCurrentTasks;
		} // release lock

		// wake up one thread
		mQueueCondition.notify_one();
	}

	bool empty() const;
private:
	void _worker_func();

private:

	// need to keep track of threads so we can join them
	std::vector<std::thread> mWorkers;

	// the task queue
	std::deque<task_func> mTasks;

	// synchronization
	std::mutex mQueueMutex;
	std::condition_variable mQueueCondition;
	std::condition_variable mCompleteCondition;
	bool mRunning;
	std::atomic<std::size_t> mCurrentTasks;
};


#endif