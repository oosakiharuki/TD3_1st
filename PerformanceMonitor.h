#pragma once
#include <chrono>
#include <deque>
#include <string>
#include <atomic>
#include <unordered_map>

class PerformanceMonitor {
public:
    static PerformanceMonitor* GetInstance();
    void Finalize();
    
    // フレーム計測開始
    void BeginFrame();
    
    // フレーム計測終了
    void EndFrame();
    
    // 特定セクションの計測開始
    void BeginSection(const std::string& sectionName);
    
    // 特定セクションの計測終了
    void EndSection(const std::string& sectionName);
    
    // パフォーマンス情報取得
    float GetFPS() const { return currentFPS; }
    float GetFrameTime() const { return currentFrameTime; }
    float GetAverageFPS() const { return averageFPS; }
    float GetAverageFrameTime() const { return averageFrameTime; }
    
    // メモリ使用量取得（MB）
    float GetMemoryUsageMB() const;
    
    // CPUコア数取得
    int GetCPUCoreCount() const;
    
    // パフォーマンス統計を文字列で取得
    std::string GetPerformanceStats() const;
    
    // デバッグ表示用
    void DrawDebugInfo();
    
    // パフォーマンスログ出力
    void LogPerformance() const;
    
    // フレームドロップ検出
    bool IsFrameDropped() const { return isFrameDropped; }
    int GetDroppedFrameCount() const { return droppedFrameCount; }
    
    // 目標FPS設定（デフォルト60）
    void SetTargetFPS(float fps) { targetFPS = fps; }
    float GetTargetFPS() const { return targetFPS; }
    
private:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
    
    static PerformanceMonitor* instance;
    
    // タイミング関連
    std::chrono::steady_clock::time_point frameStartTime;
    std::chrono::steady_clock::time_point frameEndTime;
    std::chrono::steady_clock::time_point lastFrameTime;
    
    // FPS計算用
    std::deque<float> frameTimes;
    const size_t maxFrameSamples = 60;  // 1秒分のサンプル
    
    // 現在のパフォーマンス値
    std::atomic<float> currentFPS{60.0f};
    std::atomic<float> currentFrameTime{16.67f};
    std::atomic<float> averageFPS{60.0f};
    std::atomic<float> averageFrameTime{16.67f};
    
    // フレームドロップ検出
    std::atomic<bool> isFrameDropped{false};
    std::atomic<int> droppedFrameCount{0};
    float targetFPS = 60.0f;
    
    // セクション計測用
    struct SectionData {
        std::chrono::steady_clock::time_point startTime;
        float totalTime = 0.0f;
        int callCount = 0;
    };
    std::unordered_map<std::string, SectionData> sections;
    
    // 統計更新
    void UpdateStatistics();
};
