# Starburst Test 2 / 回折光条 Test 2

## 日本語

Test 2では回折光条の見え方と、ボケ中心の薄まりを別々に調整できます。

- 「光条の強さ」はTest 1の約2倍へ再基準化しました。Test 2の1.0がTest 1の2.0とほぼ同じです。
- 強さの上限を4.0へ拡張しました。
- 「ボケ中心の補光」を追加しました。周囲より暗いボケ内部へ、16方向から近傍のハイライトを戻します。
- 中心補光は光条を伸ばさず、外周そのものを一律に明るくしません。
- 中心補光と光条は同じ「光条の発生しきい値」を使用します。しきい値を下げるほど曖昧な明部も対象になります。

最初の比較設定:

- 光条の強さ: 1.0
- 光条の長さ: 0.65
- 光条の発生しきい値: 0.20～0.45
- ボケ中心の補光: 0.50

中心補光だけを比較する場合は、光条の強さを0.0にして「ボケ中心の補光」を切り替えてください。
Test 1で保存した光条の強さ2.0はTest 2ではかなり強くなるため、まず1.0へ戻すことを推奨します。

このオーバーレイにはINIを同梱していません。既存の設定ファイルは上書きされません。

## English

Test 2 separates diffraction-ray intensity from correction of dim bokeh centers.

- Starburst Strength has been rebased to roughly twice the Test 1 response. A Test 2 value of 1.0 is approximately equivalent to Test 1's 2.0.
- The strength range now extends to 4.0.
- Bokeh Core Fill gathers nearby highlights from 16 circular directions only where the local bokeh highlight is weaker.
- Core Fill does not extend the rays or uniformly brighten the outer contour.
- Core Fill and diffraction rays share Starburst Threshold. Lower values include softer bright regions.

Suggested first comparison:

- Starburst Strength: 1.0
- Starburst Length: 0.65
- Starburst Threshold: 0.20–0.45
- Bokeh Core Fill: 0.50

To isolate Core Fill, set Starburst Strength to 0.0 and toggle Bokeh Core Fill.
If Test 1 saved Starburst Strength at 2.0, return it to 1.0 first because Test 2 intentionally makes it much stronger.

No INI is included in this overlay, so the user's existing configuration is preserved.
