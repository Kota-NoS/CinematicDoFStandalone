# Experiment 12: Near stronger aperture shape

## 日本語

### 目的

実験11で中心を保ったまま、手前ぼけの絞り形状をもう一段分かりやすくします。

### 今回の変更

- 遠景は実験9以降から変更しません。
- 中心を含む明部全体の平均を60%、外周側の平均を40%混合します。
- 実験11の75%対25%から、外周成分だけを15ポイント強めました。
- 最も明るい1サンプルを選ぶ処理には戻しません。
- 追加パス、追加テクスチャ、追加サンプルはありません。

### 期待する変化

- 実験11より五角形・六角形の角が分かりやすくなる。
- 実験9より中心の抜けと白い輪を抑える。
- 設計上のGPU負荷は実験11とほぼ同じです。

### 推奨比較

1. 実験11と実験12を、同じ手前の花・同じカメラ位置で比較。
2. 羽根5枚、丸み0.10、形状の強さ0.60、明るいぼけの強調0.70。
3. 形状の角、中心の充填、白い外周、遠景、黒い輪、FPSを確認。

## English

### Purpose

Make near aperture shapes one step clearer while retaining the filled centre established by Experiments 10 and 11.

### Changes

- The far path remains unchanged from Experiment 9 onward.
- Mix 60% complete-footprint highlight mean with 40% perimeter mean.
- Increase only the perimeter contribution by 15 percentage points from Experiment 11's 75/25 balance.
- Do not return to selecting the single brightest sample.
- No extra pass, texture, or sample is added.

### Expected result

- Aperture corners should read more clearly than in Experiment 11.
- Hollow centres and bright rims should remain less pronounced than in Experiment 9.
- The designed GPU cost is nearly identical to Experiment 11.

### Suggested comparison

1. Compare Experiments 11 and 12 at the same foreground flowers and camera position.
2. Use 5 blades, roundness 0.10, shape strength 0.60, and bright-bokeh emphasis 0.70.
3. Check corner definition, centre fill, bright rims, unchanged far rendering, dark rings, and FPS.
