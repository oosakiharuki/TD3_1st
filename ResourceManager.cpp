#include "ResourceManager.h"
#include "Engine/2d/TextureManager.h"
#include "Engine/3d/ModelManager.h"
#include "Engine/audio/Audio.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

ResourceManager* ResourceManager::instance = nullptr;

ResourceManager* ResourceManager::GetInstance() {
    if (instance == nullptr) {
        instance = new ResourceManager();
    }
    return instance;
}

void ResourceManager::Finalize() {
    if (instance) {
        instance->shouldStopLoading = true;
        if (instance->loadingThread.joinable()) {
            instance->loadingThread.join();
        }
        delete instance;
        instance = nullptr;
    }
}

void ResourceManager::Initialize() {
    // リソースマネージャーの初期化
    totalResourceCount = 0;
    loadedResourceCount = 0;
    isLoading = false;
    shouldStopLoading = false;
    
    totalMemoryUsage = 0;
    textureMemoryUsage = 0;
    modelMemoryUsage = 0;
    
    OutputDebugStringA("ResourceManager: 初期化完了\n");
}

void ResourceManager::QueueResource(const std::string& path, ResourceType type, ResourcePriority priority) {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    // 既にロード済みまたはキューに存在する場合はスキップ
    if (IsResourceLoaded(path)) {
        return;
    }
    
    ResourceInfo info;
    info.path = path;
    info.type = type;
    info.priority = priority;
    info.estimatedSize = EstimateResourceSize(path, type);
    info.isLoaded = false;
    
    loadQueue.push(info);
    totalResourceCount++;
    
    char debugMsg[256];
    sprintf_s(debugMsg, "ResourceManager: リソースをキューに追加 [%s] Priority=%d\n", 
              path.c_str(), static_cast<int>(priority));
    OutputDebugStringA(debugMsg);
}

void ResourceManager::QueueResources(const std::vector<std::string>& paths, ResourceType type, ResourcePriority priority) {
    for (const auto& path : paths) {
        QueueResource(path, type, priority);
    }
}

void ResourceManager::StartBatchLoading() {
    if (isLoading) {
        OutputDebugStringA("ResourceManager: 既にローディング中です\n");
        return;
    }
    
    isLoading = true;
    shouldStopLoading = false;
    
    // ワーカースレッドでローディング開始
    loadingThread = std::thread(&ResourceManager::LoadingWorker, this);
    
    OutputDebugStringA("ResourceManager: バッチローディング開始\n");
}

void ResourceManager::LoadingWorker() {
    while (!shouldStopLoading) {
        ResourceInfo info;
        
        // キューから次のリソースを取得
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (loadQueue.empty()) {
                break;  // キューが空になったら終了
            }
            
            info = loadQueue.top();
            loadQueue.pop();
        }
        
        // リソースタイプに応じてロード
        try {
            switch (info.type) {
                case ResourceType::Texture:
                    LoadTexture(info.path);
                    break;
                case ResourceType::Model:
                    LoadModel(info.path);
                    break;
                case ResourceType::Sound:
                    LoadSound(info.path);
                    break;
            }
            
            // ロード完了をマーク
            {
                std::lock_guard<std::mutex> lock(loadedMutex);
                loadedResources.insert(info.path);
            }
            
            loadedResourceCount++;
            
            char debugMsg[256];
            sprintf_s(debugMsg, "ResourceManager: ロード完了 [%s] (%d/%d)\n", 
                      info.path.c_str(), loadedResourceCount.load(), totalResourceCount.load());
            OutputDebugStringA(debugMsg);
            
        } catch (std::exception& e) {
            char errorMsg[512];
            sprintf_s(errorMsg, "ResourceManager: ロードエラー [%s] - %s\n", 
                      info.path.c_str(), e.what());
            OutputDebugStringA(errorMsg);
        }
        
        // CPUの負荷を下げるために短い休止（優先度に応じて調整）
        if (info.priority == ResourcePriority::Low) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    
    isLoading = false;
    OutputDebugStringA("ResourceManager: バッチローディング完了\n");
}

void ResourceManager::LoadTexture(const std::string& path) {
    // フルパスを構築
    std::string fullPath = path;
    if (path.find("resource/") == std::string::npos) {
        fullPath = "resource/Sprite/" + path;
    }
    
    // TextureManagerを使用してロード
    if (!TextureManager::GetInstance()->CheckTextureExist(fullPath)) {
        TextureManager::GetInstance()->LoadTexture(fullPath);
        
        // メモリ使用量を推定（テクスチャサイズから計算）
        size_t estimatedSize = EstimateResourceSize(fullPath, ResourceType::Texture);
        textureMemoryUsage += estimatedSize;
        totalMemoryUsage += estimatedSize;
    }
}

void ResourceManager::LoadModel(const std::string& path) {
    // ModelManagerを使用してロード
    if (!ModelManager::GetInstance()->CheckModelExist(path)) {
        ModelManager::GetInstance()->LoadModel(path);
        
        // メモリ使用量を推定
        size_t estimatedSize = EstimateResourceSize(path, ResourceType::Model);
        modelMemoryUsage += estimatedSize;
        totalMemoryUsage += estimatedSize;
    }
}

