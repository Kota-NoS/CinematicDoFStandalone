# Far Depth Bands Diagnostic Test 25 / 遠距離深度帯診断 Test 25

## 日本語

これは正式版ではありません。Test 24で通常月・巨大月・見切れた月の正確な輪郭が公開深度と元の`kMAIN`深度の両方に存在すると確認できたため、円形推定を使わず、月と遠景を分けられる距離境界を測定します。

### 4画面の意味

- 左上：`kRAW_WATER.z`。正の値はシアン、負の値はオレンジ、ゼロは黒、取得不能はマゼンタです。
- 右上：Community Shadersから現在公開されている深度を、下記の距離帯で色分けします。
- 左下：差し替え前の実`kMAIN`深度を、同じ距離帯で色分けします。
- 右下：公開深度と実`kMAIN`深度の比較です。黒=両方とも空、黄=片方だけ深度あり、緑=距離差0.5%以内、水上の一致はシアン、赤=公開深度の方が近い、青=公開深度の方が遠い、マゼンタ=実深度を取得不能です。

### 距離帯の色

- 黒：深度未描画の空
- 赤：50 m未満
- オレンジ：50–100 m
- 黄：100–250 m
- 緑：250–500 m
- シアン：500 m–1 km
- 青：1–2 km
- マゼンタ：2–4 km
- 白：4 km以上

### 確認手順

1. Skyrimを完全に終了し、Test 24を含む以前の月・診断Test版をすべて無効化します。
2. 本Test 25を正式版1.0.1より後に配置します。DLLとシェーダーだけを上書きし、INIは含みません。
3. 通常月とMegalophobia巨大月について、中央、半分見切れ、僅かに見える構図を撮影します。
4. 可能なら同じ場所と構図でTerrain BlendingをON/OFFし、右上・左下・右下を比較します。

最重要なのは右上と左下における月の色です。同じ色の地形が月の周囲にどこまで残るかも確認してください。これにより、解像度や画面内の月サイズに依存しない画素単位マスクの距離境界を決められます。

## English

This is an experimental diagnostic, not a release. Test 24 confirmed that the exact silhouette of normal, huge, and partially off-screen moons exists in both exposed and native `kMAIN` depth. Test 25 therefore measures a distance cutoff that can separate the moon from scenery without circular reconstruction.

### Four panels

- Top-left: `kRAW_WATER.z`. Positive is cyan, negative orange, zero black, and unavailable magenta.
- Top-right: distance bands from the depth currently exposed by Community Shaders.
- Bottom-left: the same bands from the untouched native `kMAIN` depth.
- Bottom-right: exposed/native comparison. Black=both clear, yellow=only one writes depth, green=within 0.5%, cyan=agreement on water, red=exposed is nearer, blue=exposed is farther, magenta=native unavailable.

### Distance-band colours

- Black: untouched sky
- Red: below 50 m
- Orange: 50–100 m
- Yellow: 100–250 m
- Green: 250–500 m
- Cyan: 500 m–1 km
- Blue: 1–2 km
- Magenta: 2–4 km
- White: 4 km or farther

### Test procedure

1. Exit Skyrim completely and disable every earlier moon/diagnostic test, including Test 24.
2. Place Test 25 after the official 1.0.1 mod. It overrides only the DLL and shader and contains no INI.
3. Capture normal and Megalophobia moons while centred, half off-screen, and barely visible.
4. If possible, repeat the same view with Terrain Blending enabled and disabled, comparing the top-right, bottom-left, and bottom-right panels.

The moon colour in the two depth-band panels is the key result. Also note how much scenery shares that colour. This determines a resolution-independent, per-pixel cutoff for the final mask.
