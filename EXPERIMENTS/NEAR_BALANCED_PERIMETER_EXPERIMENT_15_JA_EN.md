# Experiment 15: Near balanced aperture perimeter

## 日本語

### 目的

実験13（全体50%・外周50%）と実験14（全体30%・外周70%）の間を補い、手前ぼけの中心に光を残しながら、絞り形状の外周も読み取れる均衡点を確認します。

### 今回の変更

- 遠景は実験9以降から変更しません。
- 中心を含む明部全体の平均を40%、外周側の平均を60%混合します。
- 実験14から中心を含む成分を10ポイント増やし、外周成分を10ポイント減らします。
- 「形状の強さ」の設定値と働きは変更しません。
- 「ボケの密度」の計算、リング数、サンプル数は変更しません。
- 最も明るい1サンプルを選ぶ処理には戻しません。
- 追加パス、追加テクスチャ、追加サンプルはありません。

### 期待する変化と注意点

- 実験14より中心の光が残りやすく、中央の空白が少し目立ちにくくなる。
- 実験13より外周側の影響が強く、五角形・六角形の輪郭を保ちやすい。
- 実験14より形状が少し穏やかになるため、必要なら「形状の強さ」を0.60から少し上げて比較できます。
- 設計上のGPU負荷は実験13・14とほぼ同じです。

### 推奨比較

1. まず現在と同じ「形状の強さ」0.60、「ボケの密度」0.00で実験14と比較。
2. 同じ手前の炎または花、同じカメラ位置、同じ時刻・照明で確認。
3. 中心の光、中央の空白、外周の輪郭、白い縁、暗い輪、遠景、FPSを確認。
4. 輪郭が弱い場合だけ「形状の強さ」を0.70、0.80の順に上げて確認。
5. 実験14の方が自然なら、保存済みの実験14へ戻します。

## English

### Purpose

Fill the untested midpoint between Experiment 13 (50% complete footprint / 50% perimeter) and Experiment 14 (30% / 70%), retaining light near the centre of foreground bokeh while keeping the aperture perimeter readable.

### Changes

- The far path remains unchanged from Experiment 9 onward.
- Mix 40% complete-footprint highlight mean with 60% perimeter mean.
- Increase the centre-inclusive contribution by 10 percentage points and reduce the perimeter contribution by 10 points relative to Experiment 14.
- Do not change the Shape Strength setting or its behaviour.
- Do not change Bokeh Busy Factor weighting, ring count, or sample count.
- Do not return to selecting the single brightest sample.
- No extra pass, texture, or sample is added.

### Expected result and cautions

- Preserve more centre light and make the central gap slightly less apparent than Experiment 14.
- Retain more pentagonal or hexagonal outline than Experiment 13 because the perimeter still has the larger share.
- The shape may look slightly gentler than Experiment 14; if needed, compare by raising Shape Strength slightly above 0.60.
- The designed GPU cost is nearly identical to Experiments 13 and 14.

### Suggested comparison

1. Begin with the current Shape Strength 0.60 and Bokeh Busy Factor 0.00, then compare against Experiment 14.
2. Use the same foreground flame or flowers, camera position, time, and lighting.
3. Check centre light, central gaps, perimeter definition, bright rims, dark rings, unchanged far rendering, and FPS.
4. Only if the outline is too weak, compare Shape Strength 0.70 and then 0.80.
5. If Experiment 14 remains more natural, return to the preserved Experiment 14 package.
