# Experiment 10: Near soft-fill aperture highlights

## 日本語

### 目的

実験9を安定候補として維持したまま、手前ぼけで中心が抜け、明るい輪郭だけが強く見える問題を調べます。

### 今回の変更

- 遠景は実験9から変更しません。
- 手前の明部処理を「外周寄りの最も明るい1サンプル」から、形状内の明部の重み付き平均へ変更します。
- 手前では外周を優先せず、絞り形状の内側全体を同じ基準で評価します。
- 従来は明部判定に含まれていなかった中心サンプルも集計へ加えます。
- 追加パス、追加テクスチャ、減算処理はありません。
- 通常の手前ぼかし形状、遠景カーネル、CoC、合成処理は変更しません。

### 期待する変化

- 手前の明るい花などで、暗い中心と白い外周の差が弱くなる。
- 細かな輪が、一つの柔らかい多角形へまとまりやすくなる。
- 遠景と手前の明部描写が近づく。

参考写真のような均一な絞り像は、本来は小さな点光源をぼかした結果です。花や葉のように面積と内部模様を持つ被写体では、その模様も畳み込まれるため、完全に同じ均一面になるとは限りません。

### 推奨比較

1. 実験9と実験10を、同じ手前の花・同じカメラ位置で比較。
2. 羽根5枚、丸み0.10、形状の強さ0.60、明るいぼけの強調0.70。
3. 手前最大ぼかしを通常値と強い値で比較。

中心の抜け、外周の白い輪、遠景の変化有無、黒い輪、FPSを確認してください。

## English

### Purpose

Keep Experiment 9 as the stable candidate while investigating hollow, edge-heavy highlights in strong near blur.

### Changes

- The Experiment 9 far path is unchanged.
- Replace the near path's brightest outer-biased sample with a weighted mean of eligible bright samples.
- Use equal highlight eligibility throughout the near aperture instead of preferring its rim.
- Include the centre sample, which the previous near highlight estimate did not inspect.
- No extra pass, texture, or subtractive term is added.
- The ordinary near gather shape, far kernel, CoC, and combiner are unchanged.

### Expected result

- Less contrast between a dark centre and bright rim on blurred foreground flowers.
- Small rings should merge into a softer polygonal footprint.
- Near and far highlights should look more consistent.

A uniformly filled photographic aperture image normally comes from defocusing a small point light. An extended object such as a flower or leaf still contributes its own area and internal texture to the convolution, so it may not become a perfectly uniform polygon.

### Suggested comparison

1. Compare Experiment 9 and Experiment 10 at the same foreground flowers and camera position.
2. Use 5 blades, roundness 0.10, shape strength 0.60, and bright-bokeh emphasis 0.70.
3. Compare normal and strong near maximum blur.

Check centre fill, bright rims, unchanged far rendering, dark rings, and FPS.
