# GameCoreFramework (GCF)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.7+-white.svg?logo=unrealengine&logoColor=white&color=0E1128)
![License](https://img.shields.io/badge/License-MIT-green.svg)

*他の言語で読む: [English](README.md), [日本語 (Japanese)](README_ja.md)*

## 🎯 プロジェクト概要

本プロジェクトは、Unreal Engine 5の最新アーキテクチャ（Lyra Starter Game）とGameplay Ability System（GAS）の設計思想をベースに構築した、拡張性が高く疎結合なモジュラー型ゲームフレームワークです。

---

## 💡 コアとなる設計思想

- **完全な関心の分離**  
ACharacter や APlayerController へのハードコーディングを排除し、各機能を独立したコンポーネント（GameFeatures）として分割。これにより、既存のコードを汚染することなく新しい機能やキャラクターを安全に追加可能です。

- **魂と肉体の分離**  
PlayerState（永続的な魂）と Pawn（一時的な肉体）の責務を完全に分離。これによりAbilityやInputのバインディングを永続的なものと一時的なものに分解して定義することが可能になります。

- **データドリブンなルーティング**  
プログラマ以外（プランナーやアーティスト）でも、C++を触ることなく入力とアビリティの紐付けや動作の調整ができるよう、DataAssetを介したルーティング層を構築しています。これによりチーム全体のイテレーション速度を最大化します。

- **安全な非同期ライフサイクル管理**  
GameFrameworkComponentManager（GFCM）を活用し、各コンポーネントの依存関係と初期化状態（Feature State）を厳密に管理。イベント駆動と状態管理のハイブリッド構成を実装することにより、常に最新の状態を同期することができ、「参照先のものがまだ生成されていない」というUE特有の初期化順序問題をシステムレベルで防いでいます。

- **パフォーマンス指向のTickレス設計**
毎フレームのTick処理への依存を原則として排除し、イベント駆動（Event-Driven）と状態のブロードキャストによってシステムを稼働させています。これにより待機中のCPUオーバーヘッドを極限まで削ぎ落とし、Gameスレッドの処理時間を数ミリ秒単位に抑える圧倒的なパフォーマンスを実現しています。

---

## 📚 詳細ドキュメント

GCFの設計思想や各システムの詳細については、以下のドキュメントを参照してください。

- **[アーキテクチャ概要](Document/ja/Architecture/overview.md)**
  - マルチプレイ開発で頻発する「初期化のレースコンディション」や「責務の肥大化」をシステムレベルで防ぐための根幹思想（魂と肉体の分離など）と、実務の失敗から得た教訓をまとめています。

- **[GAS統合とAbilityルーティング (Dual ASC & Router パターン))](Document/ja/Architecture/ability_system.md)** 
  - PlayerStateとPawnの両方にASCを持たせる高度な「Dual ASC構成」を採用。入力とアビリティの密結合を排除し、タグのプレフィックスで発動先を動的に切り替えるルーティング機構を解説しています。

- **[インプットシステム (InputBridge & Manager パターン)](Document/ja/Architecture/input_system.md)** 
  - ポゼッション時の非同期ロードによって発生する入力バインドのクラッシュを根絶するため、状態が整うまで要求を待機し、安全なタイミングで一斉適用する堅牢なマネージャー設計です。

- **[アクター制御システム (Interface駆動制御 & Control System)](Document/ja/Architecture/control_system.md)**
  - 「プレイヤーの操作意図」と「Pawnの物理挙動・固有アクション」をインターフェースで完全に分離。クラスのキャスト地獄を防ぎ、移動だけでなく搭乗物ごとの固有操作（ジャンプ、ライト点灯等）を疎結合に処理する仕組みです。

---


## 🚀 クイックスタート（インストール手順）

本リポジトリは、フレームワーク本体（プラグイン）と、すぐに挙動を確認できるサンプルプロジェクトが一体となった構成になっています。

拡張性と堅牢性を最大化するため、本フレームワークはEpic Games公式の「Lyra Starter Game」に含まれる最新の基盤プラグインに依存しています。以下の手順に沿ってデモ環境を構築してください。

### Step 1: リポジトリの取得

1. 本リポジトリをローカルにクローン、またはZIPでダウンロードして展開します。

### Step 2: Lyra依存プラグインの移植 (必須)

本フレームワークをコンパイル・動作させるには、Epic Games Launcherから「Lyra Starter Game（UE5.7対応版）」をダウンロードし、特定のプラグインを本リポジトリ内にコピーしてくる必要があります。

1. Lyra Starter Gameのプロジェクトフォルダを開きます（`[LyraProjectDirectory]/Plugins/`）。

2. クローンした本リポジトリの `Plugins` フォルダ内に、以下のLyra専用プラグインフォルダをコピーしてください。

  - CommonGame
  - CommonUser
  - CommonLoadingScreen
  - GameplayMessageRouter
  - ModularGameplayActors
  - GameSubtitles
  - UIExtension

*(※ GameFeatures、Mover などのエンジンプラグインはUE本体に組み込まれているためコピー不要です)*

▼ 最終的なディレクトリ構成のイメージ
```text
GameCoreFramework/ (クローンしたリポジトリのルート)
 ├── GCF_SampleProject.uproject
 ├── Source/
 └── Plugins/
     ├── GameCoreFramework/      <-- 本フレームワーク（最初から含まれています）
     ├── CommonGame/             <-- 📥 Lyraからコピーして配置
     ├── GameplayMessageRouter/  <-- 📥 Lyraからコピーして配置
     └── ... (その他のコピーしたプラグイン)
```

### Step 3: プロジェクトのビルド

