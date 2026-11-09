# Filled Aperture Test 1 / 形状ボケ内部補完 Test 1

## 日本語

このテスト版は、既存の「絞り形状ボケ」に撮影向けの高品質オプションを追加します。

- 「内部を滑らかに埋める」がOFFの場合、従来の描画処理と負荷を維持します。
- 「高」は半径・角度方向のサンプルを増やし、形状ボケの中央を滑らかに埋めます。
- 「最高」はさらにサンプルを増やした静止画撮影向け設定です。GPU負荷は「高」より大きくなります。
- 通常描画、「髪を馴染ませる」モード1・2のすべてに同じ設定が適用されます。

確認時は「絞り形状ボケ」をONにし、大きくピンぼけした灯火などを背景へ配置してください。「内部を滑らかに埋める」のOFF／高／最高を切り替え、中央の欠け、輪の分離、フレームレートを比較してください。

## English

This test build adds an optional screenshot-quality path to the existing Aperture Bokeh feature.

- With `Fill Aperture Interior` off, the previous rendering path and cost are preserved.
- `High` adds radial and angular samples to fill the centre of shaped bokeh more smoothly.
- `Highest` adds still more samples for still-image capture and costs more GPU time than High.
- The same setting applies to standard rendering and Blend Hair modes 1 and 2.

For testing, enable Aperture Bokeh and place strongly defocused lights in the background. Compare Off, High, and Highest for centre gaps, separated rings, and frame rate.
