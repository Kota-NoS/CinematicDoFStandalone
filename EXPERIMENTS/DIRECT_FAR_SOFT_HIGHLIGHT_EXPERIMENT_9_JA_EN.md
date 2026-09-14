# Experiment 9: Direct far kernel with soft highlights

## 日本語

### 実機結果からの判断

実験8では、強い遠景ぼかしで五角形の輪郭が確認できました。遠景ぼかし本体を多角形へ変形する方式は機能しています。一方、細かな五角形が連続する硬い模様は公開版 `0.8.32` と近く、写真のような滑らかな絞りボケには届いていませんでした。

### 今回の変更

- 実験8の等面積・多角形遠景カーネルを維持します。
- 公開版から残っていた「有効サンプル中の最も明るい値を採用する」遠景処理を外します。
- 代わりに、形状内の明るいサンプルを重み付き平均し、検出された明部の割合に応じて穏やかに合成します。
- 一様に明るい空は、明部平均と通常平均が同程度になるため、不必要に白く増幅されません。
- 小さな点光源は平方根応答で残しますが、単一サンプルを画面へ最大強度で広げません。
- 結果を暗くする減算はなく、追加パスもありません。
- 手前ぼかしは変更しません。

### 推奨比較

1. 通常設定でON/OFF。
2. 強い遠景ぼかし、羽根5枚、丸み0.10、形状の強さ0.60～0.70。
3. 同じ強いぼかしで羽根3枚と6枚。

五角形の連続模様が穏やかになったか、形そのものが残るか、暗所の黒輪、手前ぼかし、FPSを確認してください。

## English

### Conclusion from Experiment 8

Experiment 8 produced visible pentagonal outlines at strong far blur, validating direct polygon shaping of the real far kernel. However, the repeated hard pentagonal pattern remained close to public `0.8.32` and did not yet resemble smooth photographic aperture bokeh.

### Changes in this experiment

- Keep Experiment 8's equal-area polygonal far kernel.
- Remove the public-version far path that selected the brightest eligible sample.
- Replace it with a weighted mean of eligible bright samples, blended according to their coverage within the gather.
- A uniformly bright sky should not be unnecessarily amplified because its bright mean remains close to the ordinary blur mean.
- A square-root coverage response preserves small lights without spreading one tap at maximum strength.
- No subtractive term or additional pass is used.
- The near-blur path is unchanged.

### Suggested comparison

1. ON versus OFF at normal settings.
2. Strong far blur, 5 blades, roundness 0.10, shape strength 0.60 to 0.70.
3. Three versus six blades at the same strong blur.

Check whether the repeated pentagonal pattern becomes smoother while the shape remains visible, plus dark-scene rings, near bokeh, and FPS.
