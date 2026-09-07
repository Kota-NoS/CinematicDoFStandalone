# CinematicDoFStandalone 0.8.31

## 日本語

Jiaye氏のCommunity Shaders AIOを基に、Cinematic DoFを単独で利用できるよう実装したSKSEプラグインです。Community Shaders本体は必須ではなく、未導入環境でも動作します。

### 特徴

- 単体動作するSKSE統合と日本語／英語UI
- 3つのフォーカスモード：「ピント固定」、「画面位置AF」、「対象追従」
- Community Shaders AIO由来のコンソール選択対象追従に加え、プレイヤーの自動選択・追従を追加
- 8つのプリセット、編集機能、配布時初期値への復元
- 手前／奥ピント範囲の調整
- 会話中の被写体への自動フォーカス
- 1人称視点で手前ぼかしを無効にするオプション
- 未割り当てを初期値とするDoF ON/OFFキーボードホットキー
- 通常プレイ、人物撮影、広角撮影、オブジェクト撮影向けの調整

### 必要環境

- Skyrim AE 1.6.1170（配布版の実ゲーム確認環境）
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0（ゲーム内UIを使う場合のみ）

ビルドはSkyrim SE／AEを対象にしていますが、配布版の実ゲーム確認はAE 1.6.1170で行っています。Community Shadersは不要です。併用する場合は、二重適用を避けるためCommunity Shaders側のDepth of FieldをOFFにしてください。

### インストール

1. ZIPをMO2などのMOD管理ツールでインストールします。
2. Community Shadersを併用している場合は、Community Shaders側のDepth of FieldをOFFにします。

日本語表示には次の設定が必要です。

```ini
; Data/SKSE/Plugins/SKSEMenuFramework.ini
[Fonts]
EnableJapanese=true
```

### 基本操作

SKSE Menu Frameworkを導入している場合、ゲーム中にF1を押し、`Cinematic DoF Standalone` → `Depth of Field`を開きます。

- ピント固定：カメラからピント面までの距離を直接指定します。
- 画面位置AF：指定した画面X／Y位置の深度を読み取ります。
- 対象追従：人物は頭部ノード、非人物は対象の基準位置を投影した距離を使用します。
- ピント位置補正：マイナスでカメラ側、プラスで奥側へピント面を移動します。
- レンズ調整：焦点距離で前後の分離を大まかに決め、F値でピント範囲を整え、最後に手前／奥最大ぼかしで強さを決めると扱いやすくなります。
- DoFホットキー：「DoFを有効にする」の右側にあるホットキーボタンを押し、登録したいキーボードのキーを押します。Escで登録を中止、BackspaceまたはDeleteで登録を解除できます。

ホットキーの割り当てはINIへ保存され、プリセットには含まれません。ホットキーで切り替えたDoFのON/OFF状態は自動保存されません。Skyrim本体や他MODと同じキーを割り当てると、両方の操作が実行される場合があります。

### プリセットとINI保存

次の8枠を用意し、使用例を配布時初期値として実装しています。

1. ゲームプレイ：遠距離ぼかし
2. シネマ：プレイヤーを中心に手前と奥をぼかす
3. 視点連動：画面の指定位置にピントを合わせる
4. 人物撮影：プレイヤーを中心に深いぼかし
5. 広角撮影：プレイヤーを中心に浅いぼかし
6. 1人称撮影：コンソールで選んだ対象をフォーカス
7. Custom 1：自由枠（初期は深い遠景ぼかし）
8. Custom 2：自由枠（初期は深い手前ぼかし）

- プリセット番号／「適用」：保存済みのDoF・会話フォーカス・対象追従設定を現在映像へ適用します。プリセット枠と次回起動設定は変更しません。
- 現在値を保存：現在映像へ適用中のDoF・会話フォーカス・対象追従設定を、そのプリセット枠へ保存します。次回起動設定とUI言語は変更しません。
- 初期値へ戻す：同じボタンを2回押すと、配布時の初期値を保存せず映像へ適用します。プリセット枠へ保存する場合は、その後「現在値を保存」を押します。
- 次回起動設定を保存：現在映像へ適用中のDoF・会話フォーカス・対象追従設定とUI言語を、次回起動時の設定として保存します。プリセット枠は上書きしません。
- INIを再読み込み：保存済みの次回起動設定とプリセット枠をINIから読み直します。

### 補足

- 1人称で通常プレイする場合は、詳細設定の「一人称の手前ぼかし」をOFFにすると武器や手元が鮮明になります。
- 「周辺ボケの強さ」は、画面周辺の既存ボケを接線方向へ引き延ばし、渦巻くレンズボケを再現します。
- 奥行きの大きい非人物オブジェクトでは、対象の基準位置が意図した表面と離れる場合があります。「ピント位置補正」または「手前／奥ピント範囲」で構図に合わせて調整してください。
- ログは`Documents/My Games/Skyrim Special Edition/SKSE/CinematicDoFStandalone.log`へ出力されます。

