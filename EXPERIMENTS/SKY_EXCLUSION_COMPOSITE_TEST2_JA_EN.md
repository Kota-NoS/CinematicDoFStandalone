# Sky Exclusion Composite Test 2 / 空DoF除外・実画像合成 Test 2

## 日本語

これは正式版ではなく、Test 1で確認したクリア深度マスクを実際のDoF映像へ適用する実験版です。`1.0.1-final-candidate`（commit `391414f`）を基準にしています。

### 動作

- 元の深度バッファがクリア値 `1.0` の画素は、CoCを0にして元の未ぼけ色を復元します。
- 深度を書き込んだ画素は、従来のDoF処理をそのまま使用します。
- Test 1で黒く表示された海面、島、船、地形、建物、人物などは空扱いしません。
- 空判定のための追加レンダーターゲットや追加ディスパッチはありません。既存のCoC計算と合成シェーダー内で、元深度を各1回直接確認します。

この版の目的は、空を鮮明に戻したときに、水平線、島、木の枝、屋根などの境界へハロー、色漏れ、ちらつき、不自然な切れ目が出ないかを実画像で確認することです。

### 導入

1. Skyrimを終了します。
2. MO2で正式候補版より後に本Test版を配置し、本Test版のDLLとシェーダーが競合に勝つことを確認します。
3. Test 1を含む過去のTest版・診断版は同時に有効化しないでください。
4. INIは含まれません。保存済み設定、プリセット、言語設定は変更されません。
5. ゲーム内でDoFを有効にします。会話外でも確認する場合は「会話外でもDoFを使用」も有効にします。

### 確認してほしい場面

- Test 1と同じ海岸の同一アングル
- 海面と空の水平線、遠くの島や船
- 木の枝、葉、草、柵、屋根と空の境界
- 晴天、曇天、悪天候、昼夜、太陽、月、星、オーロラ
- 強い奥ボケおよび通常使用する奥ボケ
- カメラ移動中の境界のちらつき
- Community Shaders ON / OFF
- `bUse64bitsHDRRenderTarget=1 / 0`

特に海面そのものがDoFから外れて見えるか、空だけが鮮明になって海・島には従来どおりDoFが残るか、水平線へ明るい縁や暗い縁が出ないかを確認してください。

Testを終えたら本Modを無効化または削除すると、下にある正式候補版へ戻ります。

## English

This is an experimental build, not a release. It applies the clear-depth mask validated in Test 1 to the real DoF composite. It is based on `1.0.1-final-candidate` (commit `391414f`).

### Behaviour

- A pixel whose original depth texel still contains the clear value `1.0` receives zero CoC and its original unblurred colour is restored.
- Pixels that wrote depth continue through the existing DoF path unchanged.
- Water, islands, ships, terrain, buildings, actors, and other areas that appeared black in Test 1 are not treated as sky.
- The test adds no render target and no dispatch. It performs one direct source-depth check in the existing CoC shader and one in the existing combiner.

The purpose of this build is to inspect the real composite for halos, colour leakage, flicker, or an unnatural cut along horizons, islands, branches, roofs, and other sky boundaries.

### Installation

1. Exit Skyrim.
2. In MO2, place this test after the final candidate and confirm that both its DLL and shader win conflicts.
3. Do not enable Test 1 or any earlier test/diagnostic build at the same time.
4. No INI is included. Saved settings, presets, and language selection are preserved.
5. Enable DoF in game. Also enable “Use DoF outside dialogue” when testing outside conversations.

### Suggested scenes

- The same coast and camera angle used for Test 1
- The sea/sky horizon and distant islands or ships
- Branches, leaves, grass, fences, and roofs against the sky
- Clear, cloudy, and storm weather; day/night; sun, moon, stars, and aurora
- Strong far blur and ordinary far-blur settings
- Boundary stability while moving the camera
- Community Shaders ON / OFF
- `bUse64bitsHDRRenderTarget=1 / 0`

Please check whether the water itself appears excluded from DoF, whether only the sky stays sharp while water and islands retain the existing DoF, and whether any bright or dark fringe appears along the horizon.

Disable or remove this test mod to return to the final candidate installed below it.
