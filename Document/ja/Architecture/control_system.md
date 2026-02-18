# アクター制御システム (Interface駆動制御 & Control System)

## 概要
本システムは、プレイヤーの「移動の意思（入力）」と、肉体側の「物理的な挙動（歩行、運転など）」を完全に切り離すためのアーキテクチャです。

Unreal Engineの標準的な開発において発生しやすい、移動ロジックの`ACharacter`へのハードコーディングを防ぎ、 インターフェース（Interface）とコンポーネント指向を用いた疎結合な移動制御基盤を提供します。

---

## 🛑 解決している課題

移動処理をPawnやCharacterクラスに直接記述すると、実務において以下の問題に直面します。

1. **ベースクラスへの依存によるCastの増殖**  
   「二足歩行の人間」と「四輪の車」ではベースクラス（`ACharacter` と `AWheeledVehiclePawn`）が異なります。移動ロジックを直書きしていると、これらを同じコントローラーで操作するために分岐（Cast）が無限に増殖し、コードが肥大化します。

2. **物理コンポーネントへの直接参照**  
   標準の `CharacterMovementComponent` や新機能の `Mover` など、特定の物理APIに密結合してしまうと、移動速度などのデータを共通のフローで適用できず、コンポーネントの差し替えが困難になります。

3. **移動ベクトルとカメラの密結合**  
   移動方向とカメラの向きが強固に結びつきすぎていると、「操作キャラクターは固定したままカメラだけを独立して動かす」といった柔軟な演出や視点制御が不可能になります。

本システムでは、これらの課題を「インターフェースによる抽象化」と「ドメインの分割」によって解決しています。

---

## 📐 アーキテクチャの4つのレイヤー

[![Movement System Architecture](../Assets/Images/control_system.drawio.png)](../Assets/Images/control_system.drawio.png)
*▲ クラス図の細部を確認される際は、画像をクリックするかダウンロードして拡大してご覧ください。*

クラス図に基づく本システムの構造は、大きく以下の4層に分かれています。

### 1. Input & Controller Layer (操作・普遍的な意図のレイヤー)
プレイヤーからの入力を受け取り、「どのように動きたいか」という肉体に依存しない普遍的な意図を計算するレイヤーです。
- **[`UGCFMovementControlComponent`][GCFMovementControlComponent]** / **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: これらはプレイヤーコントローラー（魂）に付与されます。アナログスティック等の汎用的な移動入力（Move）や視点操作を処理し、人間用・車用といった特定の肉体に依存する処理は一切持ちません。

### 2. Pawn & Implementation Layer (肉体・固有の実行レイヤー)
コントローラーからの普遍的な命令を受け取ったり、その「肉体（Pawn）」特有の物理挙動や固有アクションを実行するレイヤーです。
- **[`UGCFCharacterControlComponent`][GCFCharacterControlComponent]** / **[`UGCFVehicleControlComponent`][GCFVehicleControlComponent]**: これらはPawn側（肉体）に直接付与されるコンポーネントです。ジャンプ（人間用）やヘッドライト（車用）など、特定の肉体形態に完全に依存する固有の入力操作を処理します。

- **[`IGCFLocomotionHandler`][GCFLocomotionHandler]**: 本システムの中核となるインターフェースです。Controller（魂）の 
**[`UGCFMovementControlComponent`][GCFMovementControlComponent]** は、相手のクラス型を判定することなく、このインターフェースを通じて移動ベクトルだけを肉体へ伝達します。

### 3. Data Layer (データ駆動レイヤー)
移動に関するパラメータ（速度、加速度など）を管理するレイヤーです。
- **[`UGCFMovementConfig`][GCFMovementConfig] (DataAsset)**: プランナーが調整可能な移動パラメータを保持します。

- **[`IGCFMovementConfigReceiver`][GCFMovementConfigReceiver]**: データを受け取るためのインターフェースです。標準の [`UGCFCharacterMovementComponent`][GCFCharacterMovementComponent] や、UE5の新機能である [`UGCFMoverComponent`][GCFMoverComponent] がこれを実装することで、内部の物理コンポーネントが何であっても、共通のフローでパラメータ（Config）を適用できます。

### 4. SubSystem & Camera Layer (カメラ・状態監視レイヤー)
カメラの挙動を移動ロジックから切り離し、メッセージベースで制御するレイヤーです。
- **`UGameplayMessageSubsystem`**: コントローラー側の計算結果やカメラのポリシー変更を、イベントを直接バインドすることなくブロードキャストします。

- **[`GCFCameraMode`][GCFCameraMode]**: Orbit（旋回）やThirdPerson（三人称）など、カメラのポリシーを抽象化し、移動状態に応じて柔軟にカメラの挙動を切り替えます。

---

## ⚙️ コアメカニズム：インターフェース駆動の移動フロー

プレイヤーがスティックを倒してからキャラクターが動くまでの流れは以下の通りです。

1. **入力の受付:**  
   [`UGCFInputComponent`][GCFInputComponent] からバインドされた入力アクションが発火し、[`UGCFMovementControlComponent`][GCFMovementControlComponent] の `Input_Move()` が呼ばれます。

2. **移動ベクトルの計算:**  
   コントローラーがカメラの向いている方向（Rotation）を基準に、実際にPawnが向かうべき移動ベクトルを計算します。

3. **インターフェース経由の命令送信:**  
   コントローラーは、現在ポゼス（憑依）しているPawnが **[`IGCFLocomotionHandler`][GCFLocomotionHandler]** を実装しているかを確認し、実装されていれば `HandleMoveInput()` に計算済みのベクトルを渡します。

