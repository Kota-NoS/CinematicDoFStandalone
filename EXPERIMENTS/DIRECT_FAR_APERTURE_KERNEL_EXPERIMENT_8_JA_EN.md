# Experiment 8: Direct far-aperture kernel

## 日本語

### 目的

実験2～7では、遠景の円形ぼかしとは別に明部レイヤーを作り、絞り形状を後から合成しました。実機比較では、強度1.00のON/OFFおよび羽根3枚と6枚でも遠景に有意な差が見えなかったため、この追加レイヤー方式を終了します。

### 今回の変更

- 公開版 `0.8.32` の15パス構成へ戻し、追加の明部抽出・形状レイヤーを使用しません。
- 遠景ぼかしの実際のサンプル配置を、円から正多角形へ直接変形します。
- 多角形は元の円とほぼ同じ面積になるよう補正します。辺方向は内側、頂点方向は外側へ動くため、ぼかしの大きさだけを縮める方式より輪郭差が出やすくなります。
- 三角形だけは頂点が広がり過ぎないよう補正量を制限します。
- 減算、露出ゲイン、追加ブルームはありません。暗い場所で黒い輪を作った符号付き差分処理もありません。
- 手前ぼかしは公開版 `0.8.32` の処理を変更しません。
- 追加パスと追加テクスチャがないため、処理負荷は公開版 `0.8.32` と同等になる設計です。

### 推奨比較

同じカメラ位置で次を比較してください。

1. 絞り形状ボケ OFF
2. ON、羽根3枚、丸み0.00、形状の強さ1.00
3. ON、羽根6枚、丸み0.00、形状の強さ1.00

遠景の小さな点光源、空と木の境界、暗所の光源周辺、手前の花、FPSを確認します。

## English

### Goal

Experiments 2 through 7 built a separate bright-pass layer and composited the aperture shape over the circular far blur. In-game comparisons showed no meaningful far-field difference at strength 1.00, either between ON and OFF or between three and six blades, so that layered approach is now retired.

### Changes in this experiment

- Return to the public `0.8.32` fifteen-pass layout; no extra highlight extraction or shape-layer passes are used.
- Directly transform the actual far-blur sample footprint from a circle into a regular polygon.
- Normalize the polygon to approximately the same area as the original disc. Its sides move inward while its vertices move outward, making the silhouette easier to distinguish than an inscribed-only kernel.
- Cap the triangle normalization to avoid excessive vertex reach.
- No subtraction, exposure gain, or added bloom is used. The signed-difference operation that produced dark rings in an earlier experiment is absent.
- The near-blur path is unchanged from public `0.8.32`.
- With no additional passes or textures, the intended cost is comparable to public `0.8.32`.

### Suggested comparison

From the same camera position, compare:

1. Aperture bokeh OFF.
2. ON, 3 blades, roundness 0.00, shape strength 1.00.
3. ON, 6 blades, roundness 0.00, shape strength 1.00.

Check small distant lights, sky/tree boundaries, lights in dark scenes, foreground flowers, and FPS.
