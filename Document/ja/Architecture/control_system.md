# アクター制御システム (Interface駆動オプトイン制御 & Control System)

🌍 *他の言語で読む: [English](../../en/Architecture/control_system.md) | [日本語 (Japanese)](../../ja/Architecture/control_system.md)* 

## 概要
本システムは、プレイヤーの「移動の意思（入力）」と、肉体側の「物理的な挙動（歩行、運転など）」を完全に切り離すためのアーキテクチャです。

Unreal Engineの標準的な開発において発生しやすい、移動ロジックの`ACharacter`へのハードコーディングを防ぎ、インターフェース（Interface）と「Push ➔ Cache ➔ Pull」のバケツリレーを用いた、疎結合な移動制御基盤を提供します。

---

## 🛑 解決している課題

移動処理をPawnやCharacterクラスに直接記述すると、実務において以下の問題に直面します。

1. **ベースクラスへの依存によるCastの増殖**
   「二足歩行の人間」と「四輪の車」ではベースクラス（`ACharacter` と `AWheeledVehiclePawn`）が異なります。移動ロジックを直書きしていると、これらを同じコントローラーで操作するために分岐（Cast）が無限に増殖し、コードが肥大化します。

2. **物理コンポーネントへの直接参照**
   標準の `CharacterMovementComponent` や新機能の `Mover` など、特定の物理APIに密結合してしまうと、移動速度などのデータを共通のフローで適用できず、コンポーネントの差し替えが困難になります。

3. **移動ベクトルとカメラの密結合**
   移動方向とカメラの向きが強固に結びつきすぎていると、「操作キャラクターは固定したままカメラだけを独立して動かす」といった柔軟な演出や視点制御が不可能になります。

本システムでは、これらの課題を「インターフェースによる抽象化」と「ドメインの厳密な分割」によって解決しています。

---

## 📐 アーキテクチャの4つのレイヤー

<img width="11889" height="7089" alt="control_system drawio" src="https://github.com/user-attachments/assets/ee58b564-9fd2-4455-8dc6-ebddf42a762a" />

*▲ クラス図の細部を確認される際は、画像をクリックするかダウンロードして拡大してご覧ください。*

本システムの構造は、大きく以下の4層に分かれています。「魂（Controller）」と「肉体（Pawn）」が明確に分離されているのが特徴です。

### 1. Input & Controller Layer (魂・普遍的な意図のレイヤー)
プレイヤーからの入力を受け取り、「どのように動きたいか」という肉体に依存しない普遍的な意図（Intent）を計算するレイヤーです。

- **[`UGCFLocomotionDirectionComponent`][GCFLocomotionDirectionComponent]**: プレイヤーコントローラー（魂）に付与されます。アナログスティック等の汎用的な移動ベクトル（Direction）を処理し、相手の肉体構造を一切気にせず意図を押し込みます（Push）。

- **[`UGCFCameraControlComponent`][GCFCameraControlComponent]**: 視点操作を担当し、移動ロジックから完全に独立してカメラの回転（ControlRotation）を制御します。

### 2. Pawn & Implementation Layer (肉体・固有アクションと意図の保持レイヤー)
コントローラーからの普遍的な命令をキャッシュしたり、「ジャンプ」や「しゃがみ」といった特定の肉体に依存するアクション入力を処理する層です。
- **GCF Pawn 3層アーキテクチャ**: 肉体の物理的要件に合わせてPawnを厳密にクラス分割しています。
  - [`AGCFPawn`][GCFPawn]: 意図のキャッシュのみを行う最も純粋な基底クラス。
  - [`AGCFAvatarPawn`][GCFAvatarPawn]: メッシュを持ち、形状に依存しない「ジャンプ」などの汎用アクションを担当。
  - [`AGCFHumanoid`][GCFHumanoid]: カプセルコリジョンを持ち、「しゃがみ」などカプセルに強く依存する特化アクションを担当。

- **[`UGCFLocomotionActionComponent`][GCFLocomotionActionComponent]**: Pawn側（肉体）などに付与され、固有アクション（Jump/Crouch等）の意図をPushします。

- **[`IGCFLocomotionInputHandler`][GCFLocomotionInputHandler]**: 入力コンポーネントからPawnへ意図を伝える「Push」用のインターフェースです。

- **[`IGCFLocomotionInputProvider`][GCFLocomotionInputProvider]**: Moverのプロデューサーが、Pawnにキャッシュされた意図を安全に吸い上げる「Pull」用の読み取り専用インターフェースです。

### 3. Data & Simulation Layer (データ駆動・物理シミュレーションレイヤー)
実際の物理的な移動や、パラメータ（速度、加速度など）を管理するレイヤーです。
- **[`UGCFCachedInputProducer`][GCFCachedInputProducer]**: UE5のMoverプラグインにおいて、Pawnからインターフェース越しに意図をPullし、物理エンジンへ翻訳・伝達するコンポーネントです。
  - ※しゃがみ等、Moverに専用の入力構造体（[`FGCFHumanoidInputs`][GCFHumanoidInputs]）を渡す必要がある場合は、これを継承した [`UGCFHumanoidInputProducer`][GCFHumanoidInputProducer] を作成することで、コアロジックを再利用しつつ安全に拡張（Inject）できます。

- **[`UGCFMovementConfig`][GCFMovementConfig] (DataAsset)**: プランナーが調整可能な移動パラメータを保持します。

