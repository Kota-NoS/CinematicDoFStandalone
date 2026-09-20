# CinematicDoFStandalone 1.0.0

> **Version 1.0.0:** The first stable release completes aperture-shaped bokeh, officially supports Skyrim 1.7.104 and its Address Library v5 database, and adds narrowly scoped subject protection for Skyrim display configurations that otherwise blur a tracked character. Skyrim 1.6.1170 compatibility and the completed 0.8.32 rendering baseline are preserved.

## 日本語

Jiaye氏のCommunity Shaders AIOを基に、Cinematic DoFを単独で利用できるよう実装したSKSEプラグインです。Community Shaders本体は必須ではなく、未導入環境でも動作します。

### 特徴

- 単体動作するSKSE統合と日本語／英語UI
- 3つのフォーカスモード：「ピント固定」、「画面位置AF」、「対象追従」
- Community Shaders AIO由来のコンソール選択対象追従に加え、プレイヤーの自動選択・追従を追加
- 9つのプリセット、編集機能、配布時初期値への復元
- 手前／奥ピント範囲の調整
- 会話中の被写体への自動フォーカス
- 通常時のDoFを止め、会話中だけDoFを使用する独立オプション
- 1人称視点で手前ぼかしを無効にするオプション
- 地形との境界を滑らかに保ちながら空をDoFから除外する、プリセット保存対応のオプション
- 未割り当てを初期値とするDoF ON/OFFキーボードホットキー
- 通常プレイ、人物撮影、広角撮影、オブジェクト撮影向けの調整
- 外部画像を使わず、手前／奥ぼかしのサンプル配置を直接変形し、強くぼけた明点を元の明るさの範囲内で形状として残す絞り形状ボケ

### 必要環境

- Skyrim AE 1.6.1170／1.7.104（配布版の実ゲーム確認環境）
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0（ゲーム内UIを使う場合のみ）

ビルドはSkyrim SE／AEを対象とし、配布版はAE 1.6.1170と1.7.104で実ゲーム確認済みです。1.7.104ではAddress Library v5形式へ対応します。Community Shadersは不要です。併用する場合は、二重適用を避けるためCommunity Shaders側のDepth of FieldをOFFにしてください。

Community Shadersを使わない環境では、SSRまたは64-bit HDRが無効な場合に対象追従中の人物保護を有効にします。64-bit HDRが明示的に無効で、有効な追従対象がいるフレームに限り、保存値を変更せず手前ピント範囲の実効値を最低0.15 mにします。64-bit HDRとSSRが有効な通常環境では、これらの保護は無効です。Community Shaders使用時の既存深度フォールバック条件は変更していません。

### インストール

1. 新規導入では`Full-MO2`版をMO2などのMOD管理ツールでインストールします。この版には配布時初期値の`CinematicDoFStandalone.ini`が含まれます。
2. 旧版から更新する場合は、INIを含まない`Update-Merge-MO2`版を使用し、MO2では既存MODへ`Merge`してください。`Replace`すると既存INIも削除されるため使用しないでください。
3. 既存環境へ`Full-MO2`版を上書きすると、保存済み設定が配布時初期値へ置き換わります。
4. Community Shadersを併用している場合は、Community Shaders側のDepth of FieldをOFFにします。

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
- 会話限定DoF：「DoFを有効にする」をON、「会話外でもDoFを使用」をOFF、「会話中の被写体にピントを合わせる」をONにします。
- 空の除外：「詳細設定を表示」の右側にある「空をぼかさない」で切り替えます。初期値はONです。

ホットキーの割り当てはINIへ保存され、プリセットには含まれません。ホットキーで切り替えたDoFのON/OFF状態は自動保存されません。Skyrim本体や他MODと同じキーを割り当てると、両方の操作が実行される場合があります。

### プリセットとINI保存

次の9枠を用意し、使用例を配布時初期値として実装しています。

