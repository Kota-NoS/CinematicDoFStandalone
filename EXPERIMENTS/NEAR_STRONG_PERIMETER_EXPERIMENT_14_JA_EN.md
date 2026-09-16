# Experiment 14: Near strong aperture perimeter

## 日本語

### 目的

実験13の中心が埋まった手前ぼけを基礎に、近景の絞り形状をもう一段はっきり見せます。

### 今回の変更

- 遠景は実験9以降から変更しません。
- 中心を含む明部全体の平均を30%、外周側の平均を70%混合します。
- 実験13の50%対50%から、外周成分を20ポイント強めました。
- 「ボケの密度」の計算、リング数、サンプル数は変更しません。
- 最も明るい1サンプルを選ぶ処理には戻しません。
- 追加パス、追加テクスチャ、追加サンプルはありません。

### 期待する変化と注意点

- 実験13より手前ぼけの五角形・六角形の輪郭が明確になる。
- 中心を含む平均を30%残すため、完全な外周だけの描写にはしません。
- 実験13より中心の空洞化、明るい縁、暗い輪が目立つ可能性は高くなります。
- 設計上のGPU負荷は実験13とほぼ同じです。

### 推奨比較

1. 「ボケの密度」はまず0.00にして、実験13と実験14を比較。
2. 同じ手前の花・炎、同じカメラ位置、同じ時刻・照明で確認。
3. 形状の角、中心の充填、白い外周、暗い輪、遠景、FPSを確認。
4. 中心が空きすぎる場合は、実験13を安定版として使用。

## English

### Purpose

Make near aperture shapes one step more distinct while building on the filled foreground blur of Experiment 13.

### Changes

- The far path remains unchanged from Experiment 9 onward.
- Mix 30% complete-footprint highlight mean with 70% perimeter mean.
- Increase the perimeter contribution by 20 percentage points from Experiment 13's 50/50 balance.
- Do not change Bokeh Busy Factor weighting, ring count, or sample count.
- Do not return to selecting the single brightest sample.
- No extra pass, texture, or sample is added.

### Expected result and cautions

- Near pentagonal and hexagonal outlines should read more clearly than in Experiment 13.
- Keeping 30% of the complete footprint avoids a fully perimeter-only result.
- Hollow centres, bright rims, or dark rings may become more visible than in Experiment 13.
- The designed GPU cost is nearly identical to Experiment 13.

### Suggested comparison

1. Start with Bokeh Busy Factor at 0.00 and compare Experiments 13 and 14.
2. Use the same foreground flowers or flames, camera position, time, and lighting.
3. Check corner definition, centre fill, bright rims, dark rings, unchanged far rendering, and FPS.
4. If the centre becomes too hollow, keep Experiment 13 as the stable fallback.
