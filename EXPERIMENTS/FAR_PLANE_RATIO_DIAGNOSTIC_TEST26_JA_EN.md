# Far Plane Ratio Diagnostic Test 26 / ファープレーン比率診断 Test 26

## 日本語

これは正式版ではありません。Test 25では、月が常に4 km以上にあり、通常の水面は主に4 km未満である一方、水平線付近のごく遠い水面・地形も4 km以上になることが確認できました。Test 26はその白い領域をさらに細分化し、月と最遠景を深度だけで分離できるか判定します。

`kRAW_WATER.z`は、水のない草原でも地面を広く検出したため、水の意味マスクとしては使用しません。

### 4画面の意味

- 左上：Community Shadersから公開された深度を、現在のカメラ・ファープレーンに対する線形距離の割合で色分けします。
- 右上：公開深度について、生の深度値とクリア値`1.0`との差を細かく色分けします。
- 左下：差し替え前の実`kMAIN`深度について、右上と同じ細分化を行います。
- 右下：公開深度と実`kMAIN`深度の比較です。黒=両方とも空、黄=片方だけ深度あり、緑=生深度差が`1e-7`以内、赤=公開深度の方が近い、青=公開深度の方が遠い、マゼンタ=実深度を取得不能です。

### 左上：ファープレーン比率

- 黒：深度未描画の空
- 赤：90%未満
- オレンジ：90–95%
- 黄：95–98%
- 緑：98–99%
- シアン：99–99.5%
- 青：99.5–99.8%
- マゼンタ：99.8–99.95%
- 白：99.95%以上

### 右上・左下：生深度のクリア値との差

色は赤から白になるほどファープレーンへ近づきます。

- 黒：深度未描画の空（`rawDepth >= 1.0`）
- 赤：差が`1e-4`以上
- オレンジ：`3e-5`～`1e-4`
- 黄：`1e-5`～`3e-5`
- 緑：`3e-6`～`1e-5`
- シアン：`1e-6`～`3e-6`
- 青：`3e-7`～`1e-6`
- マゼンタ：`1e-7`～`3e-7`
- 白：`1e-7`未満

### 確認手順

1. Skyrimを完全に終了し、Test 25を含む以前の月・診断Test版をすべて無効化します。
2. 本Test 26を正式版1.0.1より後に配置します。DLLとシェーダーだけを上書きし、INIは含みません。
3. 月と水平線が同時に入る構図を優先し、通常月・巨大月・水のない遠景・遠い水面を撮影します。
4. 最重要なのは、右上と左下で月と水平線が同じ色になるかどうかです。

月と最遠景の色が異なれば、その境界を本番の画素単位マスクにできます。同じ色のままなら、両者は深度上ほぼ同じファープレーンに固定されており、深度だけによる完全分離はできないと判断できます。

## English

This is an experimental diagnostic, not a release. Test 25 showed that the moon always lies beyond 4 km, while ordinary water is mostly nearer than 4 km. A thin strip of very distant water or terrain can also enter the same 4 km-or-farther band. Test 26 subdivides that band to determine whether depth alone can separate the moon from the far horizon.

`kRAW_WATER.z` is intentionally not used as a semantic water mask because it also marked broad ground areas in a grassland scene with no water.

### Four panels

- Top-left: exposed depth as a percentage of the current camera far plane.
- Top-right: fine bands of the gap between exposed raw depth and the clear value `1.0`.
- Bottom-left: the same fine raw-gap bands from untouched native `kMAIN` depth.
- Bottom-right: exposed/native comparison. Black=both clear, yellow=only one writes depth, green=raw-depth difference at or below `1e-7`, red=exposed is nearer, blue=exposed is farther, magenta=native depth unavailable.

### Top-left: far-plane ratio

- Black: untouched sky
- Red: below 90%
- Orange: 90–95%
- Yellow: 95–98%
- Green: 98–99%
- Cyan: 99–99.5%
- Blue: 99.5–99.8%
- Magenta: 99.8–99.95%
- White: 99.95% or farther

### Top-right and bottom-left: raw clear-value gap

Colours progress from red to white as depth approaches the far plane.

- Black: untouched sky (`rawDepth >= 1.0`)
- Red: gap at least `1e-4`
- Orange: `3e-5` to `1e-4`
- Yellow: `1e-5` to `3e-5`
- Green: `3e-6` to `1e-5`
- Cyan: `1e-6` to `3e-6`
- Blue: `3e-7` to `1e-6`
- Magenta: `1e-7` to `3e-7`
- White: gap below `1e-7`

### Test procedure

1. Exit Skyrim completely and disable every earlier moon/diagnostic test, including Test 25.
2. Place Test 26 after the official 1.0.1 mod. It overrides only the DLL and shader and contains no INI.
3. Prioritize views containing both the moon and horizon; capture normal and huge moons, dry distant scenery, and distant water.
4. The key result is whether the moon and horizon have the same colour in the top-right and bottom-left panels.

Different colours provide a usable threshold for the final per-pixel mask. If they remain the same, both are effectively pinned to the same far-plane depth and depth alone cannot fully separate them.