1. リポジトリのルートにある `GCF_SampleProject.uproject` を右クリックし、「Generate Visual Studio project files」を実行します。

2. 生成された `.sln`（またはIDEのプロジェクトファイル）を開き、プロジェクトをビルド（Development Editor等）してください。

3. エディタが起動したら、`Edit > Plugins` から本プラグインおよび依存プラグイン（GameplayAbilities, EnhancedInput, Mover など）が有効化（Enabled）されていることを確認してください。

---

### ご自身のプロジェクトへ導入（移植）する場合

このフレームワークをご自身のゲームプロジェクトに導入したい場合は、上記の `Step 2` で用意したLyraの依存プラグイン群と一緒に、本リポジトリ内の `Plugins/GameCoreFramework` フォルダを、**ご自身のプロジェクトの `Plugins` フォルダへそのままコピー**してください。

## 🎬 デモ & サンプル

本プロジェクトには挙動を確認するための、デモ用アセットを用意してあります。  
C++のコードを変更せずとも、DataAssetを編集するだけで動的にアビリティや挙動を切り替えられる設計になっています。

```text
GameCoreFramework/Content/Sample
├── Assets/         # PawnやParticleのためのアセット類
├── Blueprints/     # PawnやAbilityのBPクラス群
├── DataAssets/     # データ駆動のコア部分、ここに各DataAssetが定義されています
├── Experiences/    # Experience定義
└── Maps/           # デモ用マップ定義
└── UI/             # デバッグHUD用UI定義
```

### 🖥️ Sample動画  

https://github.com/user-attachments/assets/8a15b152-5af3-4229-9f05-af8823d93742

#### **💡 本動画の見どころ**
ポゼッション（憑依）を変更した瞬間に、古い肉体の入力バインドが安全に破棄され、**新しいPawnの `InputBinding` が動的に更新される様子**や、新しい肉体に付与されたAbilityが即座に有効化・ルーティングされる「ライフサイクルの完全な同期」を確認できます。

#### 📊 画面上のデバッグ表示について
- **Debug Input Info:** 現在アクティブにバインドされている入力アクションとルーティング状態をリアルタイムに表示します。
- **Debug State Info:** GFCMの `InitState`（各機能の初期化フェーズ）や、現在のポゼッション状態を監視します。
- **Debug Log:** ポゼッション切り替えイベントや、Abilityルーター経由のタグ送信ログを出力します。

#### 🎮 ポゼッションの実行とターゲット選定
- 画面内でハイライト（アウトライン表示）されたActorに対して、`Interact` アビリティ（魂側の永続能力）を実行することでポゼッションを移行します。
- ターゲットはカメラモードに応じて動的に選定されます。（TPSモード時は画面中央、見下ろしモード時はマウスカーソルの位置を基準に判定）。

#### 🤖 操作可能なActorのバリエーション（新旧システム・物理エンジンの共存）
アーキテクチャの疎結合性を証明するため、全く異なる物理コンポーネントを持つPawn間をシームレスに行き来します。
- **白いマネキン:** 従来の `CharacterMovementComponent` による移動と、Jump / Crouch 機能。
- **色付きマネキン:** 白いマネキンの機能に加え、現在の「肉体」に固有で定義された専用アビリティの実行機能。
- **球体 (Mover):** UE5の次世代システム `Mover` プラグインによるTickベースの移動制御と、見下ろし視点カメラ。
- **車両 (Chaos Vehicle):** `ChaosVehicleMovementComponent` による本格的な車両制御と、Headlight / Handbrake などの固有操作。

### 🌐 Client Sample動画（Network Lag Simulation）  

https://github.com/user-attachments/assets/212a420c-ef0d-4caa-8247-e3068315acb4

#### **💡 本動画の見どころ**
劣悪な通信環境下において、通信遅延による「初期化順序の逆転」や「非同期ロード時のライフサイクルのズレ（レースコンディション）」が発生しても、GFCMによる厳格な状態管理がそれらを完全に吸収している様子が確認できます。

#### 📡 テスト環境（意図的なネットワーク遅延）
Listen Serverに接続したClient環境にて、マルチプレイで頻発する過酷なネットワーク制限をエミュレートしています。
- **Latency (Ping遅延):** 200 ms
- **Packet Loss (パケットロス):** 10%

このような厳しいラグ環境下においても、ポゼッションの移行が安全に完了し、Null参照によるクラッシュや入力のバインド漏れが一切発生せず、アーキテクチャが堅牢に機能していることが証明されています。

---

## 📌 プロジェクトのステータスと貢献（Contribution）について

本プロジェクトは、Unreal Engine 5におけるモダンなアーキテクチャ設計の実証、および **学習・参考用（Reference purpose）** として個人的に公開しているものです。

そのため、バグ修正を除く「新機能の追加」や「設計の根幹に関わる大規模なプルリクエスト（PR）」については、**原則として受け付けておりません**。

本フレームワークの設計思想に共感していただけた場合は、ぜひ本リポジトリをForkするか、必要なコードやアーキテクチャのアイデアをご自身のプロジェクトに自由に持ち帰って（MITライセンスの範囲内で）ご活用ください！

---

## 💖 Credits & Acknowledgments

- **[Lyra Starter Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine)** by Epic Games: 
  モジュラーな設計思想の活用において多大な影響を受けました。
- **Unreal Engine Community**: 
  ベストプラクティスを共有してくれたすべての開発者に感謝します。

---

## ⚖ ライセンス

このプロジェクトは MIT ライセンスの下で公開されています。
詳細は [LICENSE](LICENSE) を参照してください。  
Inspired by Lyra Starter Game by Epic Games.

---
