# Starburst Strong-Light Suppression Test 1 / 強光源抑制テスト1

## 日本語

回折光条へ「強光源抑制」を追加した試験版です。

- 極端に明るい窓・ろうそく・照明などから発生する光条の入力輝度だけを、ソフトニー方式で緩やかに圧縮します。
- 食器の反射や木漏れ日など、中程度の明部はできるだけ維持します。
- 「広域フレア抑制」は面積の広い一様な明部を判定し、「強光源抑制」は局所的でも明るすぎる光源を判定します。両方を併用できます。
- 絞り形状ボケと「ボケ中心の補光」には影響しません。
- 0で従来と同じ動作です。まず0.25、次に0.50を目安として比較してください。
- 現在のプリセット8「光条」の見た目を維持するため、組み込み既定値は0です。

## English

This test build adds `Strong-Light Suppression` to Diffraction Starburst.

- It uses a soft knee to compress only the starburst input from extremely bright windows, candles, and lamps.
- Moderate highlights such as reflections on tableware and sun-dappled foliage are preserved as much as possible.
- `Wide-Flare Suppression` identifies broad, nearly uniform bright regions. `Strong-Light Suppression` instead controls sources that are excessively bright even when compact. Both controls can be combined.
- Aperture-shaped bokeh and Bokeh Core Fill are unaffected.
- Zero preserves the previous response. Try 0.25 first, then compare 0.50.
- The built-in Starburst preset remains at zero so its current appearance is not changed automatically.
