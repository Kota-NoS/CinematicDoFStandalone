# Sky Clear-Depth Mask Diagnostic Test 1 / 空クリア深度マスク診断 Test 1

## 日本語

これは正式版ではなく、空だけをDoFから除外できる可能性を調べる診断版です。`1.0.1-final-candidate`（commit `391414f`）を基準にしていますが、通常のDoF映像は表示しません。

### 表示の意味

- **白**：元の深度バッファがクリア値 `1.0` のまま残っている画素（空候補）
- **黒**：何らかのジオメトリが深度を書き込んだ画素

シェーダーは深度テクスチャの元の1画素を直接読み、フィルタリングを行わず二値化します。後段の平滑化も診断中は単純コピーへ置き換えているため、白黒境界へ意図的なぼかしは加えません。

これは**判定マスクだけの確認**です。空を鮮明に戻す処理はまだ実装していません。また、この段階では実際に空を合成した際の地平線ハローや色漏れまでは判断できません。

### 導入

1. Skyrimを終了します。
2. MO2で正式候補版より後に本診断版を配置し、本診断版のDLLとシェーダーが競合に勝つことを確認します。
3. 過去のTest版・診断版は同時に有効化しないでください。
4. INIは含まれません。保存済み設定、プリセット、言語設定は変更されません。
5. ゲーム内でDoFを有効にします。会話外でも確認する場合は「会話外でもDoFを使用」も有効にします。

焦点距離、F値、手前・奥ピント範囲などは白黒判定に影響しません。

### 確認してほしい場面

- 晴天、曇天、悪天候
- 昼、夜、太陽、月、星、オーロラ
- 山や建物が重なる地平線
- 木の枝、葉、柵など細い形状越しの空
- 遠景LOD
- 室内
- Community Shaders ON / OFF
- `bUse64bitsHDRRenderTarget=1 / 0`

特に、空以外で白くなる場所、空なのに黒くなる場所、カメラ移動で白黒がちらつく場所を確認してください。雲、太陽、月、霧、天候エフェクトが白か黒かも重要な結果です。

診断を終えたら本Modを無効化または削除すると、下にある正式候補版へ戻ります。

## English

This is a diagnostic build, not a release. It tests whether the sky can potentially be excluded from DoF by identifying untouched depth-buffer pixels. It is based on `1.0.1-final-candidate` (commit `391414f`) but does not display the normal DoF image.

### Display meaning

- **White:** the source depth texel still contains the clear value `1.0` (a sky candidate)
- **Black:** some geometry wrote depth at that texel

The shader reads the original depth texel directly and converts it to a binary mask without sampler filtering. The two later smoothing passes are diagnostic pass-through copies, so they do not intentionally soften the black/white boundary.

This build shows **only the classification mask**. It does not yet restore a sharp sky, and it cannot by itself prove that the final composite will be free of horizon halos or colour leakage.

### Installation

1. Exit Skyrim.
2. In MO2, place this diagnostic after the final candidate and confirm that both its DLL and shader win conflicts.
3. Do not enable any earlier test or diagnostic build at the same time.
4. No INI is included. Saved settings, presets, and language selection are preserved.
5. Enable DoF in game. Also enable “Use DoF outside dialogue” when testing outside conversations.

Focus distance, F-number, near range, and far range do not affect this black/white classification.

### Suggested scenes

- Clear, cloudy, and storm weather
- Day, night, sun, moon, stars, and aurora
- Mountain and building horizons
- Sky seen through branches, leaves, fences, and other thin geometry
- Distant LOD
- Interiors
- Community Shaders ON / OFF
- `bUse64bitsHDRRenderTarget=1 / 0`

Please look especially for non-sky areas that turn white, sky areas that remain black, and mask flicker during camera motion. Whether clouds, the sun, moon, fog, and weather effects become white or black is also useful evidence.

Disable or remove this diagnostic mod to return to the final candidate installed below it.
