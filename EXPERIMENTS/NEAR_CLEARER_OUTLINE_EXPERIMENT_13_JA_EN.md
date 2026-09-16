# Experiment 13: Near clearer aperture outline

## 日本語

### 目的

実験12の中心が埋まった自然な手前ぼけを保ちながら、近景の絞り形状をもう一段だけ明確にします。

### 今回の変更

- 遠景は実験9以降から変更しません。
- 中心を含む明部全体の平均を50%、外周側の平均を50%混合します。
- 実験12の60%対40%から、外周成分だけを10ポイント強めました。
- 「ボケの密度」の計算、リング数、サンプル数は変更しません。
- 最も明るい1サンプルを選ぶ処理には戻しません。
- 追加パス、追加テクスチャ、追加サンプルはありません。

### 期待する変化

- 実験12より手前ぼけの五角形・六角形の輪郭が少し読みやすくなる。
- 外周だけにはせず、中心を含む平均を半分残すことで空洞化を抑える。
- 設計上のGPU負荷は実験12とほぼ同じです。

### 推奨比較

1. 実験12と実験13を、同じ手前の花・炎・同じカメラ位置で比較。
2. 「ボケの密度」は0.00から0.20程度を推奨。
3. 形状の角、中心の充填、白い外周、遠景、黒い輪、FPSを確認。

## English

### Purpose

Make near aperture outlines one final step clearer while retaining the naturally filled foreground blur established by Experiment 12.

### Changes

- The far path remains unchanged from Experiment 9 onward.
- Mix 50% complete-footprint highlight mean with 50% perimeter mean.
- Increase only the perimeter contribution by 10 percentage points from Experiment 12's 60/40 balance.
- Do not change Bokeh Busy Factor weighting, ring count, or sample count.
- Do not return to selecting the single brightest sample.
- No extra pass, texture, or sample is added.

### Expected result

- Near pentagonal and hexagonal outlines should read slightly more clearly than in Experiment 12.
- Half of the result still comes from the complete footprint, limiting hollow-centre regression.
- The designed GPU cost is nearly identical to Experiment 12.

### Suggested comparison

1. Compare Experiments 12 and 13 at the same foreground flowers or flames and camera position.
2. A Bokeh Busy Factor between 0.00 and 0.20 is recommended.
3. Check corner definition, centre fill, bright rims, unchanged far rendering, dark rings, and FPS.
