#pragma once
#include <externals/assimp/include/assimp/Importer.hpp>
#include <externals/assimp/include/assimp/postprocess.h>
#include <externals/assimp/include/assimp/scene.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "WorldTransform.h"
#include "Camera.h"
#include "Object3d.h"
#include "TextureManager.h"

// アニメーションで使用する骨情報
struct BoneInfo {
    Matrix4x4 boneOffset;        // 初期姿勢の逆行列（メッシュ空間からボーン空間への変換）
    Matrix4x4 finalTransformation; // 最終的な変換行列
    int id;                      // ボーンのID
};

// 頂点のボーン重み情報
struct VertexBoneData {
    static const int MAX_BONES_PER_VERTEX = 4;
    int boneIds[MAX_BONES_PER_VERTEX];      // 影響を与えるボーンのID
    float weights[MAX_BONES_PER_VERTEX];    // 各ボーンの影響度（重み）

    // コンストラクタで初期化
    VertexBoneData() {
        for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
            boneIds[i] = -1;
            weights[i] = 0.0f;
        }
    }

    // ボーン情報を追加
    void AddBoneData(int boneId, float weight) {
        for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
            if (weights[i] == 0.0f) {
                boneIds[i] = boneId;
                weights[i] = weight;
                return;
            }
        }
    }
};

// アニメーション用ノード
struct AnimNode {
    std::string name;
    Matrix4x4 transformation;
    std::vector<AnimNode*> children;
};

class FBXModel {
public:
    FBXModel();
    ~FBXModel();

    // モデルの読み込み
    bool LoadModel(const std::string& filePath);

    // 更新処理
    void Update(float deltaTime);

    // 描画処理
    void Draw(const WorldTransform& transform, Camera& camera);

    // アニメーション再生メソッド
    void PlayAnimation(const std::string& animName, bool loop = true);

    // アニメーション再生速度の設定
    void SetAnimationSpeed(float speed) { animationSpeed_ = speed; }

    // アニメーション関連の状態取得
    bool IsAnimationPlaying() const { return isPlaying_; }
    float GetAnimationProgress() const { return animationProgress_; }

private:
    // Assimpのオブジェクト
    Assimp::Importer importer_;
    const aiScene* scene_ = nullptr;

    // モデルデータ
    std::vector<Object3d*> meshModels_;
    std::vector<std::string> textureFilePaths_;
    std::vector<VertexBoneData> vertexBoneData_;

    // アニメーション関連
    std::map<std::string, const aiNodeAnim*> nodeAnimMap_;
    std::map<std::string, int> boneNameToIdMap_;
    std::vector<BoneInfo> boneInfos_;
    Matrix4x4 globalInverseTransform_;
    AnimNode* rootNode_ = nullptr;

    // 現在のアニメーション状態
    std::string currentAnimation_ = "";
    bool isPlaying_ = false;
    bool isLooping_ = true;
    float animationProgress_ = 0.0f;  // 0.0〜1.0の間の値
    float animationSpeed_ = 1.0f;     // アニメーション再生速度

    // 現在のアニメーションの開始/終了時間
    float startTime_ = 0.0f;
    float endTime_ = 0.0f;
    float currentTime_ = 0.0f;

    // モデル読み込み関連のヘルパーメソッド
    void ProcessNode(aiNode* node, const aiScene* scene);
    Object3d* ProcessMesh(aiMesh* mesh, const aiScene* scene);
    void LoadBones(aiMesh* mesh, std::vector<VertexBoneData>& vertexBoneData);
    void LoadAnimations(const aiScene* scene);

    // アニメーション計算用メソッド
    void UpdateAnimation(float deltaTime);
    void CalculateBoneTransform(float timeInSeconds);
    void ReadNodeHierarchy(float animationTime, const aiNode* node, const Matrix4x4& parentTransform);

    // 補間計算
    aiVector3D InterpolatePosition(float animTime, const aiNodeAnim* nodeAnim);
    aiQuaternion InterpolateRotation(float animTime, const aiNodeAnim* nodeAnim);
    aiVector3D InterpolateScaling(float animTime, const aiNodeAnim* nodeAnim);

    // 変換行列関連のヘルパーメソッド
    Matrix4x4 ConvertAssimpMatrix(const aiMatrix4x4& assimpMatrix);

    // ボーンアニメーション用バッファの更新
    void UpdateBoneTransforms(const std::vector<Matrix4x4>& transforms);
};