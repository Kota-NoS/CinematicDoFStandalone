# Raw Depth Moon Mask Test 27 / 生深度・月マスク Test 27

## 日本語

これは正式版ではありません。Test 26の海岸4地点では、月の生深度とクリア値`1.0`との差が常に`1e-7～3e-7`、最遠の海面・地形でも主に`3e-6`以上でした。Test 27はこの約10倍以上の間隔を使う実映像候補です。

### 方式

- `rawDepth < 1.0`かつ`1.0 - rawDepth < 1e-6`の画素だけを月候補としてDoFから保護します。
- 深度未描画の空（ちょうど`1.0`）は除外します。
- 円形マスク、中心検出、形状学習、画面外履歴を使用しません。
- 月の見えている画素にだけ毎フレーム直接反応します。
- 手前の人物・山・木は近い深度を書き込むため、自動的に保護対象から外れます。
- Test 26で黄・緑だった遠景水面は閾値に入らず、通常どおりDoFが適用されます。

### 確認手順

1. Skyrimを完全に終了し、Test 26以前の月・診断Test版をすべて無効化します。
2. 本Test 27を正式版1.0.1より後に配置します。DLLとシェーダーだけを上書きし、INIは含みません。
3. 通常サイズと巨大サイズの月で、中央・画面端・中心が画面外・山や人物による遮蔽を確認します。
4. 月を完全に画面外へ出し、円形の鮮明領域が残らないことを確認します。
5. 海岸の水平線を確認し、遠景水面が不自然に鮮明化しないことを確認します。

`Keep Sky Sharp`を無効にすると、この月保護も無効になります。

## English

This is an experimental candidate, not a release. Across the four Test 26 sea views, the moon consistently had a raw-depth clear gap of `1e-7` to `3e-7`, while even the farthest water and terrain remained mainly at `3e-6` or more. Test 27 uses that separation directly.

### Method

- Only pixels satisfying `rawDepth < 1.0` and `1.0 - rawDepth < 1e-6` are protected from DoF.
- Untouched clear sky at exactly `1.0` is excluded.
- No circle, centre detection, learned shape, or off-screen history is used.
- The mask reacts directly to the moon pixels visible in the current frame.
- Foreground actors, mountains, and trees write nearer depth and therefore occlude the protection naturally.
- Distant water shown as yellow or green in Test 26 remains outside the threshold and receives normal DoF.

### Test procedure

1. Exit Skyrim completely and disable every earlier moon/diagnostic test, including Test 26.
2. Place Test 27 after the official 1.0.1 mod. It overrides only the DLL and shader and contains no INI.
3. Test normal and huge moons at screen centre, near each edge, with the centre off-screen, and behind mountains or actors.
4. Move the moon completely off-screen and verify that no circular sharp region remains.
5. Check a sea horizon and verify that distant water is not unnaturally sharpened.

Disabling `Keep Sky Sharp` also disables this moon protection.
