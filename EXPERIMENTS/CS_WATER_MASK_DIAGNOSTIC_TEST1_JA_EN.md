# Community Shaders Water Mask Diagnostic Test 1 / CS水面マスク診断 Test 1

## 日本語

これは正式版ではありません。CinematicDoFStandalone 1.0.1を基準に、Community Shadersが水面合成に利用している生の水面レンダーターゲットを、DoF側から直接読み取れるか確認する診断版です。正式版1.0.1のソースや設定は変更しません。

通常のDoF映像の代わりに、`kRAW_WATER` のZ成分をCommunity Shadersの`ISWaterBlend`と同じ閾値で可視化します。

### 色の意味

- シアン：水面マスクあり。水として判別できた領域です。
- 黒：水面マスクなし。空、月、地形、人物などはこの色になる想定です。
- 全面マゼンタ：`kRAW_WATER`のShader Resource Viewを取得できませんでした。空のマスクとは別の失敗表示です。

水面がシアン、月が黒になれば、正式版の空判定から水面だけを差し引く軽量な経路を検討できます。水面まで黒の場合は、実行順の時点ですでにマスクが消えている、別のレンダーターゲットが使われている、または対象成分が異なる可能性があります。

### 導入と確認

1. Skyrimを完全に終了します。起動中にDLLやシェーダーを差し替えないでください。
2. MO2で正式版1.0.1より後に本診断版を配置します。
3. 本診断版はDLLとシェーダーを上書きしますが、INIとプリセットは含みません。
4. Community Shadersを有効にし、Cinematic DoFをONにします。
5. 月と水面が同時に見える場面、近い水面、遠い水面、川・湖・海、可能なら水中でも撮影してください。
6. 全面マゼンタ、全面黒、または想定外の色になった場合は、`Documents/My Games/Skyrim Special Edition/SKSE/CinematicDoFStandalone.log`も保存してください。

ログには次のどちらかが一度だけ記録されます。

- `CS water-mask diagnostic: kRAW_WATER is available (...)`
- `CS water-mask diagnostic: kRAW_WATER has no readable shader-resource view`

診断終了後は本Modを無効化または削除すると、下にある正式版1.0.1へ戻ります。

## English

This is an experimental diagnostic, not a release. It is based on CinematicDoFStandalone 1.0.1 and tests whether the DoF plugin can directly read the raw water render target used by Community Shaders. It does not modify the official 1.0.1 branch or saved settings.

Instead of the normal DoF image, it visualizes the Z component of `kRAW_WATER` using the same thresholds as Community Shaders' `ISWaterBlend` shader.

### Colour key

- Cyan: water-mask coverage was detected.
- Black: no water-mask coverage. Sky, moons, terrain, actors, and other non-water areas are expected to be black.
- Full-screen magenta: no readable shader-resource view was available for `kRAW_WATER`. This is deliberately distinct from a valid empty mask.

If water is cyan while the moon remains black, a lightweight path can subtract water from the existing sky candidate. If water is also black, the mask may already be unavailable at this point in the frame, another target may be used, or a different channel may be required.

### Installation and test

1. Exit Skyrim completely. Do not replace the DLL or shader while the game is running.
2. Place this diagnostic after the official 1.0.1 mod in MO2.
3. It overrides the DLL and shader, but contains no INI or presets.
4. Enable Community Shaders and enable Cinematic DoF.
5. Capture the moon and water together, near and distant water, rivers/lakes/ocean, and an underwater view if possible.
6. If the frame is entirely magenta, entirely black, or otherwise unexpected, also save `Documents/My Games/Skyrim Special Edition/SKSE/CinematicDoFStandalone.log`.

The log records one of these messages once:

- `CS water-mask diagnostic: kRAW_WATER is available (...)`
- `CS water-mask diagnostic: kRAW_WATER has no readable shader-resource view`

Disable or remove this diagnostic to return to the official 1.0.1 installed below it.
