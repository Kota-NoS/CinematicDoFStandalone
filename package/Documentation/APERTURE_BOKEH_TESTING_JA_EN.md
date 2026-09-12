# Aperture Bokeh Test 4 historical verification / 絞り形状ボケ Test 4 検証記録

This document records the component test that selected the aperture implementation now used unchanged in version 0.8.32. See `RELEASE_0.8.32_JA_EN.md` for the integrated release.

この文書は、0.8.32正式版へ処理を変更せず採用した絞り実装の選定テスト記録です。統合版については`RELEASE_0.8.32_JA_EN.md`を参照してください。

This is an experimental 0.8.32 Test 4 build based on the public 0.8.31 release. Test 4 keeps Test 2's natural aperture-shaped gather, preserves its brightest eligible shaped sample, and uses `Highlight Boost` to blend toward that peak without exceeding the sampled source highlight. It still requires in-game verification.

これは公開版0.8.31を基にした実験的な0.8.32 Test 4です。Test 2の自然な絞り形状サンプリングを残し、条件を満たす形状付き明点の最大値を保持します。「明るいボケの強調」は、ぼかし平均からその明点へ近づける量を調整します。元のサンプルを超える無制限な明るさは追加しません。ゲーム内動作は未確認です。

## 日本語

1. MO2で公開版0.8.31を一時的に無効化するか、このテスト版が上書きするように配置します。
2. F1メニューから `Cinematic DoF Standalone` → `Depth of Field` → `詳細設定を表示` を開きます。
3. `絞り形状ボケ` をONにします。
4. 最初は次の値をおすすめします。
   - 絞り羽根枚数：3
   - 羽根の丸み：0.00
   - 形状の強さ：1.00
   - 形状の回転：0度
   - ぼかし品質：12～16
   - ボケの密度：0.80前後
   - 仕上げの滑らかさ：0.00
5. 遠景最大ぼかしを強め、暗めの背景にある小さな松明、ろうそく、魔法、窓などの明るい点を見ます。「明るいボケの強調」は0.00から始め、0.10、0.20、0.30の順に比較してください。Test 4では、それぞれ形状付き明点へ0%、30%、60%、90%近づきます。手前側も、強い手前ぼかしと近くの明るい点で確認できます。

確認してほしい内容：

- OFF時の映像が0.8.31と変わらない。
- 「明るいボケの強調」0.00ではTest 2相当の自然な形になり、0.10～0.30へ上げると明るいボケの形が読み取りやすくなる。
- 羽根3・丸み0・強さ1と、羽根8・丸み0・強さ1を比較すると、明るいボケの形が変わる。
- 羽根の丸みを1.00へ近づけると円形へ近づく。
- 形状の強さを0.00へ近づけると通常の円形ボケへ近づく。
- 羽根3または5の状態で形状の回転を0度と90度へ変えると、多角形の向きが変わる。
- 明るい部分が過度に白飛びせず、点滅、目立つ輪郭、通常の背景が白っぽくなる現象がない。
- 黒画面、DoF消失、ちらつき、クラッシュ、極端なFPS低下がない。
- プリセット保存・適用、次回起動設定、INI再読み込み、再起動、初期値へ戻すで新しい値が正しく保存・復元される。

この機能は画面へ人工的な光点を追加しません。元の映像に存在し、強くピンぼけした明るい点ほど形状が見えやすくなります。

## English

1. Temporarily disable public version 0.8.31 in MO2, or place this test build after it so the test files win conflicts.
2. Open `Cinematic DoF Standalone` → `Depth of Field` → `Show Advanced Settings` from the F1 menu.
3. Enable `Aperture-Shaped Bokeh`.
4. Recommended starting values:
   - Aperture Blades: 3
   - Blade Roundness: 0.00
   - Shape Strength: 1.00
   - Shape Rotation: 0 degrees
   - Blur Quality: 12–16
   - Bokeh Density: around 0.80
   - Post Blur Smoothing: 0.00
5. Use strong far blur and observe small torches, candles, magic effects, windows, or other bright points against a darker background. Start with Highlight Boost 0.00, then compare 0.10, 0.20, and 0.30. Test 4 maps these to a 0%, 30%, 60%, and 90% blend toward the eligible shaped-highlight peak. Near bokeh can be checked with strong near blur and a nearby bright point.

Please verify that:

- Disabling the feature produces the same image as 0.8.31.
- Highlight Boost 0.00 should retain the natural Test 2 result; increasing it through 0.10–0.30 should make bright bokeh shapes easier to read.
- Comparing 3 blades / roundness 0 / strength 1 against 8 blades / roundness 0 / strength 1 should change the bright bokeh silhouette.
- Roundness near 1.00 approaches a circle.
- Strength near 0.00 approaches the standard circular bokeh.
- With 3 or 5 blades selected, changing rotation from 0 to 90 degrees changes the polygon orientation.
- Highlights do not clip excessively, pulse, form objectionable halos, or wash out ordinary scene detail.
- There is no black screen, missing DoF, flicker, crash, or excessive FPS regression.
- Preset store/apply, startup settings, INI reload, game restart, and Restore Defaults correctly preserve or reset all new values.

This feature does not create artificial light spots. Its shape is most visible on bright points that already exist in the image and are strongly out of focus.
