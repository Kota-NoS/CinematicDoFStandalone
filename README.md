# CinematicDoFStandalone 1.0.11

> **1.0.11** fixes DoF coverage and hair blending with sub-native upscaling. See Documentation/RELEASE_1.0.11_JA_EN.md.

## 日本語

Jiaye氏のCommunity Shaders AIOを基に、Cinematic DoFを単独で利用できるよう実装したSKSEプラグインです。Community Shaders本体は必須ではなく、未導入環境でも動作します。

### 特徴

- 単体動作するSKSE統合と日本語／英語UI
- 3つのフォーカスモード：「ピント固定」、一人称／三人称で位置を個別保存する「画面位置AF」、「対象追従」
- Community Shaders AIO由来のコンソール選択対象追従に加え、プレイヤーの自動選択・追従を追加
- プレイヤー自動追従が一人称で停止している間、一人称画面AFへ任意で切り替える設定
- 10個のプリセット、編集機能、配布時初期値への復元
- 手前／奥ピント範囲の調整
- 会話中の被写体への自動フォーカス
- 通常時のDoFを止め、会話中だけDoFを使用する独立オプション
- 1人称視点で手前ぼかしを無効にするオプション
- 地形との境界を滑らかに保ちながら空と遠端深度の月をDoFから除外する、プリセット保存対応のオプション
- 髪の後ろの背景を自然にぼかす「髪を馴染ませる」。強く柔らかいモード1と、透過・形・細部をより保つモード2を選択可能
- DoF設定やプリセットを変更せず、深度を独立した2色画像へ変換するシルエット撮影モード
- 近距離の精度を保ったまま0.1～2000mを扱える、対数操作の「ピント距離」
- 未割り当てを初期値とするDoF ON/OFFキーボードホットキー
- 通常プレイ、人物撮影、広角撮影、オブジェクト撮影向けの調整
- 外部画像を使わず、手前／奥ぼかしのサンプル配置を直接変形し、強くぼけた明点を元の明るさの範囲内で形状として残す絞り形状ボケ
- 絞り・星・ハート、内部充填、回折光条、ボケ中心の補光、広域フレア抑制を備えた撮影向けの光学効果

### 必要環境

- Skyrim AE 1.6.1170／1.7.104
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0（ゲーム内UIを使う場合のみ）

ビルドはSkyrim SE／AEを対象とします。1.0.4の透過・HDR0・空境界処理はメイン環境と`bUse64bitsHDRRenderTarget=0`の最小環境で実ゲーム確認済みです。1.0.6以降はBottled Shadersの通常Z／Reverse Zの両方で、HDR0と「髪を馴染ませる」モード1・2を実ゲーム確認済みです。1.7.104対応とAddress Library v5形式は、1.0.0で検証済みの経路を維持します。Community Shaders／Bottled Shadersは不要です。併用する場合は、二重適用を避けるためシェーダー側のDepth of FieldをOFFにしてください。

人物追従中は、Community ShadersやHDR設定にかかわらず、画面上の人物範囲と実際の深度を組み合わせた共通の人物保護を使用します。接写時に深度を無視して頭部を強制保護する処理は使用せず、首周りや画面端の背景を人物として保護しないようにします。Community Shaders併用時も手前ピント範囲へ隠れた下限を設けず、`0.00 m`を含む保存値をそのまま使用します。DoFの距離判定には常にフル解像度のメイン深度を使用し、Community Shadersの有無やSAO・SSR・64-bit HDR設定によって別の深度へ切り替えません。

ピント境界のサンプルでは、手前と奥で大きく異なるCoCの色が混ざりにくいよう重みを制限します。また、有効な手前ボケの色サンプルが一つもない画素は空のレイヤーとして扱い、HDR0環境で人物周囲に現れていた太い黒膜を防ぎます。

Community Shadersを使わない環境でも同じ人物保護を使用します。非人物のコンソール追跡対象については、SSRまたは64-bit HDRが無効な従来条件でのみ保護します。64-bit HDRが明示的に無効で、有効な追従対象がいるフレームに限り、保存値を変更せず手前ピント範囲の実効値を最低0.15 mにします。

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

