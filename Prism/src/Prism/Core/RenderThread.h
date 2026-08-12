#pragma once

#include "Core.h"
#include "Thread.h"

#include <atomic>

namespace Prism
{
    struct RenderThreadData;

    enum class ThreadingPolicy
    {
        None = 0,
        SingleThreaded,
        MultiThreaded
    };


    class RenderThread
    {
    public:
        enum class State
        {
            Idle = 0,
            Busy,
            Kick
        };

    public:
        RenderThread(ThreadingPolicy policy);
        ~RenderThread();

        // 启动 / 终止渲染线程
        void Run();
        bool IsRunning() const { return m_IsRunning; }
        void Terminate();

        // 状态机等待 / 唤醒
        void Wait(State waitForState);
        void WaitAndSet(State waitForState, State setToState);
        void Set(State setToState);

        // 帧推进：翻转命令队列索引 + 触发渲染线程执行。
        void NextFrame();
        // 阻塞主线程直到渲染线程完成当前帧
        void BlockUntilRenderComplete();
        // 触发渲染线程开始执行
        void Kick();
        // NextFrame + Kick + BlockUntilRenderComplete，单帧完整推进
        void Pump();

        static bool IsCurrentThreadRT();

    private:
        RenderThreadData* m_Data = nullptr;
        ThreadingPolicy m_ThreadingPolicy;

        Thread m_RenderThread;
        bool m_IsRunning = false;

        std::atomic<uint32_t> m_AppThreadFrame = 0;
    };
}
