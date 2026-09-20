# About this mod

Jiaye氏のCommunity Shaders AIOを基に、Cinematic DoFを単独で利用できるよう実装したSKSEプラグインです。Community Shaders本体は必須ではありません。

バージョン1.0.1は、実深度を併用する人物保護と、地形・水面との境界を滑らかに保ちながら空をDoFから除外する機能を追加します。「空を鮮明に保つ」が有効な場合は明るいボケの強調からも空を除外し、水平線の境目や明るい輪郭を抑えながら奥行き表現を維持します。Skyrim 1.6.1170／1.7.104対応と従来のプリセット互換性も維持します。

## 特徴

- 単体動作するSKSE統合と日本語／英語UI
- 3つのフォーカスモード：「ピント固定」、「画面位置AF」、「対象追従」
- コンソール選択対象の追従に加え、プレイヤーの自動選択・追従を追加
- 9つの編集可能なプリセットと配布時初期値への復元
- 手前／奥ピント範囲、手前／奥最大ぼかしの調整
- 会話中の被写体への自動フォーカス
- 1人称視点で手前ぼかしを無効にするオプション
- 地形・水面との境界を補正しながら空をDoFから除外するオプション
- 未割り当てを初期値とするDoF ON/OFFキーボードホットキー
- 羽根枚数、丸み、強さ、回転、明部強調を調整できる絞り形状ボケ

## 必要環境

- Skyrim AE 1.6.1170／1.7.104
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0（ゲーム内UIを使う場合のみ）

ビルドはSkyrim SE／AEを対象とします。1.0.1の新機能はAE 1.6.1170で実ゲーム確認済みです。1.7.104対応とAddress Library v5形式は、1.0.0で検証済みの経路を維持します。Community Shadersは不要です。併用する場合は、二重適用を避けるためCommunity Shaders側のDepth of FieldをOFFにしてください。

人物追従中はCommunity ShadersやHDR設定にかかわらず、投影した人物範囲と実深度を組み合わせて人物だけを保護します。Community Shaders併用時は人物追従中に限り手前ピント範囲の実効値を最低0.17 m、64-bit HDRが明示的に無効な場合は最低0.15 mにします。保存済みのINIやプリセット値は変更しません。

## インストール

1. 新規導入では`Full-MO2`版をインストールします。この版には配布時初期値のINIが含まれます。
2. 旧版から更新する場合は、INIを含まない`Update-Merge-MO2`版を使用し、MO2では既存MODへ`Merge`してください。`Replace`すると既存INIも削除されます。
3. 既存環境へ`Full-MO2`版を上書きすると、保存済み設定が配布時初期値へ置き換わります。
4. Community Shadersを併用している場合は、Community Shaders側のDepth of FieldをOFFにします。

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
- 空の除外：「詳細設定を表示」の右側にある「空を鮮明に保つ」で切り替えます。互換用の基準初期値はOFFです。既存INI／プリセットに項目がない場合もOFFとして読み込みます。

ホットキーの割り当てはINIへ保存され、プリセットには含まれません。ホットキーで切り替えたDoFのON/OFF状態は自動保存されません。Skyrim本体や他MODと同じキーを割り当てると、両方の操作が実行される場合があります。

## プリセットと保存

9枠は「ゲームプレイ」「シネマ」「視点連動」「人物撮影」「広角撮影」「1人称撮影」「絞り形状」「Custom 1」「Custom 2」です。旧版のCustom 1／2は名前付きINIセクションと内部IDを維持したまま、画面上の8／9番へ移るため、保存値は上書きされません。

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