void ResourceManager::LoadSound(const std::string& path) {
    // Audioクラスを使用してロード
    Audio::GetInstance()->LoadWave(path.c_str());
    
    // メモリ使用量を推定
    size_t estimatedSize = EstimateResourceSize(path, ResourceType::Sound);
    totalMemoryUsage += estimatedSize;
}

void ResourceManager::LoadResourceImmediate(const std::string& path, ResourceType type) {
    switch (type) {
        case ResourceType::Texture:
            LoadTexture(path);
            break;
        case ResourceType::Model:
            LoadModel(path);
            break;
        case ResourceType::Sound:
            LoadSound(path);
            break;
    }
    
    // ロード済みリストに追加
    {
        std::lock_guard<std::mutex> lock(loadedMutex);
        loadedResources.insert(path);
    }
}

float ResourceManager::GetLoadingProgress() const {
    if (totalResourceCount == 0) {
        return 1.0f;
    }
    
    return static_cast<float>(loadedResourceCount) / static_cast<float>(totalResourceCount);
}

bool ResourceManager::IsLoadingComplete() const {
    return !isLoading && (loadedResourceCount >= totalResourceCount);
}

bool ResourceManager::IsResourceLoaded(const std::string& path) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(loadedMutex));
    return loadedResources.find(path) != loadedResources.end();
}

size_t ResourceManager::GetTotalMemoryUsage() const {
    return totalMemoryUsage;
}

size_t ResourceManager::GetTextureMemoryUsage() const {
    return textureMemoryUsage;
}

size_t ResourceManager::GetModelMemoryUsage() const {
    return modelMemoryUsage;
}

size_t ResourceManager::EstimateResourceSize(const std::string& path, ResourceType type) const {
    // ファイル拡張子から推定サイズを計算（実際のファイルサイズを取得する方が良い）
    size_t baseSize = 0;
    
    switch (type) {
        case ResourceType::Texture:
            // テクスチャは解像度に応じて推定
            if (path.find("1024") != std::string::npos || path.find("stage") != std::string::npos) {
                baseSize = 4 * 1024 * 1024;  // 4MB (1024x1024 RGBA)
            } else if (path.find("512") != std::string::npos) {
                baseSize = 1 * 1024 * 1024;  // 1MB (512x512 RGBA)
            } else {
                baseSize = 256 * 1024;  // 256KB (デフォルト)
            }
            break;
            
        case ResourceType::Model:
            // モデルは複雑さに応じて推定
            if (path.find("stage") != std::string::npos) {
                baseSize = 8 * 1024 * 1024;  // 8MB (ステージモデル)
            } else if (path.find("player") != std::string::npos) {
                baseSize = 2 * 1024 * 1024;  // 2MB (キャラクターモデル)
            } else {
                baseSize = 512 * 1024;  // 512KB (小物)
            }
            break;
            
        case ResourceType::Sound:
            // サウンドは長さに応じて推定
            if (path.find("bgm") != std::string::npos || path.find("stage") != std::string::npos) {
                baseSize = 5 * 1024 * 1024;  // 5MB (BGM)
            } else {
                baseSize = 128 * 1024;  // 128KB (効果音)
            }
            break;
    }
    
    return baseSize;
}

void ResourceManager::ReleaseUnusedResources() {
    // 未実装：使用されていないリソースを解放する機能
    // TextureManagerとModelManagerに解放機能を追加する必要がある
    OutputDebugStringA("ResourceManager: 未使用リソースの解放（未実装）\n");
}

void ResourceManager::PrewarmResources() {
    // GPU転送の最適化（将来の実装用）
    OutputDebugStringA("ResourceManager: リソースプリウォーム（未実装）\n");
}

void ResourceManager::ClearCache() {
    // キャッシュクリア
    cacheEntries.clear();
    OutputDebugStringA("ResourceManager: キャッシュクリア完了\n");
}

void ResourceManager::PrintDebugInfo() const {
    std::stringstream ss;
    ss << "\n=== ResourceManager Debug Info ===\n";
    ss << "Total Resources: " << totalResourceCount << "\n";
    ss << "Loaded Resources: " << loadedResourceCount << "\n";
    ss << "Loading Progress: " << std::fixed << std::setprecision(1) 
       << (GetLoadingProgress() * 100.0f) << "%\n";
    ss << "Total Memory Usage: " << (totalMemoryUsage / (1024.0f * 1024.0f)) << " MB\n";
    ss << "Texture Memory: " << (textureMemoryUsage / (1024.0f * 1024.0f)) << " MB\n";
    ss << "Model Memory: " << (modelMemoryUsage / (1024.0f * 1024.0f)) << " MB\n";
    ss << "Is Loading: " << (isLoading ? "Yes" : "No") << "\n";
    ss << "Queue Size: " << loadQueue.size() << "\n";
    ss << "=================================\n";
    
    OutputDebugStringA(ss.str().c_str());
}
