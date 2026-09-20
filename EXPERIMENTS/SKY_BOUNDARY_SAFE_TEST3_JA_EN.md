# Sky Boundary-Safe Composite Test 3 / 空境界保護・実画像合成 Test 3

## 日本語

これは正式版ではなく、Test 2で確認された空と遠景・海面の不自然な境界を軽減する実験版です。`1.0.1-final-candidate`（commit `391414f`）とTest 2の空判定を基準にしています。

### Test 2からの変更

- 深度を書き込んだ地形、木、建物、海面などの輪郭上では、従来のDoFを維持します。
- 空の画素から3段階の距離で周囲の深度を確認し、輪郭から離れるほど元の鮮明な空を強く復元します。
- 境界幅は解像度と現在の「奥最大ぼかし」に合わせて変化し、最大でも限定された範囲に抑えます。
- 海、島、船、地形、建物、人物は引き続き空扱いしません。
- 追加レンダーターゲットと追加ディスパッチはありません。DLLとINIも変更していません。

空画素でのみ最大12回の周辺深度読み取りを行うため、Test 2よりシェーダー負荷は少し増えます。境界保護値は既存のCoCテクスチャへ反映し、合成段階では同じ周辺検索を繰り返しません。

### 導入

1. Skyrimを終了します。
2. MO2で正式候補版より後に本Test版を配置し、本Test版のDLLとシェーダーが競合に勝つことを確認します。
3. Test 1・Test 2を含む過去のTest版・診断版は同時に有効化しないでください。
4. INIは含まれません。保存済み設定、プリセット、言語設定は変更されません。
5. ゲーム内でDoFを有効にします。会話外でも確認する場合は「会話外でもDoFを使用」も有効にします。

### 優先確認箇所

- Test 2で横線が見えたCommunity Shaders ONの海岸
- 海面と空の水平線、遠くの島や船
- ぼけた木、山、屋根、細い枝と鮮明な空の境界
- 通常の奥ぼけと強い奥ぼけ
- カメラ移動中の境界のちらつき
- Community Shaders ON / OFF
- `bUse64bitsHDRRenderTarget=1 / 0`

見るべき点は、Test 2より境界が自然になったか、空の広い部分の鮮明さが残っているか、輪郭の周囲へ別の帯やハローが移動していないか、そしてFPS差が体感できるかです。

Testを終えたら本Modを無効化または削除すると、下にある正式候補版へ戻ります。

## English

This is an experimental build, not a release. It attempts to reduce the unnatural boundary between the sharp sky and blurred distant geometry/water observed in Test 2. It is based on `1.0.1-final-candidate` (commit `391414f`) and the Test 2 clear-depth classification.

### Changes from Test 2

- The normal DoF result is retained at silhouettes belonging to depth-writing terrain, trees, buildings, water, and other geometry.
- Sky pixels inspect surrounding depth at three distances and restore progressively more of the sharp source sky farther away from geometry.
- The boundary width scales with resolution and the current Far Plane Max Blur while remaining capped to a limited region.
- Water, islands, ships, terrain, buildings, and actors are still not treated as sky.
- No render target or dispatch was added. The DLL and INI are unchanged.

Sky pixels perform at most twelve additional neighbouring depth reads, so shader cost is slightly higher than Test 2. The resulting protection is encoded in the existing CoC texture, avoiding a repeated neighbourhood search in the combiner.

### Installation

1. Exit Skyrim.
2. In MO2, place this test after the final candidate and confirm that both its DLL and shader win conflicts.
3. Do not enable Test 1, Test 2, or any earlier test/diagnostic build at the same time.
4. No INI is included. Saved settings, presets, and language selection are preserved.
5. Enable DoF in game. Also enable “Use DoF outside dialogue” when testing outside conversations.

### Priority scenes

- The Community Shaders ON coast where Test 2 showed a horizontal seam
- The sea/sky horizon and distant islands or ships
- Blurred trees, mountains, roofs, and thin branches against a sharp sky
- Ordinary and strong far-blur settings
- Boundary stability while moving the camera
- Community Shaders ON / OFF
- `bUse64bitsHDRRenderTarget=1 / 0`

Check whether the boundary looks more natural than Test 2, whether broad open-sky detail remains sharp, whether the previous seam merely moved into a new band or halo, and whether the FPS difference is perceptible.

Disable or remove this test mod to return to the final candidate installed below it.
