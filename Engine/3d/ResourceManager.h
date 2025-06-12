#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>

// リソースタイプ
enum class ResourceType {
    Texture,
    Model,
    Sound
};

// リソース優先度
enum class ResourcePriority {
    Essential = 0,  // 必須（ロード画面など）
    High = 1,       // 高優先度（UI、プレイヤー）
    Medium = 2,     // 中優先度（ステージ）
    Low = 3         // 低優先度（装飾など）
};

// リソース情報
struct ResourceInfo {
    std::string path;
    ResourceType type;
    ResourcePriority priority;
    size_t estimatedSize;  // 推定サイズ（バイト）
    bool isLoaded = false;
    
    bool operator<(const ResourceInfo& other) const {
        return priority > other.priority;  // 優先度が高い順
    }
};

class ResourceManager {
public:
    static ResourceManager* GetInstance();
    void Finalize();
    
    // 初期化
    void Initialize();
    
    // リソースをキューに追加
    void QueueResource(const std::string& path, ResourceType type, ResourcePriority priority = ResourcePriority::Medium);
    void QueueResources(const std::vector<std::string>& paths, ResourceType type, ResourcePriority priority = ResourcePriority::Medium);
    
    // バッチローディング開始
    void StartBatchLoading();
    
    // 単一リソースの即座ロード（同期）
    void LoadResourceImmediate(const std::string& path, ResourceType type);
    
    // 進行状況取得（0.0～1.0）
    float GetLoadingProgress() const;
    
    // ロード完了チェック
    bool IsLoadingComplete() const;
    
    // リソースが読み込まれているかチェック
    bool IsResourceLoaded(const std::string& path) const;
    
    // メモリ使用量取得
    size_t GetTotalMemoryUsage() const;
    size_t GetTextureMemoryUsage() const;
    size_t GetModelMemoryUsage() const;
    
    // 未使用リソースの解放
    void ReleaseUnusedResources();
    
    // リソースプリウォーム（GPU転送最適化）
    void PrewarmResources();
    
    // キャッシュクリア
    void ClearCache();
    
    // デバッグ情報出力
    void PrintDebugInfo() const;
    
private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    static ResourceManager* instance;
    
    // リソースローディングワーカー
    void LoadingWorker();
    
    // 個別リソースローダー
    void LoadTexture(const std::string& path);
    void LoadModel(const std::string& path);
    void LoadSound(const std::string& path);
    
    // リソースサイズ推定
    size_t EstimateResourceSize(const std::string& path, ResourceType type) const;
    
    // ロードキュー（優先度付き）
    std::priority_queue<ResourceInfo> loadQueue;
    std::mutex queueMutex;
    
    // ロード済みリソース
    std::unordered_set<std::string> loadedResources;
    std::mutex loadedMutex;
    
    // メモリ使用量追跡
    std::atomic<size_t> totalMemoryUsage{0};
    std::atomic<size_t> textureMemoryUsage{0};
    std::atomic<size_t> modelMemoryUsage{0};
    
    // ロード進行状況
    std::atomic<int> totalResourceCount{0};
    std::atomic<int> loadedResourceCount{0};
    std::atomic<bool> isLoading{false};
    
    // ワーカースレッド
    std::thread loadingThread;
    std::atomic<bool> shouldStopLoading{false};
    
    // キャッシュ管理
    struct CacheEntry {
        std::string path;
        std::chrono::steady_clock::time_point lastAccess;
        size_t size;
    };
    std::vector<CacheEntry> cacheEntries;
    const size_t maxCacheSize = 512 * 1024 * 1024; // 512MB
};
