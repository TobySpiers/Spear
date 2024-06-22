#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <queue>

namespace Spear
{
	class ThreadManager
	{
	public:
		NO_COPY(ThreadManager);

		using ThreadTask = std::function<int(u32 taskInstanceID)>;

		ThreadManager();
		~ThreadManager();

		void DispatchTask(const ThreadTask& task);
		void DispatchTaskMulti(const ThreadTask& task, u32 taskInstances);

	private:
		static void WorkerThread(u8 threadId, ThreadManager* pThreadManager);

		struct QueuedTask
		{
			ThreadTask task;
			u32 taskInstanceID;
		};

		static const uint32_t THREAD_COUNT{7};
		std::mutex m_mutex;
		std::queue<QueuedTask> m_queuedTasks;
		std::condition_variable m_queuedTasksUpdated;
		std::vector<std::jthread> m_threads;
		std::atomic<uint32_t> m_activeThreads;
		bool m_bKillThreads{false};
	};
}