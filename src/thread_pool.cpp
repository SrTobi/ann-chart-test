#include <cassert>
#include "thread_pool.hpp"



ThreadPool::ThreadPool( std::size_t _pool_size )
	: mRunning(true)
	, mCurrentTasks(0)
{
	while(_pool_size--)
	{
		mWorkers.push_back(std::thread(std::bind(&ThreadPool::_worker_func, this)));
	}
}

ThreadPool::~ThreadPool()
{
	mRunning = false;
	for(std::thread& thr : mWorkers)
	{
		mQueueCondition.notify_all();
		thr.join();
	}
}


void ThreadPool::complete()
{
	std::unique_lock<std::mutex> lock(mQueueMutex);
	while(!empty())
	{
		mCompleteCondition.wait(lock);
	}
}

bool ThreadPool::empty() const
{
	return mCurrentTasks == 0;
}


void ThreadPool::_worker_func()
{
	while(mRunning)
	{
		task_func task;
		{
			std::unique_lock<std::mutex> lock(mQueueMutex);
			while(mTasks.empty())
			{
				mQueueCondition.wait(lock);
				if(!mRunning)
					return;
			}
			task = mTasks.front();
			mTasks.pop_front();
		}

		task();

		assert(mCurrentTasks > 0);
		--mCurrentTasks;

		if(mCurrentTasks == 0)
			mCompleteCondition.notify_all();
	}
}
