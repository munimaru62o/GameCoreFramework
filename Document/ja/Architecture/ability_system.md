# GAS統合とAbilityルーティング (Dual ASC & Router パターン)

## 概要
本システムは、Unreal EngineのGameplay Ability System（GAS）を大規模マルチプレイや複雑なポゼッション（憑依）要件に適応させるためのコアモジュールです。

最大の特徴は、PlayerState（魂）とPawn（肉体）の**両方にASC（Ability System Component）を持たせる「Dual ASC構成」**を採用している点と、プレイヤーの入力をタグのプレフィックスによって各ASCへ自動的に振り分ける**「Ability Router パターン」**を実装している点です。

---

## 🛑 解決している課題

一般的なGASの実装チュートリアルでは、ASCをPawnかPlayerStateのどちらか一方に配置します。しかし、実務においてそのセオリーに従うと以下の問題が発生します。

1. **Pawn単一ASCによる永続性の喪失**  
   ASCを肉体側にしか持たない場合、キャラクターが死亡してPawnが破棄された瞬間、保持していたバフ（Cooldownや永続効果）もすべて同時に消滅してしまいます。

2. **PlayerState単一ASCによる能力の汚染**  
   ASCを魂側にしか持たない場合、車への搭乗やモンスターへの変身を行った際、「人間のアビリティ」と「車のアビリティ」がPlayerState内で混ざり合い、状態管理が完全に破綻します。

3. **入力とアビリティの直接バインド（密結合）**  
   「ジャンプボタン＝JumpAbilityの起動」とC++で直書きしてしまうと、別の肉体に乗り換えた先で「ジャンプボタン＝飛行Ability」のように動的に割り当てを変更することができなくなります。

本システムは、これらの課題を「ドメイン（領域）の完全分離」と「ルーティング層の導入」によってアーキテクチャレベルで解決しています。

---

## 📐 アーキテクチャの5つのレイヤー

[![Ability System Architecture](../Assets/Images/ability_system.drawio.png)](../Assets/Images/ability_system.drawio.png)
*▲ クラス図の細部を確認される際は、画像をクリックするかダウンロードして拡大してご覧ください。*

クラス図に基づく本システムの構造は、明確に責務が分かれた以下の5層で構成されています。

### 1. Feature & Data Layer (データ駆動レイヤー)
能力や入力の定義をデータとして保持し、システムに動的に注入するレイヤーです。

- **[`UGCFInputConfig`][GCFInputConfig] (DataAsset)**: どのアクションがどのGameplayTagを発火させるかを定義します。

- **`GameFeatureAction`**: GameFeatureの機能を利用し、既存のコードを汚さずに動的にアビリティ（魂用・肉体用）やInputConfigを注入します。

### 2. Input Binding Layer (入力受付レイヤー)
プレイヤーからの物理的な入力（ボタン押下など）を安全に受け付けるレイヤーです。

- **[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent]** / **[`UGCFInputComponent`][GCFInputComponent]**: プレイヤーコントローラーに紐付き、非同期のロードやポゼッションの状態が整ったタイミングで、安全にEnhanced Inputのバインド処理を行います。

### 3. Ability Routing Layer (ルーティング層・本システムの中核)
入力を受け取り、適切なASCへ処理をディスパッチ（転送）するレイヤーです。

- **[`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent]** / **[`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent]**: 入力レイヤーに対してバインドを要求し、入力発生時には「GameplayTag」のみをRouterへ送信する橋渡しを行います。

- **[`UGCFAbilityInputRouterComponent`][GCFAbilityInputRouterComponent]**: 入力されたGameplayTagのプレフィックスを解析し、以下のルールに従って自動でルーティングを行います。
  - **`Ability.Player.*`** のタグ → **PlayerState の ASC** へ送信
  - **`Ability.Pawn.*`** のタグ → **Pawn の ASC** へ送信

### 4. Player Domain Layer (魂の領域)
永続的なデータと能力を管理するレイヤーです。

- **[`UGCFPlayerState`][GCFPlayerState]**: プレイヤーの「魂」として機能し、独自の **[`UGCFAbilitySystemComponent`][GCFAbilitySystemComponent]** を保持します。

