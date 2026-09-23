# CinematicDoFStandalone 1.0.2 RC1

## 日本語

この候補版は、実ゲームで安定した月保護を正式版向けに整理し、検証済みのシルエット撮影モードと遠景用ピント距離を統合したものです。

### 変更点

- 「空を鮮明に保つ」がONのとき、未描画深度の空に加えて、深度クリア値の直前へ書き込まれる月も保護します。月保護だけの独立スイッチは設けず、従来の空保護と同じ設定・プリセット値に連動します。
- 月はCoC計算、明るいボケ抽出、合成、最終スムージングの各段階で保護されます。診断用の水マスク、追加深度SRV、履歴バッファ、投影円は正式候補から除去しました。
- 独立した2色のシルエット撮影モードを追加しました。DoF本体がOFFでも動作し、DoF設定と9つのプリセットを変更しません。色と専用ホットキーはINIへ保存できます。
- 「ピント固定」の「ピント距離」を0.1～4000mへ拡張しました。保存値とシェーダー入力は従来どおり実メートル値のままです。UIだけを対数操作にして、近距離の調整精度を保っています。
- 既存INIとプリセットはそのまま読み込めます。150m以下の保存値は変換されず、見た目も変わりません。

### 月保護の実ゲーム確認済み条件

- メイン環境
- メイン環境（Community Shadersなし）
- 最小環境
- 最小環境（`bUse64bitsHDRRenderTarget=0`）
- 海、遠景水面、雪原、草原、樹木、岩、建物、巨大な月、画面端で一部だけ見える月

### RC1で追加確認する項目

- シルエット小窓の開閉、2色変更、専用ホットキー、タイトル／ロード／マップでの停止
- 「ピント距離」の150m、500m、1000m、2000m、4000m付近での操作と保存／再読み込み
- 「空を鮮明に保つ」ON/OFFで、空と月が同時に保護／解除されること

## English

This release candidate promotes the in-game-validated moon protection into a clean release path and combines it with the tested Silhouette Photo Mode and extended distant-focus control.

### Changes

- With `Keep Sky Sharp` enabled, the shader now protects both unwritten clear-depth sky and moons written immediately below the depth clear value. Moon protection intentionally shares the existing sky switch and preset value.
- Moon pixels are protected during CoC calculation, bright-bokeh extraction, compositing, and final smoothing. Diagnostic water masks, extra depth SRVs, history buffers, and projected circles are not present in this candidate.
- Added an independent two-colour Silhouette Photo Mode. It works while DoF itself is disabled and does not modify DoF settings or any of the nine presets. Colours and a dedicated hotkey can be saved to the INI.
- Extended Fixed Focus distance from 0.1 m to 4000 m. Saved values and shader input remain ordinary metre values; only the UI response is logarithmic so short-range precision is retained.
- Existing INIs and presets remain compatible. Values at or below the former 150 m limit are neither converted nor visually changed.

### Moon-protection configurations already validated in game

- Main setup
- Main setup without Community Shaders
- Minimal setup
- Minimal setup with `bUse64bitsHDRRenderTarget=0`
- Sea, distant water, snowfields, grasslands, trees, rocks, buildings, very large moons, and partially off-screen moons

### Additional RC1 checks

- Silhouette window open/close, both colour controls, dedicated hotkey, and automatic suspension in title/loading/map menus
- Fixed Focus operation and save/reload near 150 m, 500 m, 1000 m, 2000 m, and 4000 m
- Confirm that `Keep Sky Sharp` enables/disables sky and moon protection together
