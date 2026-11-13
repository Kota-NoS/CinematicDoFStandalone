# Cinematic DoF Standalone 1.0.12

## 日本語

- Skyrim 1.7.99 以降で内部描画倍率と動的解像度の状態を正しい位置から読み取るよう修正しました。Skyrim 1.7.104 と Open Shaders 2.20.0 の組み合わせで、DLSS Quality 時にぼけが四角く途切れる問題の解消を確認しました。
- 同じ環境で HDR 有効時の深度フレームも確認済みです。

新規導入には Full、既存の INI を引き継ぐ場合は Update を使用してください。試作版や診断版は無効にしてください。

## English

- Fixed how the plugin reads the internal render scale and dynamic resolution state on Skyrim 1.7.99 and later. The fix was tested on Skyrim 1.7.104 with Open Shaders 2.20.0 and resolved rectangular DoF artifacts with DLSS Quality.
- Depth Frame was also verified with HDR enabled in the same setup.

Use Full for a new installation or Update to keep your existing INI. Disable any separate trial or diagnostic overlays.