- ピント固定：カメラからピント面までの距離を0.1～2000mで直接指定します。スライダーは近距離の精度を保つ対数操作です。
- 一人称画面AF／三人称画面AF：指定した画面X／Y位置の深度を読み取ります。座標は視点ごとに個別保存され、視点変更時に自動で切り替わります。
- 対象追従：人物は頭部ノード、非人物は対象の基準位置を投影した距離を使用します。
- ピント位置補正：マイナスでカメラ側、プラスで奥側へピント面を移動します。
- レンズ調整：焦点距離で前後の分離を大まかに決め、F値でピント範囲を整え、最後に手前／奥最大ぼかしで強さを決めると扱いやすくなります。
- DoFホットキー：「DoFを有効にする」の右側にあるホットキーボタンを押し、登録したいキーボードのキーを押します。Escで登録を中止、BackspaceまたはDeleteで登録を解除できます。
- 会話限定DoF：「DoFを有効にする」をON、「会話外でもDoFを使用」をOFF、「会話中の被写体にピントを合わせる」をONにします。
- 空の除外：「詳細設定を表示」の右側にある「空を鮮明に保つ」で切り替えます。互換用の基準初期値はOFFです。
- 髪の馴染ませ：上部の「髪を馴染ませる」をONにし、1または2を選びます。1は強く柔らかく、2は透過と形をより保って馴染ませます。初期値はOFFです。
- 絞り形状と光条：絞り・星・ハートの形状ボケを選び、「内部を滑らかに埋める」で中心を補います。「回折光条」では本数・強さ・長さ・回転を調整でき、「広域フレア抑制」で広く一様なフレアから発生する光条を弱められます。
- シルエット撮影：DoFホットキー右側の白黒円アイコンから小窓を開き、前景色・背景色・専用ホットキーを設定します。初期色は完全な黒／白です。DoF本体がOFFでも使用できます。

ホットキーの割り当てはINIへ保存され、プリセットには含まれません。ホットキーで切り替えたDoF／シルエットのON/OFF状態は自動保存されません。Skyrim本体や他MODと同じキーを割り当てると、両方の操作が実行される場合があります。

### プリセットとINI保存

次の10枠を用意し、使用例を配布時初期値として実装しています。

1. ゲームプレイ：遠距離ぼかし
2. シネマ：プレイヤーを中心に手前と奥をぼかす
3. 視点連動：画面の指定位置にピントを合わせる
4. 人物撮影：プレイヤーを中心に深いぼかし
5. 広角撮影：プレイヤーを中心に浅いぼかし
6. 1人称撮影：コンソールで選んだ対象をフォーカス
7. 絞り形状：控えめな5枚羽根の形状ボケを確認する人物撮影設定
8. 光条：内部を滑らかに埋めた6枚羽根の形状ボケへ、6本の回折光条と広域フレア抑制を加える撮影設定
9. Custom 1：自由枠（初期は深い遠景ぼかし）
10. Custom 2：自由枠（初期は一定距離まで鮮明な箱庭風設定）

旧版から更新しても、Custom 1／2は従来の名前付きINIセクションと内部IDをそのまま使用し、表示上だけ9／10番へ移動します。7番は`Preset.ApertureBokeh`、新しい8番は`Preset.ShapeStarburst`を使用するため、既存の保存値は移動も上書きもされません。

- プリセット番号／「適用」：保存済みのDoF・会話フォーカス・対象追従設定を現在映像へ適用します。プリセット枠と次回起動設定は変更しません。
- 現在値を保存：現在映像へ適用中のDoF・会話フォーカス・対象追従設定を、そのプリセット枠へ保存します。次回起動設定とUI言語は変更しません。
- 初期値へ戻す：同じボタンを2回押すと、配布時の初期値を保存せず映像へ適用します。プリセット枠へ保存する場合は、その後「現在値を保存」を押します。
- 次回起動設定を保存：現在映像へ適用中の主スイッチ・会話外DoF・DoF・シルエット・会話フォーカス・TDMロックオン・スニークモード・髪の馴染ませ・対象追従設定とUI言語を、次回起動時の設定として保存します。プリセット枠は上書きしません。
- INIを再読み込み：保存済みの次回起動設定とプリセット枠をINIから読み直します。

### 補足

