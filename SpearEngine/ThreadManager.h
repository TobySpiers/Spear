#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <queue>

namespace Spear
{
	// 'Task' refers to a function distributed across any number of threads
	class TaskHandle
	{

	public:
		void Initialise(int threads);
		int RemainingThreads();
		void DecrementRemainingThreads();
		bool IsTaskComplete();
		void WaitForTaskComplete();

	private:
		std::atomic<int> m_activeThreads;
		std::shared_mutex m_mutex;
		std::condition_variable_any m_conditionTaskComplete;
	};

	class ThreadManager
	{
	public:
		NO_COPY(ThreadManager);

		using ThreadTask = std::function<int(u32 taskInstanceID)>;

		ThreadManager();
		~ThreadManager();

		void DispatchTask(const ThreadTask& task, TaskHandle* taskStatus = nullptr);
		void DispatchTaskDistributed(const ThreadTask& task, TaskHandle* taskStatus = nullptr, u32 taskInstances = THREAD_COUNT);

	private:
		static void WorkerThread(u8 threadId, ThreadManager* pThreadManager);

		struct QueuedTask
		{
			ThreadTask task;
			TaskHandle* pTaskStatus;
			u32 taskInstanceID;
		};

		static const uint32_t THREAD_COUNT{16};
		std::mutex m_mutex;
		std::queue<QueuedTask> m_queuedTasks;
		std::condition_variable m_queuedTasksUpdated;
		std::vector<std::jthread> m_threads;
		std::atomic<uint32_t> m_activeThreads;
		bool m_bKillThreads{false};
	};
}