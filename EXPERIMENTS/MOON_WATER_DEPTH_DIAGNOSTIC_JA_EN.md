# Moon / Water Depth Diagnostic / 月・水面深度診断

## 日本語

これは正式版ではありません。CinematicDoFStandalone 1.0.1を基準に、月と水面を既存の2種類の深度だけで判別できる可能性があるか調べるための診断版です。

通常のDoF映像の代わりに、早い段階でコピーされた深度と最終的なメイン深度の組み合わせを色で表示します。

### 色の意味

- シアン：両方の深度がクリア値。通常の空です。
- 黒：両方の深度に形状があります。通常の地形、建物、人物などです。
- 緑：早い深度にだけ形状があります。通常は発生しないため、見つかった場合は撮影してください。
- 黄：早い深度では空、最終深度では形状あり。カメラから25 m未満です。
- 橙：同じ条件で25–100 mです。
- 赤：同じ条件で100–500 mです。
- マゼンタ：同じ条件で0.5–2 kmです。
- 紫：同じ条件で2–10 kmです。
- 白：同じ条件で10 km以上です。

黄から白までの色は、低設定用の早い深度コピーより後に描画されたものです。月と水面が異なる色帯に分かれれば、追加レンダーターゲットや追加ディスパッチを使わず、距離だけで両者を分けられる可能性があります。同じ色帯なら、距離だけの判別は難しいという結果になります。

### 導入と確認

1. Skyrimを完全に終了します。起動中にシェーダーを差し替えないでください。
2. MO2で正式版1.0.1より後に本診断版を配置します。
3. 本診断版にはシェーダーだけが含まれます。DLL、INI、プリセットは変更しません。
4. DoFを有効にし、月と水面が同時に見える場面を撮影してください。
5. 可能なら近い水面、遠い水面、月を同じ画角に入れます。
6. Community Shaders使用時と未使用時で色分けが変わるかも確認します。

診断終了後は本Modを無効化または削除すると、下にある正式版1.0.1へ戻ります。

## English

This is an experimental diagnostic, not a release. It is based on CinematicDoFStandalone 1.0.1 and tests whether the moon and water can be separated using only the two depth sources already available to the effect.

The normal DoF image is replaced with colours representing the relationship between the early copied depth and the final main depth.

### Colour key

- Cyan: both depth sources are clear; ordinary sky.
- Black: geometry exists in both depth sources; ordinary terrain, buildings, actors, and similar objects.
- Green: geometry exists only in the early depth. This is unexpected; capture it if it appears.
- Yellow: clear in the early depth but written in final depth, under 25 m.
- Orange: the same class at 25–100 m.
- Red: the same class at 100–500 m.
- Magenta: the same class at 0.5–2 km.
- Violet: the same class at 2–10 km.
- White: the same class at 10 km or farther.

Yellow through white identify objects rendered after the low-settings depth copy. If the moon and water fall into different distance colours, a lightweight separation may be possible without another render target or dispatch. If they occupy the same colour band, distance alone is not sufficient.

### Installation and test

1. Exit Skyrim completely. Do not replace the shader while the game is running.
2. Place this diagnostic after the official 1.0.1 mod in MO2.
3. The diagnostic contains only the shader. It does not replace the DLL, INI, or presets.
4. Enable DoF and capture a scene containing both the moon and water.
5. If possible, include near water, distant water, and the moon in the same view.
6. Also compare the classification with and without Community Shaders.

Disable or remove this diagnostic to return to the official 1.0.1 installed below it.
