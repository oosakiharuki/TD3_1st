#pragma once

// シーンの基底クラス (最低限の構造だけ)
class BaseScene {
public:
    virtual ~BaseScene() = default;

    // シーンを切り替えたタイミングで呼び出される
    virtual void Initialize() {}

    // 毎フレーム呼ばれる
    virtual void Update() {}

    // 毎フレームの描画呼び出し
    virtual void Draw() {}

    // シーン切り替え前に呼び出される
    virtual void Finalize() {}
};