- 1人称で通常プレイする場合は、詳細設定の「一人称の手前ぼかし」をOFFにすると武器や手元が鮮明になります。
- プレイヤー自動追従は一人称で一時停止します。詳細設定の「一人称時は一人称画面AFを使用」をONにすると、その間だけ一人称用X／Y位置の画面AFを使い、三人称へ戻ると自動追従を再開します。コンソール選択対象の追従には影響しません。
- 「空を鮮明に保つ」は未描画の深度から空を判定し、同じスイッチで遠端深度の月も保護します。空色を山・建造物・樹木へ運ばず、空との境界だけを局所被覆率で滑らかに合成します。既存INIや既存プリセットにこの設定がない場合はOFFとして読み込みます。
- 「髪を馴染ませる」は通常描画と分離されています。OFFでは追加方式を実行しません。モード1は最も滑らかですが透過素材も柔らかくなり、モード2は透過・形・細部と月境界をより保ちます。
- シルエット撮影モードは現在のメイン深度だけで前景／背景を分類します。水面、月、半透明物、パーティクルは、そのフレームでメイン深度へ書き込まれたかどうかに従います。
- 「絞り形状ボケ」は初期状態でOFFです。ONにすると、強くピンぼけした明るい点が設定した絞り形状に近づきます。「明るいボケの強調」は、ぼかし平均で薄まる前の明点へ近づける量を調整します。元映像の明点を超える無制限な発光を追加する機能ではありません。
- 「内部を滑らかに埋める」は、絞り形状の輪郭だけでなく中央まで高密度で描画する撮影向けオプションです。ONでは検証済みの単一品質を使用し、OFFでは従来の描画と負荷を維持します。
- 「ボケ形状」では通常の絞りに加えて星・ハートを選択できます。星では「絞り羽根枚数」が頂点数になります。「形状の丸み」「形状の強さ」「形状の回転」はすべての形状に使用できます。
- 非常に大きなボケを低い「ぼかし品質」で描画すると、サンプル密度不足によりボケが輪や点へ分裂して見える場合があります。これは既知の制限です。必要に応じて品質を上げるか、最大ぼかしを弱めてください。
- 「周辺ボケの強さ」は、画面周辺の既存ボケを接線方向へ引き延ばし、渦巻くレンズボケを再現します。
- 奥行きの大きい非人物オブジェクトでは、対象の基準位置が意図した表面と離れる場合があります。「ピント位置補正」または「手前／奥ピント範囲」で構図に合わせて調整してください。
- ログは`Documents/My Games/Skyrim Special Edition/SKSE/CinematicDoFStandalone.log`へ出力されます。

## English

CinematicDoFStandalone is an SKSE plugin that makes Cinematic DoF available as a standalone effect. It is based on Jiaye's Community Shaders AIO work, but Community Shaders itself is not required at runtime.

### Features

- Standalone SKSE integration with English and Japanese UI
- Three focus modes: Fixed Focus, Screen AF with separately stored first-/third-person positions, and Target Tracking
- Console-selected target tracking inherited from the Community Shaders AIO work, plus automatic player selection and tracking
- An optional 1st-Person AF fallback while automatic player tracking is paused in first person
- Ten editable presets with bundled defaults
- Independently adjustable near and far in-focus ranges
- Automatic focus on the conversation subject during dialogue
- A preset-independent option to use DoF only during dialogue
- Option to disable near blur in first person
- A preset-saved option that excludes clear-depth sky and far-depth moon pixels from DoF while feathering the boundary against geometry
- Optional `Blend Hair` processing with a strong soft mode and a second mode that preserves more transparency, shape, and detail
- An independent two-colour Silhouette Photo Mode that does not modify DoF settings or presets
- A logarithmic Fixed Focus distance control covering 0.1-2000 m without sacrificing short-range precision
- An optional, unassigned-by-default keyboard hotkey for toggling DoF
- Presets designed for gameplay, portraits, wide shots, first-person shots, and object photography
- Aperture-shaped bokeh that deforms near/far blur samples and can selectively emphasize bright shaped bokeh, with adjustable blade count, roundness, strength, and rotation; no external mask image is required
- Screenshot-oriented aperture, star, and heart bokeh with filled sampling, diffraction starburst, bokeh core fill, and wide-flare suppression

### Requirements

- Skyrim AE 1.6.1170 or 1.7.104
- SKSE64
- Address Library for SKSE Plugins
- SKSE Menu Framework 3.8.0 only if you want the in-game UI

The build targets Skyrim SE and AE. Version 1.0.4's transparency, HDR0, and sky-boundary changes were tested in game with the main setup and a minimal setup using `bUse64bitsHDRRenderTarget=0`. Skyrim 1.7.104 support and the Address Library v5 path retain the route validated for version 1.0.0. Community Shaders is optional. If it is installed, disable its Depth of Field effect to avoid applying two DoF effects at once.

While an actor is tracked, one subject-protection path combines the projected actor area with measured depth regardless of Community Shaders or HDR settings. It does not use the former close-up head override that ignored depth, preventing the projected head circle from protecting neck gaps or edge-of-frame background. With Community Shaders, there is no hidden minimum near-focus range: the saved value, including `0.00 m`, is used unchanged. DoF distance classification always uses the full-resolution main depth and does not switch to another depth buffer based on Community Shaders, SAO, SSR, or 64-bit HDR settings.

Samples across a focus boundary are weighted to avoid mixing colours with strongly incompatible near/far CoC values. If a near-blur pixel has no valid colour samples, it is treated as an empty layer, preventing the thick dark membrane previously visible around subjects in HDR0 configurations.

The same actor protection is used without Community Shaders. Non-actor console targets retain the legacy protection only when SSR or 64-bit HDR is disabled. When 64-bit HDR is explicitly disabled and a valid tracked target exists, the effective near-focus range is floored at 0.15 m without changing the saved value.

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

