# CinematicDoFStandalone 1.0.2 RC2

## 日本語

この候補版は、実ゲームで安定した月保護を正式版向けに整理し、検証済みのシルエット撮影モードと遠景用ピント距離を統合したものです。RC2では実用域の操作性を優先してピント距離上限を2000mに整え、シルエットの初期配色を完全な黒／白にしました。

### 変更点

- 「空を鮮明に保つ」がONのとき、未描画深度の空に加えて、深度クリア値の直前へ書き込まれる月も保護します。月保護だけの独立スイッチは設けず、従来の空保護と同じ設定・プリセット値に連動します。
- 月はCoC計算、明るいボケ抽出、合成、最終スムージングの各段階で保護されます。診断用の水マスク、追加深度SRV、履歴バッファ、投影円は正式候補から除去しました。
- 独立した2色のシルエット撮影モードを追加しました。DoF本体がOFFでも動作し、DoF設定と9つのプリセットを変更しません。初期配色は完全な黒／白です。色と専用ホットキーはINIへ保存でき、すでに保存済みの配色は変更されません。
- 「ピント固定」の「ピント距離」を0.1～2000mへ拡張しました。保存値とシェーダー入力は従来どおり実メートル値のままです。UIだけを対数操作にして、近距離の調整精度を保っています。初期RCで保存した2000m超の値は安全に2000mへ制限されます。
- 既存INIとプリセットはそのまま読み込めます。150m以下の保存値は変換されず、見た目も変わりません。

### 月保護の実ゲーム確認済み条件

- メイン環境
- メイン環境（Community Shadersなし）
- 最小環境
- 最小環境（`bUse64bitsHDRRenderTarget=0`）
- 海、遠景水面、雪原、草原、樹木、岩、建物、巨大な月、画面端で一部だけ見える月

### RC1で実ゲーム確認済み

- シルエット小窓の開閉、2色変更、専用ホットキー、タイトル／ロード／マップでの停止
- 「ピント距離」の0.1mから2000m超までの操作、遠景へのピント、保存／再読み込み
- 「空を鮮明に保つ」ON/OFFで、空と月が同時に保護／解除されること
- 月、海、遠景、近景ボケ、絞り形状ボケ、人物・草木による前景遮蔽との組み合わせ

### RC2の差分確認

- 「ピント距離」の上端が2000mであること
- シルエット色が未保存の環境で、初期色が完全な黒／白であること

## English

This release candidate promotes the in-game-validated moon protection into a clean release path and combines it with the tested Silhouette Photo Mode and extended distant-focus control. RC2 refines the focus-distance cap to 2000 m for better control across the practical range and changes the default silhouette palette to pure black and white.

### Changes

- With `Keep Sky Sharp` enabled, the shader now protects both unwritten clear-depth sky and moons written immediately below the depth clear value. Moon protection intentionally shares the existing sky switch and preset value.
- Moon pixels are protected during CoC calculation, bright-bokeh extraction, compositing, and final smoothing. Diagnostic water masks, extra depth SRVs, history buffers, and projected circles are not present in this candidate.
- Added an independent two-colour Silhouette Photo Mode. It works while DoF itself is disabled and does not modify DoF settings or any of the nine presets. Its default palette is pure black and white. Colours and a dedicated hotkey can be saved to the INI, and existing saved colours are preserved.
- Extended Fixed Focus distance from 0.1 m to 2000 m. Saved values and shader input remain ordinary metre values; only the UI response is logarithmic so short-range precision is retained. Values above 2000 m saved by an earlier candidate are safely clamped to 2000 m.
- Existing INIs and presets remain compatible. Values at or below the former 150 m limit are neither converted nor visually changed.

### Moon-protection configurations already validated in game

- Main setup
- Main setup without Community Shaders
- Minimal setup
- Minimal setup with `bUse64bitsHDRRenderTarget=0`
- Sea, distant water, snowfields, grasslands, trees, rocks, buildings, very large moons, and partially off-screen moons

### Validated in game with RC1

- Silhouette window open/close, both colour controls, dedicated hotkey, and automatic suspension in title/loading/map menus
- Fixed Focus operation from 0.1 m through and beyond 2000 m, distant-scene focus, and save/reload
- `Keep Sky Sharp` enabling/disabling sky and moon protection together
- Moon, sea, distant scenery, near blur, aperture bokeh, and foreground occlusion by actors and vegetation

### RC2 delta checks

- Confirm that the Fixed Focus upper endpoint is 2000 m
- On a setup without saved silhouette colours, confirm that the initial palette is pure black and white
