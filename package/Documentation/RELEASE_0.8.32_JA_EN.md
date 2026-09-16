# CinematicDoFStandalone 0.8.32

## 日本語

0.8.32正式版は、個別に実機確認した次の変更を一つに統合しています。

- 絞り形状ボケは`Aperture Bokeh Test 4`の処理をそのまま採用します。
- `AccentuateWhites`だけを安全化版へ置き換えました。HDR/PBRの強い反射で分母がゼロへ近づく旧処理を廃止し、色比を保つ最大2倍の輝度ゲインへ制限します。
- Open Shaders FSR使用時は、出力テクスチャ全体ではなく実際の有効描画領域でDoFを処理します。Quality、PerformanceなどNative AA未満で右端・下端に出ていた境界と、ボケ量が内部解像度に応じて変化する問題を修正します。
- 評価した螺旋サンプル版と中央＋上下左右の固定5タップ版は含めていません。通常の同心円ギャザーとTest 4の単一プリフィルタを維持しています。
- 7番へ新しい「絞り形状」プリセットを追加し、従来のCustom 1／2は表示上の8／9番へ移しました。名前付きINIセクションと内部IDは変更していないため、旧版で保存したCustom 1／2は移動も上書きもされません。
- 新しい7番「絞り形状（試験的）」は、70.3 mm、f/3.4、奥最大ぼかし1.81、手前最大ぼかし1.00、品質8、距離補正1.00、5枚羽根、丸み0.10、形状0.65、Highlight Boost 0.45、仕上げの滑らかさ0.20を初期値とします。旧INIに`Preset.ApertureBokeh`が無い場合は内蔵初期値から生成されます。
- 通常時のDoFを止めて会話中だけ使用できる独立設定と、選択状態を常時示す`[English]`／`[日本語]`ボタンを追加しました。これらはプリセットの適用・保存・初期化から独立しています。
- 奥ボケのMip選択には、実機確認済みのFar Density Mip Test 2を採用しています。

### 既知の制限

非常に大きなボケを低い「ぼかし品質」で描画すると、ギャザーのサンプル密度が不足し、明るいボケが複数の輪や点へ分裂して見える場合があります。これは絞り形状やFSR有効領域修正の不具合ではなく、現在の品質設定とサンプル数による制限です。必要に応じて「ぼかし品質」を上げるか、手前／奥最大ぼかしを弱めてください。

### 実機確認結果

1. Native AAで通常時と会話時のDoFが従来通り動作することを確認済みです。
2. FSR Quality、Performance、その他の各プリセットで右端・下端の境界が出ず、Native AAと比べて不自然にボケ量が変わらないことを確認済みです。
3. PBRを含む強い反射環境で、Highlight Boostが暴走する白飛びや色ずれを起こさないことを確認済みです。
4. 絞り形状ボケON/OFF、近景／遠景、複数の品質設定でクラッシュ、黒画面、ちらつきがないことを確認済みです。
5. 旧INIのCustom 1／2が画面上の8／9番へ同じ値を保持し、新しい7番だけが追加されることを確認済みです。
6. 会話限定DoF、主スイッチ／ホットキー、対象追従、ピント固定、言語切替、9プリセットの保存・再読込が正常に動作することを確認済みです。

## English

Version 0.8.32 combines the following changes that were previously tested separately in game:

- Aperture-shaped bokeh keeps the `Aperture Bokeh Test 4` implementation unchanged.
- Only `AccentuateWhites` is replaced by the safety-tested implementation. The legacy near-zero HDR/PBR denominator is removed and replaced with a colour-preserving luminance gain capped at 2x.
- With Open Shaders FSR, the complete DoF pipeline now processes the actual active render area instead of the full output allocation. This fixes the right/bottom boundary and resolution-dependent blur strength seen below Native AA, including Quality and Performance modes.
- The evaluated spiral sampling and fixed centre/up/down/left/right five-tap prefilter are not included. The regular concentric-ring gather and Test 4 single-sample prefilter remain in use.
- A new Aperture Bokeh preset is inserted at UI slot 7, while the existing Custom 1/2 presets move visually to slots 8/9. Their named INI sections and stable internal IDs are unchanged, so values saved by an older release are neither migrated nor overwritten.
- The new experimental slot 7 defaults to 70.3 mm, f/3.4, 1.81 far maximum blur, 1.00 near maximum blur, quality 8, 1.00 distance compensation, five blades, 0.10 roundness, 0.65 shape strength, Highlight Boost 0.45, and 0.20 post-blur smoothing. If an older INI has no `Preset.ApertureBokeh` section, the built-in defaults supply it.
- A preset-independent mode can disable normal-gameplay DoF while retaining dialogue DoF. The compact `[English]` and `[日本語]` buttons always show the selected language. Preset apply/store/reset operations do not modify either setting.
- Far-plane Mip selection uses the in-game-validated Far Density Mip Test 2 implementation.

### Known limitation

At low `Blur Quality`, very large blur discs can split into visible rings or points because the gather does not have enough sample density. This is a limitation of the current quality/sample-count trade-off, not a defect in aperture shaping or the FSR active-area fix. Raise `Blur Quality` or reduce the near/far maximum blur when necessary.

### In-game verification

1. Ordinary and conversation DoF were verified at Native AA.
2. FSR Quality, Performance, and the other presets were verified without a right/bottom boundary or an unexpected blur-strength change relative to Native AA.
3. Strong highlights, including PBR scenes, were verified without runaway clipping or hue shift from Highlight Boost.
4. Aperture bokeh on/off, near/far blur, and multiple quality settings were verified without crashes, black frames, or flicker.
5. Custom 1/2 values from an older INI were verified to remain in UI slots 8/9 while only the new slot 7 is added.
6. Dialogue-only DoF, the master switch/hotkey, target tracking, fixed focus, language switching, and nine-preset save/reload behavior were verified in game.
