# ゲーム最適化実装ガイド

## 概要
このドキュメントは、DirectX 12ゲームプロジェクトの最適化実装について説明します。

## 実装した最適化

### 1. リソース管理の最適化（ResourceManager）

#### 特徴
- **優先度付きリソースローディング**
  - Essential（必須）：ローディング画面やUI
  - High（高）：プレイヤーモデル、基本的なゲームオブジェクト
  - Medium（中）：敵キャラクター、エフェクト
  - Low（低）：ステージモデル、装飾品

- **非同期バッチローディング**
  - 別スレッドでリソースを読み込み、メインスレッドをブロックしない
  - 進行状況をリアルタイムで取得可能

- **メモリ使用量追跡**
  - テクスチャ、モデル、音声のメモリ使用量を個別に追跡
  - 未使用リソースの解放機能（将来実装予定）

#### 使用方法
```cpp
// 初期化
ResourceManager::GetInstance()->Initialize();

// リソースをキューに追加
ResourceManager::GetInstance()->QueueResource("resource/Sprite/player.png", 
                                            ResourceType::Texture, 
                                            ResourcePriority::High);

// バッチローディング開始
ResourceManager::GetInstance()->StartBatchLoading();

// 進行状況取得
float progress = ResourceManager::GetInstance()->GetLoadingProgress();
```

### 2. パフォーマンス監視（PerformanceMonitor）

#### 特徴
- **フレームレート計測**
  - 現在のFPSと平均FPS
  - フレームタイムの追跡
  - フレームドロップ検出

- **セクション別計測**
  - Update、Draw、各処理の実行時間を個別に計測
  - ボトルネックの特定が容易

- **メモリ使用量監視**
  - プロセスのメモリ使用量をリアルタイム監視
  - リソースマネージャーと連携した詳細表示

#### 使用方法
```cpp
// フレーム計測
PerformanceMonitor::GetInstance()->BeginFrame();
// ... ゲームロジック ...
PerformanceMonitor::GetInstance()->EndFrame();

// セクション計測
PerformanceMonitor::GetInstance()->BeginSection("PlayerUpdate");
player->Update();
PerformanceMonitor::GetInstance()->EndSection("PlayerUpdate");

// 統計情報取得
std::string stats = PerformanceMonitor::GetInstance()->GetPerformanceStats();
```

### 3. 最適化されたローディングシーン

#### 改善点
- **効率的なリソースローディング**
  - 優先度に基づいた段階的ローディング
  - 必要最小限のリソースを先読み

- **スムーズなアニメーション**
  - プログレスバーのイージング
  - キラキラエフェクトの最適化
  - 60FPSを維持するアニメーション設計

- **メモリ効率**
  - 不要なスレッドスリープを削除
  - リソースの重複ロードを防止

### 4. ゲームシーンの最適化

#### 実装内容
- **描画処理の最適化**
  - セクション別のパフォーマンス計測
  - 不要な描画コールの削減

- **更新処理の最適化**
  - プレイヤー、敵、マップの更新を個別計測
  - ボトルネックの可視化

## ビルド設定

プロジェクトファイル（.vcxproj）に以下のファイルを追加：
```xml
<ClCompile Include="ResourceManager.cpp" />
<ClCompile Include="PerformanceMonitor.cpp" />
<ClInclude Include="ResourceManager.h" />
<ClInclude Include="PerformanceMonitor.h" />
```

## パフォーマンス目標

- **フレームレート**: 60FPS維持
- **ローディング時間**: 従来の50%削減
- **メモリ使用量**: ピーク時512MB以下
- **描画コール**: 1フレームあたり1000コール以下

## 今後の最適化案

1. **GPU最適化**
   - インスタンシングの実装
   - フラストラムカリング
   - LOD（Level of Detail）システム

2. **メモリ最適化**
   - テクスチャ圧縮（DDS形式）
   - モデルデータの最適化
   - 動的リソース管理

3. **CPUマルチスレッド最適化**
   - 物理演算の並列化
   - AIロジックの並列処理
   - 描画コマンドの並列生成

## デバッグ機能

ImGuiウィンドウでパフォーマンス情報を表示：
- FPS、フレームタイム
- セクション別処理時間
- メモリ使用量
- リソースロード状況

## 注意事項

1. ResourceManagerとPerformanceMonitorはシングルトンパターンで実装
2. 必ずFinalize()を呼び出してリソースを解放すること
3. デバッグビルドではパフォーマンス計測のオーバーヘッドがあるため、リリースビルドでの計測を推奨

## まとめ

この最適化により、以下の改善が期待できます：
- ローディング時間の大幅短縮
- 安定したフレームレート
- メモリ使用量の削減
- パフォーマンスボトルネックの可視化

継続的な最適化により、さらなるパフォーマンス向上が可能です。
