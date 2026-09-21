# Sky Highlight Exclusion Test 5 / 空の明点除外 Test 5

## 日本語

これは正式版ではなく、「空をぼかさない」と「明るいボケの強調」を併用した際に、樹木・山・建物と空の境界が白く発光する現象を調べる実験版です。Final Candidate 2を基準にしています。

### Final Candidate 2からの変更

- 「空をぼかさない」がONのとき、メイン深度で空と判定されたサンプルを、絞り形状ボケの明点蓄積から除外します。
- 空のサンプルは通常のぼけ平均には引き続き参加します。空境界のフェザー、水面判定、人物保護、焦点・CoC計算は変更していません。
- 遠景ぼけと手前ぼけの両方の明点蓄積に同じ除外を適用します。
- 「空をぼかさない」がOFFのときは従来どおり空も明点強調へ参加します。
- 「明るいボケの強調」が0、またはサンプルが明点候補でない場合は、明点除外用の深度参照をコード上で回避します。
- 新しいテクスチャ、深度コピー、レンダーパス、ディスパッチは追加していません。

このTestは、通常のぼけに含まれる明るい空そのものを消すのではなく、空を「拡張する明るいボケ」として選ばないための第一段階です。まだ薄い発光が残る場合は、別経路の全体明部増幅を空に対して抑える追加検証が必要です。

### 導入

1. Skyrimを終了します。
2. MO2でFinal Candidate 2より後に本Test版を配置し、本Test版のシェーダーが競合に勝つことを確認します。
3. Test 1～4を含む過去のTest版・診断版は同時に有効化しないでください。
4. INIは含まれません。保存済み設定、プリセット、言語設定は変更されません。
5. 「空をぼかさない」をON、「絞り形状ボケ」をONにし、「明るいボケの強調」を問題が見えた値へ戻して比較します。

### 優先確認箇所

- 明るい空を背景にした細い枝、葉、山稜、屋根の白い輪郭
- 「明るいボケの強調」0と0.36前後の比較
- 空ではない灯火、反射、魔法効果などの明るいボケが従来どおり残ること
- 「空をぼかさない」OFF時に従来の描画へ戻ること
- 海面と空、遠くの船・島・氷山の境界がFinal Candidate 2から悪化しないこと
- 手前ぼけ、遠景ぼけ、人物追従、会話中DoF、画面位置AF
- カメラ移動中のちらつき、瞬間的な白線、フレーム時間の変化

## English

This is an experimental build, not a release. It investigates the white glow that can appear along trees, mountains, and buildings against the sky when `Keep Sky Sharp` and a strong `Highlight Boost` are used together. It is based on Final Candidate 2.

### Changes from Final Candidate 2

- When `Keep Sky Sharp` is enabled, samples classified as clear sky by the main depth are excluded from the aperture-bokeh highlight accumulator.
- Sky samples still participate in the ordinary blur average. Sky-boundary feathering, water classification, actor protection, focus, and CoC calculations are unchanged.
- The same exclusion is applied to both far- and near-blur highlight accumulation.
- When `Keep Sky Sharp` is disabled, sky participates in highlight enhancement exactly as before.
- When `Highlight Boost` is zero, or a sample is not a highlight candidate, the shader guards the depth lookup used for highlight exclusion.
- No new texture, depth copy, render pass, or dispatch was added.

This test does not remove bright sky from the ordinary blur. It only prevents sky from being selected as an expanding bright-bokeh source. If a faint glow remains, a later test may need to suppress the separate whole-image highlight amplification path for sky pixels.

### Installation

1. Exit Skyrim.
2. In MO2, place this test after Final Candidate 2 and confirm that this test's shader wins conflicts.
3. Do not enable Test 1-4 or any earlier diagnostic/test build at the same time.
4. No INI is included. Saved settings, presets, and language selection are preserved.
5. Enable `Keep Sky Sharp` and aperture-shaped bokeh, then restore the `Highlight Boost` value that exposed the issue.

### Priority scenes

- White outlines around thin branches, leaves, ridgelines, and roofs against a bright sky
- A comparison between `Highlight Boost` 0 and approximately 0.36
- Non-sky lamps, reflections, and magic effects retaining their previous bright bokeh
- Disabling `Keep Sky Sharp` returning to the previous rendering behavior
- No regression at the sea/sky horizon or around distant ships, islands, and icebergs
- Near blur, far blur, target tracking, dialogue DoF, and screen-position autofocus
- Flicker, transient white lines, and frame-time changes while moving the camera
