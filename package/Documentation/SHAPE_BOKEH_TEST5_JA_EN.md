# Shape Bokeh Test 5 / 形状ボケ Test 5

## 日本語

Test 5では、「内部を滑らかに埋める」を検証済みの単一品質へ整理し、遊び用のボケ形状として星とハートを追加します。

- 「形状ボケ品質」の高／最高は廃止しました。
- 「内部を滑らかに埋める」ONは、中央が自然に埋まった従来の「高」と同じ密度を使用します。
- 「ボケ形状」で「絞り／星／ハート」を選択できます。
- 星では「絞り羽根枚数」が星の頂点数になります。
- 「形状の丸み」「形状の強さ」「形状の回転」は3形状すべてに作用します。
- 通常描画と「髪を馴染ませる」モード1・2へ同じ形状処理を追加しています。

大きくピンぼけした点光源を背景へ置き、「内部を滑らかに埋める」をONにして比較してください。最初は「形状の丸み」0.00～0.20、「形状の強さ」0.8～1.0を推奨します。ハートは上下方向が分かりやすいため、「形状の回転」0度から確認してください。

このオーバーレイにはINIを含めません。既存の1.0.6 Fullより後へ配置し、DLLとシェーダーだけを上書きしてください。

## English

Test 5 simplifies `Fill Aperture Interior` to the single validated quality level and adds Star and Heart as playful bokeh silhouettes.

- The High / Highest `Aperture Fill Quality` selection has been removed.
- Enabling `Fill Aperture Interior` uses the density of the former High mode, which produced the accepted smooth centre.
- `Bokeh Shape` selects Aperture, Star, or Heart.
- For Star, `Aperture Blades` controls the number of points.
- `Shape Roundness`, `Shape Strength`, and `Shape Rotation` affect all three silhouettes.
- The same shape implementation is included in standard rendering and both Blend Hair modes.

Place strongly defocused point lights in the background and enable `Fill Aperture Interior`. As a starting point, use Shape Roundness 0.00–0.20 and Shape Strength 0.8–1.0. Because Heart has a clear upright direction, check it first at 0 degrees of rotation.

This overlay intentionally contains no INI. Place it after the existing 1.0.6 Full mod so that only the DLL and shaders win conflicts.
