# Far Density Mip Test 1 / 奥ボケ密度Mipテスト1

## 日本語

このテスト版は、現行0.8.32のTest 4絞り形状、HDR/PBR安全化Highlight Boost、FSR有効描画領域修正、焦点処理、9プリセット互換性を維持したまま、奥ボケのサンプル密度だけを検証するものです。

- 半解像度のプリブラーから1/4・1/8・1/16解像度のカラー画像を生成します。
- 奥ボケ半径と品質リング数から、固定サンプル間隔に合う解像度を自動選択します。
- 手前ボケ、CoC生成、被写体保護、合成、絞り形状の計算は変更していません。
- 螺旋サンプリング版および固定5タップ版のコードは含みません。

比較時は同一セーブ、同一カメラ、同一プリセットを使用し、特に大きな奥ボケにある細い構造、点光源、暗部の分裂、被写体輪郭、フレームレートを確認してください。問題が出た場合は、このテスト版を無効にして直前の9プリセット版へ戻せます。

## English

This experimental build isolates one change on top of the current 0.8.32 branch: a three-level color pyramid for far-plane gather. It retains the Test 4 aperture implementation, bounded HDR/PBR-safe Highlight Boost, FSR active-render-area fix, focus behavior, subject protection, and nine-preset compatibility.

- Quarter-, eighth-, and sixteenth-resolution color levels are generated from the existing half-resolution pre-blur.
- Far gather selects a level from the current blur radius and ring count so fixed taps retain a wider contiguous footprint at large radii.
- Near blur, CoC generation, subject protection, compositing, and aperture shaping are unchanged.
- The spiral-sampling and fixed-five-tap experiments are not included.

Compare with the same save, camera, and preset. Inspect thin background structures, point lights, dark-area breakup, subject silhouettes, and frame rate. Disable this test build to return to the preceding nine-preset build.
