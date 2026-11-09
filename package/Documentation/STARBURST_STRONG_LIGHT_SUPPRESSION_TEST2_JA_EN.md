# Starburst Strong-Light Suppression Test 2 / 強光源抑制テスト2

## 日本語

Test 1でスライダー効果がほぼ現れなかった問題を修正しました。

- Test 1は明部抽出後の輝度1.0超だけを圧縮していましたが、実際の入力は白飛びして見える部分も概ね0～1内に収まっていました。
- Test 2は追加のテクスチャ読取りを増やさず、明部抽出前の光源輝度を利用します。
- 現在の「明部抽出しきい値」から白レベルへ近づく上側領域にソフトニーを設け、極端な窓・ろうそく・照明ほど強く抑制します。
- しきい値付近の食器反射や木漏れ日はできるだけ維持します。
- 0で従来動作、1で最大抑制です。まず0と1を比較し、反応を確認できたら0.25～0.50で調整してください。
- 絞り形状ボケと「ボケ中心の補光」には影響しません。

## English

Test 2 fixes the nearly inactive slider response in Test 1.

- Test 1 compressed only post-extraction luminance above 1.0, but the actual input generally remained within 0–1 even where the image appeared blown out.
- Test 2 reuses source luminance captured before highlight extraction without adding another texture read.
- A soft upper knee runs from the current Highlight Extraction Threshold toward white, increasingly suppressing extreme windows, candles, and lamps.
- Reflections and dappled light near the lower threshold are preserved as much as possible.
- Zero preserves the previous response and one applies maximum suppression. Compare 0 and 1 first, then tune around 0.25–0.50.
- Aperture-shaped bokeh and Bokeh Core Fill are unaffected.
