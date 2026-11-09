# Starburst Test 5 / 回折光条 Test 5

## 日本語

Test 5では、形状ボケと回折光条をすぐ試せる専用プリセット10「形状＋光条」を追加します。
既存プリセット1～9の表示順、内部ID、INIセクション、保存値は変更しません。

プリセット10の主な初期値：

- 「絞り形状ボケ」「内部を滑らかに埋める」「回折光条」：ON
- 通常の絞り、5枚羽根、丸み0.10、形状の強さ0.65、回転0度
- 明部抽出しきい値0.54、ボケ中心の補光0.50
- 光条5本、強さ5.00、長さ0.90、回転0度
- 焦点距離70.3 mm、F値3.4、奥最大ぼかし1.81、手前最大ぼかし1.00

このテスト用オーバーレイにはINIを含めません。既存INIに`Preset.ShapeStarburst`が無い場合は、
DLL内の上記初期値が10番へ使用されます。「現在値を保存」を押した場合だけ、新しいセクションがINIへ作成されます。

## English

Test 5 adds a dedicated tenth preset, `Shape + Starburst`, so the shaped-bokeh and diffraction controls can
be tried without configuring each value manually. Existing slots 1 through 9 retain their display order,
stable storage IDs, INI sections, and saved values.

The principal defaults are filled five-blade aperture bokeh, a 0.54 highlight-extraction threshold, 0.50
bokeh-core fill, and five diffraction rays at strength 5.00 and length 0.90. The test overlay contains no
INI. When `Preset.ShapeStarburst` is absent, the DLL supplies these built-in defaults; the section is only
created if Store Current is used for slot 10.