- **[`IGCFMovementConfigReceiver`][GCFMovementConfigReceiver]**: Moverや旧CMCなど、内部の物理コンポーネントが何であっても共通のフローでパラメータを適用するためのインターフェースです。

### 4. SubSystem & Camera Layer (カメラ・状態監視レイヤー)
カメラの挙動を移動ロジックから切り離し、メッセージベースで制御するレイヤーです。
- **`UGameplayMessageSubsystem`**: コントローラー側の計算結果やカメラのポリシー変更を、イベントを直接バインドすることなくブロードキャストします。

- **[`GCFCameraMode`][GCFCameraMode]**: Orbit（旋回）やThirdPerson（三人称）など、カメラのポリシーを抽象化し、移動状態に応じて柔軟にカメラの挙動を切り替えます。

---

## ⚙️ コアメカニズム：意図のバケツリレー（Push ➔ Cache ➔ Pull）

プレイヤーがスティックを倒してからキャラクターが動くまでの流れは、Moverプラグインに最適化された3ステップの「バケツリレー」で行われます。

1. **意図の送信 (Push):** [`UGCFLocomotionDirectionComponent`][GCFLocomotionDirectionComponent] や [`UGCFLocomotionActionComponent`][GCFLocomotionActionComponent] が入力を検知すると、対象のPawnクラスを一切キャストせず、**[`IGCFLocomotionInputHandler`][GCFLocomotionInputHandler]** 越しに「この方向に動きたい」「ジャンプしたい」という意図（Intent）を投げ飛ばします。

2. **中継と保持 (Cache & Opt-In):** 命令を受け取ったPawnは、UHTのデフォルト実装により、自分が実行できないアクション（車のしゃがみ等）は安全に無視します。自身が対応する機能のみをオーバーライド（Opt-In）し、Moverからの要求に備えて状態を変数に保存（キャッシュ）します。

3. **意図の吸い上げと実行 (Pull):** 毎フレーム、Moverの **[`UGCFCachedInputProducer`][GCFCachedInputProducer]** （またはその派生クラス）が **[`IGCFLocomotionInputProvider`][GCFLocomotionInputProvider]** を通じてPawnからキャッシュされた移動ベクトルや向き（Orientation）、アクションの意図を安全に吸い上げ、実際の物理シミュレーションに適用します。

---

## 🎯 この設計によって得られるメリット

- **高度なプラグアンドプレイ (Plug & Play)**  
   新しい「空飛ぶ乗り物」や「全く新しい歩行アルゴリズムを持ったモンスター」を追加する際、プロデューサーやコントローラー側のコードを1行も書き換える必要がありません。新しいPawnにインターフェースを実装（オプトイン）するだけで、即座に操作可能になります。

- **UHTを活用したゼロ・ボイラープレート設計**  
   インターフェースのデフォルト挙動を利用しているため、「ジャンプできない車」に空のジャンプ関数を大量に実装させられるようなコードの肥大化（ボイラープレート）が一切発生しません。

- **コアロジックの再利用と安全な拡張性**  
  移動やジャンプといった共通処理は単一のProducerで完結し、「しゃがみ」のような固有処理が必要になった場合のみProducerを継承して拡張する設計なため、密結合を生むことなくプロジェクト全体へ安全にスケールします。

- **チーム開発のスケーラビリティ**  
   プログラマは「インターフェースの規格（Push/Pull）」さえ守れば、個々のPawnの内部実装を自由に行えます。これにより、複数人での並行開発時にコンフリクトやバグの連鎖を防ぐことができます。

- **イベント駆動とTickベースシミュレーションのシームレスな統合**  
   MoverプラグインはTickベースの入力予測（Prediction）を要求しますが、本フレームワークはイベント駆動を採用しています。このパラダイムの違いを吸収するため、Pawnを中継地点とした「キャッシュ＆Pull機構」を構築しました。これにより、イベント駆動の軽快さを保ちながら、Moverの強力なネットワーク同期とロールバック機構を完全サポートしています。


[GCFCharacter]:                          ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Character/GCFCharacter.h
[GCFPawn]:                               ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Pawn/GCFPawn.h
[GCFAvatarPawn]:                         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Avatar/GCFAvatarPawn.h
[GCFHumanoid]:                           ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Humanoid/GCFHumanoid.h
[GCFWheeledVehiclePawn]:                 ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Actor/Vehicle/GCFWheeledVehiclePawn.h

[GCFLocomotionDirectionComponent]:       ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionDirectionComponent.h
[GCFLocomotionActionComponent]:          ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionActionComponent.h
[GCFLocomotionInputHandler]:             ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionInputHandler.h
[GCFLocomotionInputProvider]:            ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Locomotion/GCFLocomotionInputProvider.h
[GCFMoverComponent]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/GCFMoverComponent.h
[GCFCachedInputProducer]:                ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/Producer/GCFCachedInputProducer.h
[GCFHumanoidInputProducer]:              ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/Producer/GCFHumanoidInputProducer.h
[GCFHumanoidInputs]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/Mover/Input/GCFHumanoidInputs.h
[GCFCharacterMovementComponent]:         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFCharacterMovementComponent.h
[GCFMovementConfig]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFMovementConfig.h
[GCFMovementConfigReceiver]:             ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Movement/GCFMovementConfigReceiver.h

[GCFCameraMode]:                         ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Camera/Mode/GCFCameraMode.h
[GCFCameraControlComponent]:             ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Camera/GCFCameraControlComponent.h

[GCFInputComponent]:                     ../../../Plugins/GameCoreFramework/Source/GameCoreFramework/Public/Input/GCFInputComponent.h