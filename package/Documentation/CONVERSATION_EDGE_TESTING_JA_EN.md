# Open Shaders FSR Active-Area Fix historical verification / Open Shaders FSR有効描画範囲修正の検証記録

This document records the component test for the FSR active-area fix. The integrated 0.8.32 release also includes the separately verified safe `AccentuateWhites`; see `RELEASE_0.8.32_JA_EN.md`.

この文書はFSR有効描画領域修正の単体テスト記録です。統合した0.8.32正式版には、別途確認済みの安全化`AccentuateWhites`も含まれます。`RELEASE_0.8.32_JA_EN.md`を参照してください。

This package is based on `Cinematic DoF Standalone 0.8.32 Aperture Bokeh Test 4`.
It fixes the full DoF pipeline using the output-allocation size instead of Open Shaders FSR's active internal render area. The INI and visual settings are unchanged.

このパッケージは `Cinematic DoF Standalone 0.8.32 Aperture Bokeh Test 4` が基準です。
Open Shaders FSRの内部有効描画範囲ではなく、出力用テクスチャ全体の寸法をDoF処理に使っていた問題を修正します。INIと見た目の設定は変更していません。

## What changed / 変更内容

- Clamp blur sampling coordinates to the nearest valid texel centre on all four screen edges.
- Clamp full-resolution tile gathering and half-resolution tent filtering instead of reading outside their textures.
- Ignore surplus compute threads when a render size is not divisible by the 8x8 thread-group size.
- Create DoF intermediate resources at the active FSR render size reported by the current viewport.
- Convert color/depth sampling coordinates from normalized render-area space into the larger input allocation.
- Copy only the processed active render rectangle back to Skyrim's main target.
- This FSR component itself does not change blur strength, conversation focus, aperture shape, presets, language UI, or saved settings.

- ぼかしの参照座標を、画面四辺の最も近い有効画素の中心へ固定します。
- フル解像度のタイル収集と半解像度のテントフィルターで、テクスチャ外を読まないようにします。
- 描画サイズが8x8のスレッド単位で割り切れない場合、余分な計算スレッドを書き込み前に停止します。
- 現在のビューポートが示すFSR内部解像度で、DoFの中間テクスチャを作成します。
- 色・深度の参照座標を、有効描画範囲の正規化座標から大きい入力テクスチャ上の座標へ変換します。
- 処理済みの有効描画範囲だけをSkyrimのメインターゲットへ書き戻します。
- このFSR修正そのものは、ぼかし強度、会話フォーカス、絞り形状、プリセット、言語UI、保存設定を変更しません。

The earlier diagnostic showed an input allocation of 2560x1440 with an active FSR Quality viewport of 1706x960. This build processes 1706x960 instead of touching the unused right and bottom parts of the allocation.

前回の診断では、入力テクスチャが2560x1440である一方、FSR Qualityの有効ビューポートは1706x960でした。この版では未使用の右側・下側へ触れず、1706x960だけを処理します。

## Test procedure / 確認手順

1. Disable the previous diagnostic package and enable only this package.
2. Test Open Shaders FSR 3.1 at Native AA, Quality, and—if practical—Ultra Performance.
3. Check ordinary gameplay DoF and conversation DoF with a strong blur setting.
4. Confirm that the white/right-bottom boundary is gone and that focus position and blur strength remain consistent.
5. After closing the game, confirm that `CinematicDoFStandalone.log` reports an active-sized resource line such as `Created depth-of-field resources: 1706x960 ... (input allocation 2560x1440)` for FSR Quality.

1. 前の診断版を無効にし、このパッケージだけを有効にします。
2. Open Shaders FSR 3.1をNative AA、Quality、可能であればUltra Performanceでも試します。
3. 強めのぼかし設定で、通常時のDoFと会話時のDoFを確認します。
4. 右端・下端の白い境界が消え、フォーカス位置とぼかし強度に不自然な変化がないか確認します。
5. ゲーム終了後、FSR Qualityなら`CinematicDoFStandalone.log`に `Created depth-of-field resources: 1706x960 ... (input allocation 2560x1440)` のような有効解像度が記録されていることを確認します。
