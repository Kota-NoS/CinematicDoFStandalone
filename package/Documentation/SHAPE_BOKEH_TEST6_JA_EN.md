# Shape Bokeh Test 6 / 形状ボケ Test 6

## 日本語

Test 6では、星・ハート・絞り形状と「内部を滑らかに埋める」を、単層の高密度サンプリングで描画します。

- ハートの0度は、一般的な正位置になるよう上下を反転しています。
- ボケ形状は「絞り」「星」「ハート」から選べます。
- 「内部を滑らかに埋める」をONにすると、形状内部まで高密度にサンプリングします。
- 多重形状の比較機能は廃止し、単層のみとしました。小形状を重ねる方式では、同心状の構造、中心点、外周の減光が発生し、単層より自然な結果にならなかったためです。

このオーバーレイにはINIを含めません。既存の1.0.6 Full版より後へ配置し、DLLとシェーダーだけを上書きしてください。

## English

Test 6 renders Aperture, Star, and Heart bokeh with a single dense sampling layer when `Fill Aperture Interior` is enabled.

- Heart at zero degrees is flipped vertically so its default orientation is the familiar upright shape.
- Bokeh shape can be selected from `Aperture`, `Star`, and `Heart`.
- `Fill Aperture Interior` increases sampling density across the full shape interior.
- Nested-shape comparison modes have been removed. Overlaying smaller copies introduced concentric structure, a centre point, and reduced outer-edge brightness without producing a more natural result than the single layer.

This overlay intentionally contains no INI. Place it after the existing 1.0.6 Full mod so that only the DLL and shaders win conflicts.
