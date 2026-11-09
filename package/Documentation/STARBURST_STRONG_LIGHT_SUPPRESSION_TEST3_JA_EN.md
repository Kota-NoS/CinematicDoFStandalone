# Starburst Strong-Light Suppression Test 3 / 強光源抑制テスト3

## 日本語

Test 2で強い光源の周囲に輪状の層が見える問題を修正しました。

- Test 2の単純な減衰倍率は、最も明るい中心を周囲より強く下げることで輝度勾配を逆転させ、輪郭を作る場合がありました。
- Test 3は抽出後の光条輝度へ単調増加するソフト圧縮曲線を適用します。入力が明るくなるほど出力も必ず明るくなるため、中心と周辺の大小関係を反転させません。
- 抑制量は維持しつつ、スライダー中間域を調整しやすいよう強度変化も滑らかにしました。
- 0で従来動作、1で最大抑制です。輪の確認はまず1で行い、問題がなければ0.25～0.60程度で調整してください。
- 絞り形状ボケと「ボケ中心の補光」には影響しません。

## English

Test 3 fixes the visible contour rings around strongly suppressed light sources in Test 2.

- Test 2 used a simple attenuation multiplier that could reduce the brightest centre more than its surroundings, invert the luminance gradient, and reveal a ring.
- Test 3 applies a strictly monotonic soft-compression curve to extracted starburst luminance. A brighter input always remains brighter in the output, so the centre-to-edge ordering cannot be inverted by this control.
- The curve retains useful suppression while making the middle of the slider easier to tune.
- Zero preserves the previous response and one applies maximum suppression. Check for rings at one first, then tune around 0.25–0.60.
- Aperture-shaped bokeh and Bokeh Core Fill are unaffected.
