# Starburst Test 7 / 回折光条 Test 7

## 日本語

Test 7は、Bottled Shaders Effects 11のLensなど、DoF入力へ既に合成された広域フレアが回折光条を過剰に発生させる場合の互換性テストです。

- 光条専用の「広域フレア抑制」を追加しました。0で無効、1で最大です。
- 光条候補と少し離れた周囲4方向の明るさを比較し、局所的に突出した光源を残しながら、広く一様なフレアを減衰させます。
- 絞り形状ボケと「ボケ中心の補光」は従来の明部抽出を使用するため、このフィルターの影響を受けません。
- プリセット「光条」の内蔵既定値は0.75です。既存INIは上書きしません。プリセット8を押すと試験値が適用されます。

完全なLens識別用マスクではないため、小さなゴースト光や局所的に明るいフレアは残る場合があります。また値を高くすると、非常に大きな本来の光源から出る光条も弱くなる可能性があります。

## English

Test 7 is a compatibility experiment for broad flare already composited into the DoF input, including the Lens effect in Bottled Shaders Effects 11.

- Adds a starburst-only `Wide-Flare Suppression` control. Zero disables it and one applies the strongest suppression.
- Each ray source is compared with a wider four-direction neighbourhood. Compact local peaks are retained while broad, nearly uniform flare is attenuated.
- Aperture-shaped bokeh and Bokeh Core Fill continue to use the original highlight extraction and are not filtered by this control.
- The built-in Starburst preset defaults to 0.75. Existing INI files are not replaced; press preset slot 8 to apply the test value.

This is not a dedicated Lens mask, so small ghost highlights or locally bright parts of a flare may remain. High values can also reduce rays from genuinely large light sources.