## English

CinematicDoFStandalone is an SKSE plugin that makes Cinematic DoF available as a standalone effect. It is based on Jiaye's Community Shaders AIO work, but Community Shaders itself is not required at runtime.

### Features

- Standalone SKSE integration with English and Japanese UI
- Three focus modes: Fixed Focus, Screen AF, and Target Tracking
- Console-selected target tracking inherited from the Community Shaders AIO work, plus automatic player selection and tracking
- Eight editable presets with bundled defaults
- Independently adjustable near and far in-focus ranges
- Automatic focus on the conversation subject during dialogue
- Option to disable near blur in first person
- An optional, unassigned-by-default keyboard hotkey for toggling DoF
- Presets designed for gameplay, portraits, wide shots, first-person shots, and object photography

### Requirements

- Skyrim AE 1.6.1170 (the runtime used for in-game release testing)
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0 only if you want the in-game UI

The build targets Skyrim SE and AE, but the distributed build was tested in game on AE 1.6.1170. Community Shaders is optional. If it is installed, disable its Depth of Field effect to avoid applying two DoF effects at once.

### Installation

1. Install the ZIP with MO2 or another mod manager.
2. If Community Shaders is installed, disable Depth of Field in Community Shaders.

For Japanese UI text, enable Japanese glyphs in `Data/SKSE/Plugins/SKSEMenuFramework.ini`:

```ini
[Fonts]
EnableJapanese=true
```

### Basic use

With SKSE Menu Framework installed, press F1 in game and open `Cinematic DoF Standalone` → `Depth of Field`.

- Fixed Focus: directly sets the distance from the camera to the focus plane.
- Screen AF: samples depth at the selected screen X/Y position.
- Target Tracking: actors use the projected head-node distance; non-actors use the projected reference/anchor distance.
- Target Focus Offset: negative values move the focus plane toward the camera; positive values move it farther away.
- Lens workflow: use focal length for broad depth separation, refine the in-focus range with the F-number, then set the final strength with the near and far maximum blur controls.
- DoF Hotkey: press the hotkey button beside `Enable DoF`, then press the keyboard key you want to assign. Esc cancels assignment; Backspace or Delete clears it.

The key assignment is saved to the INI and is not part of a preset. Toggling DoF with the hotkey does not automatically save the enabled state. If the same key is used by Skyrim or another mod, both actions may run.

### Presets and INI saving

The eight bundled slots are Gameplay, Cinematic, View Focus, Photo Portrait, Photo Wide, First-Person Photo, Custom 1, and Custom 2.

- Preset number / Apply: applies the stored DoF, dialogue-focus, and target-tracking values to the current image. It does not change the preset slot or startup settings.
- Store Current: saves the currently applied DoF, dialogue-focus, and target-tracking values in that preset slot. It does not change startup settings or the UI language.
- Restore Defaults: press twice to apply the bundled defaults without saving. Press Store Current afterward if you want to overwrite the preset slot.
- Save Startup Settings: saves the currently applied DoF, dialogue-focus, target-tracking, and UI-language values for the next launch. It does not overwrite any preset slot.
- Reload INI: reloads the saved startup settings and saved preset slots from the INI.

### Notes

- For normal first-person play, disable `First-Person Near Blur` in Advanced Settings to keep weapons and hands sharp.
- `Petzval Strength` stretches existing peripheral blur tangentially to create a swirling lens effect.
- A deep non-actor object can have a reference/anchor position away from the surface you intended to photograph. Compensate with Target Focus Offset or the near/far in-focus range controls.
- The log is written to `Documents/My Games/Skyrim Special Edition/SKSE/CinematicDoFStandalone.log`.

## Credits and license

- kota (@kotaSkyrim): CinematicDoFStandalone
- Jiaye: extensive edits and adaptation of the original CinematicDOF work
- Frans Bouma (Otis / Infuse Project): original CinematicDOF shader
- doodlum: target focus
- ProfJack: post-processing base structure
- Community Shaders team: Community Shaders base
- OpenAI Codex: development assistance

Unless a file carries a more specific notice, this project is distributed under GPL-3.0-or-later with the Community Shaders Modding Exception and the GPL-3.0 Linking Exception with Corresponding Source. See `COPYING`, `EXCEPTIONS.md`, `THIRD_PARTY_NOTICES.md`, `LICENSES`, and the bundled font's OFL for complete terms and third-party notices.

The complete corresponding source for each release is provided as a separate source archive with the same version number. Build instructions and dependency-source details are in `SOURCE_DEPENDENCIES.md` inside that archive.