- Fixed Focus: directly sets the camera-to-focus-plane distance from 0.1 to 2000 m. Its logarithmic response retains precise short-range adjustment.
- 1st-Person AF / 3rd-Person AF: samples depth at the selected screen X/Y position. Each view stores its own coordinates and switches automatically with perspective.
- Target Tracking: actors use the projected head-node distance; non-actors use the projected reference/anchor distance.
- Target Focus Offset: negative values move the focus plane toward the camera; positive values move it farther away.
- Lens workflow: use focal length for broad depth separation, refine the in-focus range with the F-number, then set the final strength with the near and far maximum blur controls.
- DoF Hotkey: press the hotkey button beside `Enable DoF`, then press the keyboard key you want to assign. Esc cancels assignment; Backspace or Delete clears it.
- Dialogue-only DoF: enable `Enable DoF`, disable `Use DoF outside dialogue`, and keep dialogue focus enabled.
- Sky exclusion: toggle `Keep Sky Sharp` to the right of `Show Advanced Settings`. Its compatibility default is off.
- Hair blending: enable `Blend Hair` in the top row, then select 1 or 2. Mode 1 is stronger and softer; mode 2 preserves more transparency and shape. It defaults to off.
- Aperture shapes and starburst: choose aperture, star, or heart bokeh and use `Fill Aperture Interior` to reinforce the centre. Diffraction Starburst exposes ray count, strength, length, and rotation; `Wide-Flare Suppression` attenuates rays generated from broad, nearly uniform flare.
- Silhouette Photo Mode: open its compact window with the black/white circle icon beside the DoF hotkey, then choose foreground/background colours and an optional hotkey. Its default palette is pure black and white. It also works while DoF itself is off.

Hotkey assignments are saved to the INI and are not part of a preset. Toggling DoF or Silhouette Mode with a hotkey does not automatically save the enabled state. If the same key is used by Skyrim or another mod, both actions may run.

### Presets and INI saving

The ten bundled slots are Gameplay, Cinematic, View Focus, Photo Portrait, Photo Wide, First-Person Photo, Aperture Bokeh, Starburst, Custom 1, and Custom 2. The eighth slot combines filled six-blade bokeh, six diffraction rays, and moderate wide-flare suppression as an approachable starting point for the new controls.

When updating from an older version, Custom 1 and Custom 2 keep their original named INI sections and stable internal IDs and only move visually to slots 9 and 10. Slot 7 uses `Preset.ApertureBokeh`, while the new eighth slot uses `Preset.ShapeStarburst`, so existing saved values are neither moved nor overwritten.

- Preset number / Apply: applies the stored DoF, dialogue-focus, and target-tracking values to the current image. It does not change the preset slot or startup settings.
- Store Current: saves the currently applied DoF, dialogue-focus, and target-tracking values in that preset slot. It does not change startup settings or the UI language.
- Restore Defaults: press twice to apply the bundled defaults without saving. Press Store Current afterward if you want to overwrite the preset slot.
- Save Startup Settings: saves the master switch, normal-gameplay mode, DoF, silhouette, dialogue-focus, TDM lock-on, Sneak Mode, hair blending, target-tracking, and UI-language values for the next launch. It does not overwrite any preset slot.
- Reload INI: reloads the saved startup settings and saved preset slots from the INI.

### Notes

- For normal first-person play, disable `First-Person Near Blur` in Advanced Settings to keep weapons and hands sharp.
- Automatic player tracking pauses in first person. Enable `Use 1st-Person AF` in Advanced Settings to use the first-person X/Y Screen AF position while paused; returning to third person resumes tracking. Console-selected target tracking is unaffected.
- `Keep Sky Sharp` identifies sky from unwritten depth and protects far-depth moon pixels through the same switch. It rejects sky-color leakage into geometry and uses local coverage for a soft one-pixel sky boundary. Existing INIs and presets without the setting load it as disabled.
- `Blend Hair` is separate from standard rendering and does not execute while disabled. Mode 1 gives the smoothest blend but also softens transparency; mode 2 preserves more transparency, shape, detail, and moon boundaries.
- Silhouette Photo Mode classifies foreground/background solely from the current main depth. Water, moons, transparent objects, and particles follow whether they wrote main depth in that frame.
- `Aperture Bokeh` is off by default. When enabled, strongly defocused bright points take on the selected aperture shape. `Highlight Boost` controls how far the blurred result moves toward the brightest eligible shaped sample. It does not add unrestricted brightness beyond the sampled source highlight.
- `Fill Aperture Interior` is a screenshot-oriented option that increases sampling across the whole aperture, including its centre. On uses the single validated quality level; Off preserves the previous rendering path and cost.
- `Bokeh Shape` offers the regular aperture, star, and heart silhouettes. For stars, `Aperture Blades` controls the number of points. `Shape Roundness`, `Shape Strength`, and `Shape Rotation` apply to every silhouette.
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
