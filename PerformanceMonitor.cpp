#include "PerformanceMonitor.h"
#include <Windows.h>
#include <Psapi.h>
#include <sstream>
#include <iomanip>
#include <numeric>

#pragma comment(lib, "psapi.lib")

PerformanceMonitor* PerformanceMonitor::instance = nullptr;

PerformanceMonitor* PerformanceMonitor::GetInstance() {
    if (instance == nullptr) {
        instance = new PerformanceMonitor();
    }
    return instance;
}

void PerformanceMonitor::Finalize() {
    delete instance;
    instance = nullptr;
}

PerformanceMonitor::PerformanceMonitor() {
    frameStartTime = std::chrono::steady_clock::now();
    lastFrameTime = frameStartTime;
    
    // フレームタイムバッファを初期化
    frameTimes.clear();
    frameTimes.resize(maxFrameSamples, 16.67f);  // 60FPS想定で初期化
}

void PerformanceMonitor::BeginFrame() {
    frameStartTime = std::chrono::steady_clock::now();
}

void PerformanceMonitor::EndFrame() {
    frameEndTime = std::chrono::steady_clock::now();
    
    // フレーム時間を計算（ミリ秒）
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - lastFrameTime);
    float frameTimeMs = frameDuration.count() / 1000.0f;
    
    // フレームタイムをバッファに追加
    frameTimes.push_back(frameTimeMs);
    if (frameTimes.size() > maxFrameSamples) {
        frameTimes.pop_front();
    }
    
    // 統計情報を更新
    UpdateStatistics();
    
    // フレームドロップ検出
    float targetFrameTime = 1000.0f / targetFPS;
    if (frameTimeMs > targetFrameTime * 1.5f) {  // 目標の1.5倍以上なら
        isFrameDropped = true;
        droppedFrameCount++;
    } else {
        isFrameDropped = false;
    }
    
    lastFrameTime = frameEndTime;
}

void PerformanceMonitor::BeginSection(const std::string& sectionName) {
    sections[sectionName].startTime = std::chrono::steady_clock::now();
}

void PerformanceMonitor::EndSection(const std::string& sectionName) {
    auto endTime = std::chrono::steady_clock::now();
    auto& section = sections[sectionName];
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - section.startTime);
    float timeMs = duration.count() / 1000.0f;
    
    section.totalTime += timeMs;
    section.callCount++;
}

void PerformanceMonitor::UpdateStatistics() {
    // 現在のフレーム時間とFPS
    if (!frameTimes.empty()) {
        currentFrameTime = frameTimes.back();
        currentFPS = (currentFrameTime > 0.0f) ? 1000.0f / currentFrameTime : 0.0f;
        
        // 平均値を計算
        float sumFrameTime = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f);
        averageFrameTime = sumFrameTime / frameTimes.size();
        averageFPS = (averageFrameTime > 0.0f) ? 1000.0f / averageFrameTime : 0.0f;
    }
}

float PerformanceMonitor::GetMemoryUsageMB() const {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f);
}

int PerformanceMonitor::GetCPUCoreCount() const {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

std::string PerformanceMonitor::GetPerformanceStats() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "FPS: " << currentFPS << " (avg: " << averageFPS << ")\n";
    ss << "Frame Time: " << currentFrameTime << "ms (avg: " << averageFrameTime << "ms)\n";
    ss << "Memory: " << GetMemoryUsageMB() << " MB\n";
    ss << "CPU Cores: " << GetCPUCoreCount() << "\n";
    ss << "Dropped Frames: " << droppedFrameCount << "\n";
    
    // セクション別の計測結果
    if (!sections.empty()) {
        ss << "\n--- Section Timings ---\n";
        for (const auto& section : sections) {
            const std::string& name = section.first;
            const SectionData& data = section.second;
            if (data.callCount > 0) {
                float avgTime = data.totalTime / data.callCount;
                ss << name << ": " << avgTime << "ms (avg)\n";
            }
        }
    }
    
    return ss.str();
}

void PerformanceMonitor::DrawDebugInfo() {
    // ImGuiを使用する場合はここに実装
    // 現在は未実装
}

void PerformanceMonitor::LogPerformance() const {
    static int logCounter = 0;
    logCounter++;
    
    // 60フレームごとに（約1秒ごとに）ログ出力
    if (logCounter % 60 == 0) {
        std::string stats = GetPerformanceStats();
        OutputDebugStringA("\n=== Performance Report ===\n");
        OutputDebugStringA(stats.c_str());
        OutputDebugStringA("========================\n");
    }
}
