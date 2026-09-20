# Sky Highlight Amplification Exclusion Test 6 / 空の明部増幅除外 Test 6

## 日本語

これは正式版ではなく、「空をぼかさない」と「明るいボケの強調」を併用した際に、樹木・山・建物と空の境界へ残る白色または水色の発光を調べる実験版です。Test 5を基準にしています。

### Test 5からの変更

- Test 5の絞り形状ボケ用の空除外を維持します。
- 「空をぼかさない」がONのとき、メイン深度で空と判定された画素を、通常の明部増幅からも除外します。
- 除外は、半解像度の色を生成するプリブラーと、元画像を混合する最終コンバイナーの両方へ適用します。
- 地形・人物・水面・灯火・反射・魔法効果など、空ではない画素の「明るいボケの強調」は従来どおりです。
- 「空をぼかさない」がOFFのときは、空を含めてTest 5以前と同じ明部増幅になります。
- 「明るいボケの強調」が0のときは、この除外用の深度参照をコード上で回避します。
- 空境界のフェザー、水面判定、人物保護、焦点・CoC計算、プリセット値は変更していません。
- 新しいテクスチャ、深度コピー、レンダーパス、ディスパッチは追加していません。

### 導入

1. Skyrimを終了します。
2. MO2でFinal Candidate 2およびTest 5より後に本Test版を配置し、本Test版のシェーダーが競合に勝つことを確認します。
3. Test 1～5を含む過去のTest版・診断版は同時に有効化しないでください。
4. INIは含まれません。保存済み設定、プリセット、言語設定は変更されません。
5. 「空をぼかさない」をONにし、絞り形状ボケONのプリセットとOFFのプリセットの両方で、「明るいボケの強調」を問題が見えた値へ戻して比較します。

### 優先確認箇所

- 明るい空を背景にした細い枝、葉、山稜、屋根、人物の水色または白い輪郭
- 絞り形状ボケOFFのシネマプリセットで「明るいボケの強調」0.15～0.30の比較
- 絞り形状ボケONのプリセットで「明るいボケの強調」0.36～0.45の比較
- 空ではない灯火、水面反射、雪、魔法効果などの明るいボケが弱くなっていないこと
- 「空をぼかさない」OFF時に従来の描画へ戻ること
- 海面と空、遠くの船・島・氷山の境界がTest 5から悪化しないこと
- カメラ移動中のちらつき、瞬間的な白線、フレーム時間の変化

## English

This is an experimental build, not a release. It investigates the residual white or cyan glow around trees, mountains, buildings, and other silhouettes against the sky when `Keep Sky Sharp` and a strong `Highlight Boost` are used together. It is based on Test 5.

### Changes from Test 5

- Retains Test 5's sky exclusion for the aperture-bokeh highlight accumulator.
- When `Keep Sky Sharp` is enabled, pixels classified as clear sky by the main depth are also excluded from the general highlight amplifier.
- The exclusion is applied both while producing the half-resolution pre-blur colour and while mixing the original image in the final combiner.
- Highlight Boost remains unchanged for non-sky terrain, actors, water, lights, reflections, and magic effects.
- When `Keep Sky Sharp` is disabled, highlight amplification, including the sky, behaves as it did before this test.
- When `Highlight Boost` is zero, the shader guards the additional depth lookup used by this exclusion.
- Sky-boundary feathering, water classification, actor protection, focus, CoC calculations, and preset values are unchanged.
- No new texture, depth copy, render pass, or dispatch was added.

### Installation

1. Exit Skyrim.
2. In MO2, place this test after Final Candidate 2 and Test 5, and confirm that this test's shader wins conflicts.
3. Do not enable Test 1-5 or any earlier diagnostic/test build at the same time.
4. No INI is included. Saved settings, presets, and language selection are preserved.
5. Enable `Keep Sky Sharp`, then compare the problematic `Highlight Boost` values with aperture-shaped bokeh both enabled and disabled.

### Priority scenes

- White or cyan outlines around thin branches, leaves, ridgelines, roofs, and actors against a bright sky
- `Highlight Boost` comparisons from 0.15 to 0.30 in the Cinematic preset with aperture-shaped bokeh disabled
- `Highlight Boost` comparisons from approximately 0.36 to 0.45 with aperture-shaped bokeh enabled
- Non-sky lamps, water reflections, snow, and magic effects retaining their bright bokeh
- Disabling `Keep Sky Sharp` returning to the previous rendering behaviour
- No regression at the sea/sky horizon or around distant ships, islands, and icebergs
- Flicker, transient white lines, and frame-time changes while moving the camera
