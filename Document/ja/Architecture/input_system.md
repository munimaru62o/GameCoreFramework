# インプットシステム (InputBridge & Manager パターン)

## 概要
本システムは、Unreal Engine 5の「Enhanced Input System」をベースにしながら、マルチプレイやポゼッション（憑依）特有の非同期問題（初期化のレースコンディション）を解決するために構築された入力ルーティング基盤です。

入力を処理するコンポーネントが、特定の入力設定（InputConfig）や対象のアクター（Pawn等）に直接依存しないよう、**Bridgeパターン**と**Manager（仲介者）パターン**を用いて完全に疎結合化されています。

---

## 🛑 解決している課題

従来の入力バインド実装では、以下のような問題が頻発します。

1. **一瞬のイベントへの依存（レースコンディション）:**  
「ポゼス（憑依）された瞬間」という不確かなタイミングに依存すると、まだPawn側のデータロードが終わっていない状態で入力をバインドしようとし、Null参照によるクラッシュが頻発します。

2. **Pawnへの入力処理の直書き（ハードコーディング）**  
`SetupPlayerInputComponent` 内に処理を直書きしてしまうと、乗り物（Vehicle）などに乗り換えた際の「動的な入力の切り替え（着脱）」が極めて困難になります。

3. **魂と肉体の混同**  
システムメニューを開く入力（魂）と、ジャンプする入力（肉体）が同じ場所に記述され、肥大化します。

本システムでは、これらの課題を「4つのレイヤー（階層）」に分割することでアーキテクチャレベルで解決しています。

---

## 📐 アーキテクチャの4つのレイヤー

<img width="10509" height="5289" alt="input_system drawio" src="https://github.com/user-attachments/assets/6bb5c28e-f7a7-423f-add1-d2f1173a91a5" />

*▲ クラス図の細部を確認される際は、画像をクリックするかダウンロードして拡大してご覧ください。*

本システムのクラス図は、大きく4つのレイヤーに分類されます。

### 1. Feature & Data Layer (データ駆動レイヤー)
入力をC++にハードコーディングせず、データアセットとして定義するレイヤーです。

- **[`UGCFPawnData`][GCFPawnData]** / **[`UGCFInputConfig`][GCFInputConfig]**: そのPawnが持つべき「Input Mapping Context（IMC）」や、入力アクションの定義を保持します。

- **`GameFeatureAction`:** GameFeatureから、プレイヤーに紐づく設定ファイルを既存のコードを改修することなく動的に入力定義を注入します。

### 2. Lifecycle & Framework Layer (状態・ライフサイクル管理レイヤー)
ポゼッションや非同期ロードによる「ズレ」を吸収し、安全なタイミングを保証するレイヤーです。

- **`UGameFrameworkComponentManager` (GFCM):** システム全体のFeature状態を同期・監視します。

- **[`UGCFPawnReadyStateComponent`][GCFPawnReadyStateComponent]** / **[`UGCFPlayerReadyStateComponent`][GCFPlayerReadyStateComponent]**: 肉体（Pawn）と魂（PlayerState）の初期化状態を個別に監視します。

- **[`UGCFInputContextComponent`][GCFInputContextComponent]**: [`FGCFContextBinder`][GCFContextBinder] を介して各ReadyStateを監視し、「入力バインドを行っても安全な状態（コンテキスト）」が整ったことをManagerへ通知します。

### 3. Core Manager & Routing Layer (仲介・ブリッジレイヤー)
本システムの中核であり、バインド要求と実際の入力設定を繋ぎ合わせるレイヤーです。

- **[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent]**: 全ての入力バインド要求を受け付けるハブです。状態が未完了の場合は `ProcessPendingBindings()` によって要求を待機（キューイング）し、[`UGCFInputContextComponent`][GCFInputContextComponent] からのGOサインが出た瞬間に安全にバインドを実行します。

- **[`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent]** / **[`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent]**: [`IUGCFInputConfigProvider`][GCFInputConfigProvider] インターフェースを実装し、それが「魂の入力設定」なのか「肉体の入力設定」なのかを抽象化してManagerへ提供します。

### 4. Consumer Layer (消費者レイヤー)
実際の入力を利用してゲームロジックを動かすレイヤーです。

- **[`UGCFMovementControlComponent`][GCFMovementControlComponent]** / **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: これらは自分が「何のPawnを動かしているか」を知る必要がありません。単に [`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent] に対して「移動の入力をバインドしてほしい」と要求（Request）を投げるだけです。

---

## ⚙️ コアメカニズム：安全な非同期バインドのフロー

本システムにおける「入力バインドが安全に完了するまでの流れ」は以下の通りです。

1. **バインドの要求 (Request)**  
   コンシューマー（例：[`UGCFMovementControlComponent`][GCFMovementControlComponent]）が、[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent] に対して入力アクションのバインドを要求します。

2. **待機 (Pending)**  
   この時点では、Pawnのデータがまだ非同期ロード中かもしれません。Managerは要求を即座に実行せず、**内部キュー（PendingBindings）に一時保存**します。

3. **状態の確定 (Context Ready)**  
   GFCMの管理下でデータのロードとポゼッションが完了すると、[`UGCFInputContextComponent`][GCFInputContextComponent] が「入力コンテキストが整った（OnContextChanged）」というイベントを発火します。

4. **安全な実行 (Execute)**  
   Managerは通知を受け取り、`UEnhancedInputLocalPlayerSubsystem` へのマッピングコンテキストの追加や、[`UGCFInputComponent`][GCFInputComponent] に対する実際のデリゲートの紐付けを一斉に実行します。

---

## 🎯 この設計によって得られるメリット

- **完全なクラッシュ耐性**  
非同期環境やラグの激しいネットワーク下でも、ヌルポインタ参照によるクラッシュが原理的に発生しません。

- **プラグアンドプレイの実現**  
新しい乗り物やキャラクターを追加する際、C++コードの変更は一切不要です。[`UGCFPawnData`][GCFPawnData] に新しい InputConfig をセットするだけで、システムが自動的にルーティングを行います。

- **入力の動的着脱**  
ポゼッションが解除された際、Managerが `ClearBindingsOnContextChange()` を呼び出し、古い肉体の入力を綺麗にクリーンアップするため、入力のゴースト（操作できないのに処理が走るバグ）を防ぎます。


[GCFPawnData]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Data/GCFPawnData.h

[GCFContextBinder]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/System/Binder/GCFContextBinder.h
[GCFPawnReadyStateComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/System/Lifecycle/GCFPawnReadyStateComponent.h
[GCFPlayerReadyStateComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/System/Lifecycle/GCFPlayerReadyStateComponent.h

[GCFInputContextComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputContextComponent.h
[GCFInputComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputComponent.h
[GCFInputConfig]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputConfig.h
[GCFInputBindingManagerComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputBindingManagerComponent.h
[GCFPlayerInputBridgeComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPlayerInputBridgeComponent.h
[GCFPawnInputBridgeComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFPawnInputBridgeComponent.h
[GCFInputConfigProvider]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputConfigProvider.h

[GCFMovementControlComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFMovementControlComponent.h
[GCFCameraControlComponent]:  ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h
