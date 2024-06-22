#include "Core.h"
#include "ThreadManager.h"

#include <thread>
#include <functional>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <shared_mutex>
#include <atomic>

namespace Spear
{
	ThreadManager::ThreadManager()
	{
		const int safeThreadCount = std::min(std::jthread::hardware_concurrency() - 1, THREAD_COUNT);
		m_threads.reserve(safeThreadCount);
		for (int i = 0; i < safeThreadCount; i++)
		{
			m_threads.push_back(std::jthread(WorkerThread, i, this));
		}
	}

	ThreadManager::~ThreadManager()
	{
		std::scoped_lock<std::mutex> lock(m_mutex);
		m_bKillThreads = true;
		m_queuedTasksUpdated.notify_all();

		// no need to call join() in destructor when using jthread
	}

	void ThreadManager::DispatchTask(const ThreadTask& task)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		m_queuedTasks.push({task, 0});

		// let a waiting thread know there is a new task available
		m_queuedTasksUpdated.notify_one();
	}

	void ThreadManager::DispatchTaskMulti(const ThreadTask& task, u32 taskInstances)
	{
		std::scoped_lock<std::mutex> lock(m_mutex);

		for(u32 i = 0; i < taskInstances; i++)
		{
			m_queuedTasks.push({ task, i });
		}

		// let all waiting threads know there are new tasks available
		m_queuedTasksUpdated.notify_all();
	}

	// Function given to managed threads: loops endlessly until shutdown, grabbing and executing functions from ThreadManager queue
	void ThreadManager::WorkerThread(u8 threadId, ThreadManager* pThreadManager)
	{
		pThreadManager->m_activeThreads++;

		while (!pThreadManager->m_bKillThreads)
		{
			QueuedTask threadTask;
			bool bClaimedJob{false};

			while(!bClaimedJob && !pThreadManager->m_bKillThreads)
			{
				std::unique_lock<std::mutex> lock(pThreadManager->m_mutex);
				if (!pThreadManager->m_queuedTasks.empty())
				{
					threadTask = pThreadManager->m_queuedTasks.front();
					pThreadManager->m_queuedTasks.pop();
					bClaimedJob = true;
				}
				else
				{
					// if queue was empty, wait until new work is posted and JobManager sends notify
					pThreadManager->m_queuedTasksUpdated.wait(lock);
				}
			}

			if (bClaimedJob)
			{
				threadTask.task(threadTask.taskInstanceID);
			}
		}

		pThreadManager->m_activeThreads--;
	}
}