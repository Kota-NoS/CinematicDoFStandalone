# Far Density Mip Test 2 / 奥ボケ密度Mipテスト2

## 日本語

このテスト版は、Far Density Mip Test 1で確認できた奥ボケの連続性を保ちながら、絞り形状の輪郭を少し戻すための保守的な調整です。

- Test 1と同じく、半解像度のプリブラーから1/4・1/8・1/16解像度のカラー画像を生成します。
- 奥ボケ用Mipを一段粗くする境界を遅らせ、同じ条件ではTest 1と同等または一段高精細な画像を選びます。
- 最大級の奥ボケでは引き続き低解像度Mipを使用できるため、密度改善を完全には失いません。
- Test 4の絞り形状、HDR/PBR安全化Highlight Boost、FSR有効描画領域修正、手前ボケ、焦点処理、9プリセット互換性は変更していません。
- 螺旋サンプリング版および固定5タップ版のコードは含みません。

同一セーブ、同一カメラ、同一プリセットでTest 1と比較してください。特に、多角形ボケの輪郭が戻るか、二重輪郭や分裂が再発しないか、Mip境界で急な見た目の変化が出ないか、フレームレートが維持されるかを確認してください。

## English

This experimental build conservatively retunes Far Density Mip Test 1 to recover some aperture-edge definition while retaining its improved far-bokeh continuity.

- It keeps the same quarter-, eighth-, and sixteenth-resolution color pyramid generated from the half-resolution pre-blur.
- Transitions to coarser far-gather levels occur later, so a given blur condition uses the same level as Test 1 or one higher-detail level.
- Extremely large far blur can still use the coarsest levels, preserving the density benefit where it matters most.
- Test 4 aperture shaping, bounded HDR/PBR-safe Highlight Boost, the FSR active-render-area fix, near blur, focus behavior, and nine-preset compatibility are unchanged.
- The spiral-sampling and fixed-five-tap experiments are not included.

Compare against Test 1 using the same save, camera, and preset. Inspect aperture-edge definition, recurrence of doubled or split outlines, sudden changes at Mip transitions, and frame rate.
