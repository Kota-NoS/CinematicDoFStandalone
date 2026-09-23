# CinematicDoFStandalone 1.0.2

## 日本語

1.0.2では、月を含む空の保護、独立したシルエット撮影モード、遠景まで扱いやすい固定ピント操作を正式版として統合しました。

### 主な変更点

- 「空を鮮明に保つ」がONのとき、未描画深度の空に加えて、深度クリア値の直前へ書き込まれる月も保護します。月保護は既存の空保護設定と各プリセットに連動します。
- 月はCoC計算、明るいボケ抽出、合成、最終スムージングの各段階で保護されます。開発中に使用した診断用の水マスク、追加深度SRV、履歴バッファ、投影円は公開版に含まれません。
- DoF本体とは独立した2色のシルエット撮影モードを追加しました。DoFがOFFでも動作し、DoF設定と9つのプリセットを変更しません。
- シルエットの初期配色は完全な黒／白です。色と専用ホットキーはINIへ保存でき、すでに保存済みの配色は維持されます。
- シルエットはゲームの最終フレームバッファへ出力し、照明や天候の色が結果へ混ざりにくい純色出力にしました。
- 「ピント固定」の「ピント距離」を0.1～2000mへ拡張しました。保存値とシェーダー入力は実メートル値のまま、UIだけを対数操作にして近距離の調整精度も保っています。
- 既存INIとプリセットはそのまま読み込めます。旧上限150m以下の保存値は変換されず、見た目も変わりません。以前のテスト版で保存した2000m超の値は2000mへ制限されます。

### 実ゲームで確認済み

- メイン環境、Community Shadersなしのメイン環境、最小環境、`bUse64bitsHDRRenderTarget=0`の最小環境
- 海、遠景水面、雪原、草原、樹木、岩、建物、巨大な月、画面端で一部だけ見える月
- シルエット小窓の開閉、2色変更、専用ホットキー、タイトル／ロード／マップでの停止
- 純粋な黒／白のシルエット出力（Community Shadersあり／なし）
- 0.1～2000mの固定ピント操作、遠景へのピント、保存／再読み込み
- 空保護ON/OFF、月、近景ボケ、絞り形状ボケ、人物・草木による前景遮蔽との組み合わせ

### 補足

ReShadeや外部の表示系アドオンがゲーム出力後に色変換を行う場合、ゲーム内表示だけに色味が加わり、保存されたスクリーンショットと異なることがあります。これはシルエット色の保存値とは独立した後段処理です。

## English

Version 1.0.2 combines sky and moon protection, an independent Silhouette Photo Mode, and practical long-distance fixed-focus control into the public release.

### Highlights

- With `Keep Sky Sharp` enabled, the shader protects both unwritten clear-depth sky and moons written immediately below the depth clear value. Moon protection follows the existing sky option and each preset.
- Moon pixels are protected during CoC calculation, bright-bokeh extraction, compositing, and final smoothing. Development-only water masks, extra depth SRVs, history buffers, and projected circles are not included.
- Added an independent two-colour Silhouette Photo Mode. It works while DoF is disabled and does not alter DoF settings or any of the nine presets.
- The default silhouette palette is pure black and white. Colours and the dedicated hotkey can be saved to the INI; previously saved colours remain unchanged.
- Silhouette output is written to the game's final framebuffer so lighting and weather colour are less likely to contaminate the requested solid colours.
- Extended Fixed Focus distance to 0.1–2000 m. Saved values and shader input remain ordinary metre values; only the UI response is logarithmic to preserve short-range precision.
- Existing INIs and presets remain compatible. Values at or below the former 150 m limit are unchanged, while values above 2000 m saved by an earlier test build are clamped safely.

### Validated in game

- Main setup, main setup without Community Shaders, minimal setup, and minimal setup with `bUse64bitsHDRRenderTarget=0`
- Sea, distant water, snowfields, grasslands, trees, rocks, buildings, very large moons, and partially off-screen moons
- Silhouette window open/close, both colour controls, dedicated hotkey, and suspension in title/loading/map menus
- Pure black-and-white silhouette output with and without Community Shaders
- Fixed Focus operation across 0.1–2000 m, distant-scene focus, and save/reload
- Sky-protection toggling, moons, near blur, aperture bokeh, and foreground occlusion by actors and vegetation

### Note

ReShade or another display add-on may apply colour processing after the game has produced its frame. In that case, the live image can be tinted even when the saved screenshot contains the requested pure colours; this is separate from the silhouette colour values stored by the plugin.