4. **肉体ごとの物理実行:**  
   命令を受け取ったのが [`AGCFCharacter`][GCFCharacter] であれば `AddMovementInput` が呼ばれ、[`AGCFWheeledVehiclePawn`][GCFWheeledVehiclePawn] であればスロットルやステアリングの処理に変換されて実行されます。

---

## 🌐 ネットワーク同期と予測（Network Prediction）の最適化

Unreal Engine 5の次世代ネットワーク同期基盤である Network Prediction Plugin (NPP) および Mover プラグインを実戦投入するにあたり、エンジン内部の仕様に起因する致命的な同期ズレや予測の暴走を、本システム独自のアーキテクチャ設計によって解決しています。

### 1. ハイブリッド環境における Clock Sync の維持（Adapterパターンの採用）
旧来の `CharacterMovementComponent` (CMC) と新しい `Mover` が混在するゲーム環境において、NPPのシミュレーション時計（Clock）はシステムごとに孤立してスリープする仕様があります。これにより、プレイヤーがCMCを操作している際、他者のMover（ドローンなど）のネットワークパケットが「未来のデータ」として破棄され、フリーズする問題（Extrapolation Starvation）が発生します。
本システムでは、常にサーバーと通信を行う `PlayerController` 側に**「物理干渉を持たない軽量なダミーMover」**をAdapterとして持たせることで、プレイヤーがどのPawnに憑依していてもNPPの時計をグローバルに同期し続ける強固なインフラを構築しています。

### 2. 予測の暴走（Extrapolation Runaway）を安全に防ぐ入力サニタイズ
Moverの Simulated Proxy（他プレイヤーのキャラクター）は、通信のパケットロスが発生した際、最後に届いた入力データを使い回して未来の位置を予測（Extrapolation）します。しかし、飛行状態など摩擦の少ない状況下では、この「古い入力」によってキャラクターが無限に加速・前進し続け、パケット到達時に強烈な引き戻し（Rubber-banding）を引き起こします。
本システムでは、NPPのコアバッファを汚染することなく、毎フレーム生成される**「予測計算用の使い捨て入力スナップショット」の方向ベクトルのみを直前で強制クリア**するハックを実装しています。これにより、NPPの巻き戻し（Rollback）の安全性を完全に担保したまま、ラグ環境下における致命的なOvershootingを防止しています。

---

## 🎯 この設計によって得られるメリット

- **究極のプラグアンドプレイ (Plug & Play)**  
   新しい「空飛ぶ乗り物」や「全く新しい歩行アルゴリズムを持ったモンスター」を追加する際、コントローラー側のコード（Input_Move等）を1行も書き換える必要がありません。新しいPawnに [`IGCFLocomotionHandler`][GCFLocomotionHandler] を実装するだけで、即座に操作可能になります。

- **新旧物理エンジンの共存**  
   [`IGCFMovementConfigReceiver`][GCFMovementConfigReceiver] インターフェースにより、従来のCharacterMovementと、次世代のMoverプラグインのどちらを採用したPawnであっても、同じデータアセット（MovementConfig）からパラメータを流し込むことができます。

- **チーム開発のスケーラビリティ**  
   プログラマは「インターフェースの規格（規約）」さえ守れば、個々のPawnの内部実装を自由に行えます。これにより、複数人での並行開発時にコンフリクトやバグの連鎖を防ぐことができます。

- **イベント駆動とTickベースシミュレーションのシームレスな統合**  
   次世代の移動システムである Mover プラグインはTickベースの入力予測（Prediction）を要求しますが、本フレームワークはイベント駆動を採用しています。このパラダイムの違いを吸収するため、入力結果を一旦ベクトルとして合成・保持し、Mover側のProducer（[`UGCFCachedInputProducer`][GCFCachedInputProducer]）がInterface経由で安全に読み取る「入力キャッシュ機構」を構築しました。これにより、イベント駆動の軽快さを保ちながら、Moverの強力なロールバック機構を完全サポートしています。


[GCFMovementControlComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementControlComponent.h
[GCFCameraControlComponent]: ../../Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h

[GCFCharacter]: ../../Source/GameCoreFramework/Public/Actor/Character/GCFCharacter.h
[GCFWheeledVehiclePawn]: ../../Source/GameCoreFramework/Public/Actor/Vehicle/GCFWheeledVehiclePawn.h
[GCFCharacterControlComponent]: ../../Source/GameCoreFramework/Public/Actor/Character/GCFCharacterControlComponent.h
[GCFVehicleControlComponent]: ../../Source/GameCoreFramework/Public/Actor/Vehicle/GCFVehicleControlComponent.h
[GCFLocomotionHandler]: ../../Source/GameCoreFramework/Public/Movement/GCFLocomotionHandler.h

[GCFMoverComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFMoverComponent.h
[GCFCharacterMovementComponent]: ../../Source/GameCoreFramework/Public/Movement/GCFCharacterMovementComponent.h

[GCFMovementConfig]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementConfig.h
[GCFMovementConfigReceiver]: ../../Source/GameCoreFramework/Public/Movement/GCFMovementConfigReceiver.h
[GCFLocomotionHandler]: ../../Source/GameCoreFramework/Public/Movement/GCFLocomotionHandler.h
[GCFCachedInputProducer]: ../../Source/GameCoreFramework/Public/Movement/Mover/GCFCachedInputProducer.h

[GCFCameraMode]: ../../Source/GameCoreFramework/Public/Camera/Mode/GCFCameraMode.h

[GCFInputComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputComponent.h