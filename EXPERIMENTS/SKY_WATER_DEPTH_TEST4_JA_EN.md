# Sky/Water Dual-Depth Test 4 / 空・水面デュアル深度 Test 4

## 日本語

これは正式版ではなく、Community Shaders有効時に残る海面の境界を調べる実験版です。Test 3の空境界保護を基準にしています。

### Test 3からの変更

- 通常のDoF、オートフォーカス、人物保護、CoC、遠景の深度境界判定は、従来どおり現在選択されている深度を使います。
- 空かどうかの判定だけは、Community Shadersの水描画後に利用できるメイン深度を別途参照します。
- 低設定用深度はCommunity Shadersが水描画前に複製するため、その深度だけを見ると海面が未描画の空として誤判定される場合があります。本Testは空判定と通常DoFの役割を分離します。
- 新しいテクスチャ、深度コピー、レンダーパス、ディスパッチは追加していません。既存のメイン深度を追加のSRVとして参照するだけです。

したがって、本Testは低設定用深度の精度や人物・地形のDoF計算を変更しません。水面そのもののCoCは引き続き低設定用深度に依存するため、これは「水面の物理的に正確な焦点距離」を作る修正ではなく、海面を空と誤認して鮮明な色を復元する境界を止める診断です。

### 導入

1. Skyrimを終了します。
2. MO2で正式候補版より後に本Test版を配置し、本Test版のDLLとシェーダーが競合に勝つことを確認します。
3. Test 1～3を含む過去のTest版・診断版は同時に有効化しないでください。
4. INIは含まれません。保存済み設定、プリセット、言語設定は変更されません。
5. Community Shadersを有効にし、これまで境界が見えた同じ海岸・同じ強い奥ぼけで比較します。

### 優先確認箇所

- 海面上に残っていた水平な境界が消えるか、位置や幅が変化するか
- 海面と空の水平線、遠くの船・島・氷山
- Test 3で改善した木、山、建物と空の境界が維持されるか
- 人物追従、会話中DoF、画面位置AF
- カメラ移動中のちらつき、瞬間的な線、色の漏れ
- 可能であればCommunity Shaders OFFでの簡単な回帰確認

海面の境界が完全には消えなくても、変化の有無で「水描画前深度による空誤判定」が原因かどうかを切り分けられます。

## English

This is an experimental build, not a release. It investigates the remaining sea boundary with Community Shaders enabled and builds on Test 3's sky-boundary protection.

### Changes from Test 3

- Normal DoF, autofocus, actor protection, CoC, and far-depth rejection continue to use the currently selected depth path.
- Only sky classification reads the current main depth that is available after Community Shaders has rendered water.
- Community Shaders copies the low-spec fallback depth before water rendering. Looking only at that copy can therefore misclassify water as untouched clear sky. This test separates sky classification from normal DoF depth.
- No texture, depth copy, render pass, or dispatch was added. The existing main depth is bound as one additional SRV.

This test therefore does not lower the precision of the low-spec depth or change the normal actor/terrain DoF calculation. Water CoC still depends on the low-spec depth, so this is not a physically exact water-surface focus fix. It is a diagnostic intended to stop sharp source colour from being restored where water was mistaken for sky.

### Installation

1. Exit Skyrim.
2. In MO2, place this test after the final candidate and confirm that both its DLL and shader win conflicts.
3. Do not enable Test 1-3 or any earlier diagnostic/test build at the same time.
4. No INI is included. Saved settings, presets, and language selection are preserved.
5. Enable Community Shaders and compare the same coast with the same strong far blur that previously showed the seam.

### Priority scenes

- Whether the horizontal boundary over the sea disappears, moves, or changes width
- The sea/sky horizon and distant ships, islands, and icebergs
- Whether Test 3's improved tree, mountain, building, and sky boundaries remain intact
- Actor tracking, dialogue DoF, and screen-position autofocus
- Flicker, transient lines, or colour leakage while moving the camera
- A quick Community Shaders OFF regression check if practical

Even if the seam does not disappear completely, whether it changes will distinguish a pre-water-depth sky misclassification from other causes.