1. ゲームプレイ：遠距離ぼかし
2. シネマ：プレイヤーを中心に手前と奥をぼかす
3. 視点連動：画面の指定位置にピントを合わせる
4. 人物撮影：プレイヤーを中心に深いぼかし
5. 広角撮影：プレイヤーを中心に浅いぼかし
6. 1人称撮影：コンソールで選んだ対象をフォーカス
7. 絞り形状：控えめな5枚羽根の形状ボケを確認する人物撮影設定
8. Custom 1：自由枠（初期は深い遠景ぼかし）
9. Custom 2：自由枠（初期は深い手前ぼかし）

旧版から更新しても、Custom 1／2は従来の名前付きINIセクションと内部IDをそのまま使用します。新しい7番は独立した`Preset.ApertureBokeh`として追加されるため、既存のCustom 1／2の保存値は移動も上書きもされません。

- プリセット番号／「適用」：保存済みのDoF・会話フォーカス・対象追従設定を現在映像へ適用します。プリセット枠と次回起動設定は変更しません。
- 現在値を保存：現在映像へ適用中のDoF・会話フォーカス・対象追従設定を、そのプリセット枠へ保存します。次回起動設定とUI言語は変更しません。
- 初期値へ戻す：同じボタンを2回押すと、配布時の初期値を保存せず映像へ適用します。プリセット枠へ保存する場合は、その後「現在値を保存」を押します。
- 次回起動設定を保存：現在映像へ適用中の主スイッチ・会話外DoF・DoF・会話フォーカス・対象追従設定とUI言語を、次回起動時の設定として保存します。プリセット枠は上書きしません。
- INIを再読み込み：保存済みの次回起動設定とプリセット枠をINIから読み直します。

### 補足

- 1人称で通常プレイする場合は、詳細設定の「一人称の手前ぼかし」をOFFにすると武器や手元が鮮明になります。
- 「空をぼかさない」は未描画の深度から空を判定します。既存INIや既存プリセットにこの設定がない場合はONとして読み込みます。月など深度を書き込む天体はぼける場合があります。
- 「絞り形状ボケ」は初期状態でOFFです。ONにすると、強くピンぼけした明るい点が設定した絞り形状に近づきます。「明るいボケの強調」は、ぼかし平均で薄まる前の明点へ近づける量を調整します。元映像の明点を超える無制限な発光を追加する機能ではありません。
- 非常に大きなボケを低い「ぼかし品質」で描画すると、サンプル密度不足によりボケが輪や点へ分裂して見える場合があります。これは既知の制限です。必要に応じて品質を上げるか、最大ぼかしを弱めてください。
- 「周辺ボケの強さ」は、画面周辺の既存ボケを接線方向へ引き延ばし、渦巻くレンズボケを再現します。
- 奥行きの大きい非人物オブジェクトでは、対象の基準位置が意図した表面と離れる場合があります。「ピント位置補正」または「手前／奥ピント範囲」で構図に合わせて調整してください。
- ログは`Documents/My Games/Skyrim Special Edition/SKSE/CinematicDoFStandalone.log`へ出力されます。

## English

CinematicDoFStandalone is an SKSE plugin that makes Cinematic DoF available as a standalone effect. It is based on Jiaye's Community Shaders AIO work, but Community Shaders itself is not required at runtime.

### Features

- Standalone SKSE integration with English and Japanese UI
- Three focus modes: Fixed Focus, Screen AF, and Target Tracking
- Console-selected target tracking inherited from the Community Shaders AIO work, plus automatic player selection and tracking
- Nine editable presets with bundled defaults
- Independently adjustable near and far in-focus ranges
- Automatic focus on the conversation subject during dialogue
- A preset-independent option to use DoF only during dialogue
- Option to disable near blur in first person
- A preset-saved option that excludes the sky from DoF while feathering its boundary against geometry
- An optional, unassigned-by-default keyboard hotkey for toggling DoF
- Presets designed for gameplay, portraits, wide shots, first-person shots, and object photography
- Aperture-shaped bokeh that deforms near/far blur samples and can selectively emphasize bright shaped bokeh, with adjustable blade count, roundness, strength, and rotation; no external mask image is required