- **役割:** プレイヤー自身のレベルアップスキル、インベントリのアイテム効果、全体クールダウンなど、「Pawn（肉体）を乗り換えても維持されるべき能力」をここで管理します。

### 5. Pawn Domain Layer (肉体の領域)
一時的な器と、それに紐づく物理的な能力を管理するレイヤーです。

- **[`UGCFCharacterWithAbilities`][GCFCharacterWithAbilities]**: プレイヤーが操作する「肉体」であり、こちらも独自の **[`UGCFAbilitySystemComponent`][GCFAbilitySystemComponent]** を保持します。

- **役割:** 「二足歩行のジャンプ」「銃を撃つ」「車のアクセルを踏む」といった、そのPawnに完全に依存する固有の能力を管理します。

---

## ⚙️ コアメカニズム：タグベースの自動振り分けフロー

プレイヤーがボタンを押してからアビリティが発動するまでの流れは以下の通りです。

1. **入力の受付:**  
   プレイヤーがボタンを押すと、[`UGCFInputBindingManagerComponent`][GCFInputBindingManagerComponent] 経由で [`UGCFPlayerInputBridgeComponent`][GCFPlayerInputBridgeComponent] または [`UGCFPawnInputBridgeComponent`][GCFPawnInputBridgeComponent] にイベントが到達します。

2. **タグの送信 (Bridge):**  
   Bridgeコンポーネントは「どのボタンが押されたか」ではなく、InputConfigに定義された**「GameplayTag」**（例：`Ability.Pawn.Jump`）のみをRouterへ送信します。

3. **動的ルーティング (Router):**  
   [`UGCFAbilityInputRouterComponent`][GCFAbilityInputRouterComponent] がタグのプレフィックスを解析します。
   この時、コントローラーは「自分が今、人間に憑依しているのか車に乗っているのか」を知る必要がありません。

4. **ASCへの通知と発動:**  
   Routerが対象のASC（PlayerState または Pawn）の `AbilityInputTagPressed()` を叩き、該当するアビリティが実行されます。

---

## 🎯 この設計によって得られるメリット

- **究極の疎結合（Decoupling）**  
   入力側（Controller）はアビリティの実装を一切知る必要がなく、アビリティ側もどのボタンに割り当てられているかを知る必要がありません。全ては「タグ」という共通言語だけで通信されます。

- **安全なポゼッション（憑依）の実装**  
   車に乗り込んだ際（Possession）、古いPawnのASCとの紐付けが切れ、新しいPawn（車）のASCへと自動的にルーティング先が切り替わります。魂のASC（PlayerState）への通信はそのまま維持されるため、システムが破綻しません。

- **プランナー主導の開発**  
   新しいスキルを追加する際、プログラマがC++で入力バインドのコードを書く必要がなくなります。プランナーがDataAsset（[`UGCFInputConfig`][GCFInputConfig]）にタグを登録するだけで、システムが自動的に適切なASCへ繋ぎ込みを行います。


[GCFInputConfig]: ../../Source/GameCoreFramework/Public/Input/GCFInputConfig.h

[GCFInputBindingManagerComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputBindingManagerComponent.h
[GCFInputComponent]: ../../Source/GameCoreFramework/Public/Input/GCFInputComponent.h

[GCFPlayerInputBridgeComponent]: ../../Source/GameCoreFramework/Public/Input/GCFPlayerInputBridgeComponent.h
[GCFPawnInputBridgeComponent]: ../../Source/GameCoreFramework/Public/Input/GCFPawnInputBridgeComponent.h
[GCFAbilityInputRouterComponent]: ../../Source/GameCoreFramework/Public/Input/GCFAbilityInputRouterComponent.h

[GCFPlayerState]: ../../Source/GameCoreFramework/Public/Actor/Player/GCFPlayerState.h
[GCFAbilitySystemComponent]: ../../Source/GameCoreFramework/Public/AbilitySystem/GCFAbilitySystemComponent.h

[GCFCharacterWithAbilities]: ../../Source/GameCoreFramework/Public/Actor/Character/GCFCharacterWithAbilities.h