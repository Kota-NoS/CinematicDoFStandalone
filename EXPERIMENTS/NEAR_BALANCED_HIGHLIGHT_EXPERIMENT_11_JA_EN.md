# Experiment 11: Near balanced aperture highlights

## 日本語

### 目的

実験10で改善した手前ぼけの中心充填を維持しながら、弱くなった絞り形状の輪郭を少し戻します。

### 今回の変更

- 遠景は実験9および実験10から変更しません。
- 実験10の、中心サンプルを含む明部全体の重み付き平均を維持します。
- 同じ既存サンプルから外周側の明部平均も計算します。
- 最終的な手前の明部は、全体平均75%と外周平均25%を混合します。
- 最も明るい1サンプルを選ぶ処理には戻しません。
- 追加パス、追加テクスチャ、追加サンプルはありません。

### 期待する変化

- 実験10より五角形・六角形の角が少し分かりやすくなる。
- 実験9より中心の抜けや白い外周が抑えられる。
- 実験10に近い柔らかさを残す。
- 設計上のGPU負荷は実験10とほぼ同じです。

### 推奨比較

1. 実験10と実験11を、同じ手前の花・同じカメラ位置で比較。
2. 羽根5枚、丸み0.10、形状の強さ0.60、明るいぼけの強調0.70。
3. 中心の充填、角の見え方、白い外周、遠景、黒い輪、FPSを確認。

## English

### Purpose

Restore a restrained amount of aperture definition while retaining the filled near-highlight centre introduced by Experiment 10.

### Changes

- The far path remains unchanged from Experiments 9 and 10.
- Keep Experiment 10's weighted mean over the complete near highlight footprint, including the centre sample.
- Compute a second perimeter mean from the same existing ring samples.
- Mix 75% complete-footprint mean with 25% perimeter mean.
- Do not return to selecting the single brightest sample.
- No extra pass, texture, or sample is added.

### Expected result

- Aperture corners should read more clearly than in Experiment 10.
- Hollow centres and bright rims should remain less pronounced than in Experiment 9.
- Most of Experiment 10's softness should remain.
- The designed GPU cost is nearly identical to Experiment 10.

### Suggested comparison

1. Compare Experiments 10 and 11 at the same foreground flowers and camera position.
2. Use 5 blades, roundness 0.10, shape strength 0.60, and bright-bokeh emphasis 0.70.
3. Check centre fill, corner definition, bright rims, unchanged far rendering, dark rings, and FPS.
