# About this mod

Jiaye氏のCommunity Shaders AIOを基に、Cinematic DoFを単独で利用できるよう実装したSKSEプラグインです。Community Shaders本体は必須ではありません。

## 特徴

- 単体動作するSKSE統合と日本語／英語UI
- 3つのフォーカスモード：「ピント固定」、「画面位置AF」、「対象追従」
- コンソール選択対象の追従に加え、プレイヤーの自動選択・追従を追加
- 8つの編集可能なプリセットと配布時初期値への復元
- 手前／奥ピント範囲、手前／奥最大ぼかしの調整
- 会話中の被写体への自動フォーカス
- 1人称視点で手前ぼかしを無効にするオプション
- 未割り当てを初期値とするDoF ON/OFFキーボードホットキー

## 必要環境

- Skyrim AE 1.6.1170（実ゲーム確認済み環境）
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0（ゲーム内UIを使う場合のみ）

ビルドはSkyrim SE／AEを対象にしていますが、配布版の実ゲーム確認はAE 1.6.1170で行っています。Community Shadersは不要です。併用する場合は、二重適用を避けるためCommunity Shaders側のDepth of FieldをOFFにしてください。

## インストール

1. ZIPをMO2などのMOD管理ツールでインストールします。
2. Community Shadersを併用している場合は、Community Shaders側のDepth of FieldをOFFにします。

日本語表示には`Data/SKSE/Plugins/SKSEMenuFramework.ini`へ次の設定が必要です。

```ini
[Fonts]
EnableJapanese=true
```

## 基本操作

SKSE Menu Frameworkを導入している場合、ゲーム中にF1を押し、`Cinematic DoF Standalone` → `Depth of Field`を開きます。

- ピント固定：カメラからピント面までの距離を直接指定します。
- 画面位置AF：指定した画面X／Y位置の深度を読み取ります。
- 対象追従：人物は頭部ノード、非人物は対象の基準位置を投影した距離を使用します。
- ピント位置補正：マイナスでカメラ側、プラスで奥側へピント面を移動します。
- レンズ調整：焦点距離で前後の分離を大まかに決め、F値でピント範囲を整え、最後に手前／奥最大ぼかしで強さを決めると扱いやすくなります。
- DoFホットキー：「DoFを有効にする」の右側にあるホットキーボタンを押し、登録したいキーボードのキーを押します。Escで登録を中止、BackspaceまたはDeleteで登録を解除できます。

ホットキーの割り当てはINIへ保存され、プリセットには含まれません。ホットキーで切り替えたDoFのON/OFF状態は自動保存されません。Skyrim本体や他MODと同じキーを割り当てると、両方の操作が実行される場合があります。

## プリセットと保存

8枠は「ゲームプレイ」「シネマ」「視点連動」「人物撮影」「広角撮影」「1人称撮影」「Custom 1」「Custom 2」です。

- プリセット番号／適用：保存済み設定を現在映像へ適用します。プリセット枠と次回起動設定は変更しません。
- 現在値を保存：現在のDoF・会話フォーカス・対象追従設定を選択中のプリセット枠へ保存します。次回起動設定とUI言語は変更しません。
- 初期値へ戻す：2回押すと配布時初期値を保存せず映像へ適用します。
- 次回起動設定を保存：現在のDoF・会話フォーカス・対象追従設定とUI言語を次回起動用として保存します。プリセット枠は上書きしません。
- INIを再読み込み：保存済みの次回起動設定とプリセット枠を読み直します。

## クレジットとライセンス

- kota (@kotaSkyrim)：CinematicDoFStandalone
- Jiaye：original CinematicDOFを基にした大規模な編集・適応
- Frans Bouma (Otis / Infuse Project)：original CinematicDOF shader
- doodlum：target focus
- ProfJack：post-processing base structure
- Community Shaders team：Community Shaders base
- OpenAI Codex：開発支援

ファイル固有の表記がある場合を除き、本MODはGPL-3.0-or-laterにCommunity Shaders Modding ExceptionおよびGPL-3.0 Linking Exception with Corresponding Sourceを加えた条件で公開します。完全な条件と第三者著作物の詳細は、`COPYING`、`EXCEPTIONS.md`、`THIRD_PARTY_NOTICES.md`、`LICENSES`、同梱フォントのOFLを参照してください。

対応する完全なソースコードは、同じバージョン番号のsourceアーカイブに収録しています。ビルド手順と依存ソースの情報はソース内の`SOURCE_DEPENDENCIES.md`を参照してください。
