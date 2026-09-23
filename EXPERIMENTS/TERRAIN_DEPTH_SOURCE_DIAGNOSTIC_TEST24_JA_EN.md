# Terrain Depth Source Diagnostic Test 24 / Terrain Blending深度源診断 Test 24

## 日本語

これは正式版ではありません。通常のDoF画像の代わりに、同じ画面を4分割してCommunity ShadersのTerrain Blendingが深度と`kRAW_WATER`へ与える影響を可視化します。円形月マスク、形状学習、履歴マスクは使用しません。

### 4画面の意味

- 左上：`kRAW_WATER.z`。正の値はシアン、負の値はオレンジ、ゼロは黒です。取得不能時はマゼンタです。
- 右上：現在ゲームから公開されているメイン深度。深度を書いた画素は緑、未描画の空は濃紺です。Terrain Blending ON時はCSの合成深度へ差し替わる可能性があります。
- 左下：差し替えの影響を受けない、実際の`kMAIN`深度テクスチャから新規作成したSRV。深度を書いた画素は青、未描画の空は暗赤です。取得不能時はマゼンタです。
- 右下：3種類の重なり。赤=`kRAW_WATER.z`正、緑=公開深度あり、青=実メイン深度ありです。混色は複数条件の一致を表します。

各区画には同じ全画面映像が縮小表示されます。白線は区画の境界です。

### 確認手順

1. Skyrimを完全に終了し、以前の月マスクTest版をすべて無効化します。
2. 正式版1.0.1より後に本診断版を配置します。本ZIPはDLLとシェーダーだけを上書きし、INIは含みません。
3. Community ShadersとCinematic DoFを有効にします。
4. Terrain BlendingをONにして、月と水が同時に見える画面を撮影します。
5. 同じ場所・角度でTerrain BlendingをOFFにし、もう一枚撮影します。
6. 普通の月とMegalophobiaの巨大月の両方で、画面中央・半分見切れ・僅かに見える状態を比較します。

特に、左上で月の実形状が出るか、右上と左下のどちらが月を保持するか、右下で月と水が別の色になるかを確認してください。

## English

This is an experimental diagnostic, not a release. It replaces the normal DoF image with four scaled views of the same frame to show how Community Shaders Terrain Blending affects depth and `kRAW_WATER`. It does not use a circular moon mask, shape learning, or mask history.

### Four panels

- Top-left: `kRAW_WATER.z`. Positive values are cyan, negative values orange, and zero is black. Magenta means unavailable.
- Top-right: the main-depth SRV currently exposed by the game. Written depth is green and untouched sky is dark navy. Terrain Blending may substitute its blended depth while enabled.
- Bottom-left: a fresh SRV made directly from the native `kMAIN` depth texture. Written depth is blue and untouched sky is dark red. Magenta means unavailable.
- Bottom-right: overlap bits. Red = positive `kRAW_WATER.z`, green = exposed depth written, blue = native depth written. Mixed colours mean multiple conditions agree.

Each panel shows the same complete frame. White lines divide the panels.

### Test procedure

1. Exit Skyrim completely and disable all earlier moon-mask test builds.
2. Place this diagnostic after the official 1.0.1 mod. The ZIP overrides only the DLL and shader and contains no INI.
3. Enable Community Shaders and Cinematic DoF.
4. Capture a scene containing both the moon and water with Terrain Blending enabled.
5. Without changing the location or camera, disable Terrain Blending and capture it again.
6. Compare the normal moon and the Megalophobia huge moon while centred, half off-screen, and barely visible.

Check whether the top-left panel contains the exact moon shape, which depth panel retains the moon, and whether the bottom-right panel gives the moon and water different colours.