### Requirements

- Skyrim AE 1.6.1170 or 1.7.104 (the runtimes used for in-game release testing)
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0 only if you want the in-game UI

The build targets Skyrim SE and AE, and the distributed build was tested in game on AE 1.6.1170 and 1.7.104. Version 1.7.104 uses the Address Library v5 database format supported by this release. Community Shaders is optional. If it is installed, disable its Depth of Field effect to avoid applying two DoF effects at once.

Without Community Shaders, target-tracking subject protection activates when SSR or 64-bit HDR is disabled. When 64-bit HDR is explicitly disabled and a valid tracked target exists, the effective near-focus range is floored at 0.15 m without changing the saved value. Both protections are disabled in a normal standalone configuration with 64-bit HDR and SSR enabled. The established Community Shaders depth-fallback conditions are unchanged.

### Installation

1. For a new installation, install the `Full-MO2` package with MO2 or another mod manager. It includes `CinematicDoFStandalone.ini` with the bundled defaults.
2. When updating from an older version, use the INI-free `Update-Merge-MO2` package and choose `Merge` into the existing mod in MO2. Do not use `Replace`, because it also deletes the existing INI.
3. Installing the `Full-MO2` package over an existing setup replaces the user's saved settings with the bundled defaults.
4. If Community Shaders is installed, disable Depth of Field in Community Shaders.

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
- Dialogue-only DoF: enable `Enable DoF`, disable `Use DoF outside dialogue`, and keep dialogue focus enabled.
- Sky exclusion: toggle `Keep Sky Sharp` to the right of `Show Advanced Settings`. It defaults to on.

The key assignment is saved to the INI and is not part of a preset. Toggling DoF with the hotkey does not automatically save the enabled state. If the same key is used by Skyrim or another mod, both actions may run.

### Presets and INI saving

The nine bundled slots are Gameplay, Cinematic, View Focus, Photo Portrait, Photo Wide, First-Person Photo, Aperture Bokeh, Custom 1, and Custom 2.

When updating from an older version, Custom 1 and Custom 2 keep their original named INI sections and stable internal IDs. The new seventh slot uses the independent `Preset.ApertureBokeh` section, so existing Custom 1/2 values are neither moved nor overwritten.

- Preset number / Apply: applies the stored DoF, dialogue-focus, and target-tracking values to the current image. It does not change the preset slot or startup settings.
- Store Current: saves the currently applied DoF, dialogue-focus, and target-tracking values in that preset slot. It does not change startup settings or the UI language.
- Restore Defaults: press twice to apply the bundled defaults without saving. Press Store Current afterward if you want to overwrite the preset slot.
- Save Startup Settings: saves the master switch, normal-gameplay mode, DoF, dialogue-focus, target-tracking, and UI-language values for the next launch. It does not overwrite any preset slot.
- Reload INI: reloads the saved startup settings and saved preset slots from the INI.

### Notes

- For normal first-person play, disable `First-Person Near Blur` in Advanced Settings to keep weapons and hands sharp.
- `Keep Sky Sharp` identifies the sky from unwritten depth. Existing INIs and presets without this setting load it as enabled. Moons and other sky objects that write depth can still be blurred.
- `Aperture Bokeh` is off by default. When enabled, strongly defocused bright points take on the selected aperture shape. `Highlight Boost` controls how far the blurred result moves toward the brightest eligible shaped sample. It does not add unrestricted brightness beyond the sampled source highlight.
- Very large blur discs rendered at low `Blur Quality` can separate into visible rings or points because the gather has insufficient sample density. This is a known limitation. Raise quality or reduce maximum blur if needed.
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
