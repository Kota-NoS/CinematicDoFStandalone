# CinematicDoFStandalone 1.0.1

## 日本語

1.0.1は、人物追従時の保護を実深度ベースへ統合し、空をDoFから自然に除外する機能を追加する更新です。海面、遠景、明るいボケを維持しながら、空との境界に生じる不自然な線や発光を抑えます。

### 主な変更

- 「詳細設定を表示」の右側へ「空をぼかさない」を追加しました。初期値はONで、プリセットごとに保存されます。
- 未描画のメイン深度から空を判定し、空だけを最終DoF合成から除外します。
- 地形、樹木、建物などの輪郭では、近傍深度を使って空との境界を滑らかに補正します。
- 水面については、低設定用深度とは分離したメイン深度を空判定に使い、Community Shaders併用時に海面へ生じる境目を解消しました。
- 「空をぼかさない」がONの場合、空を絞り形状ボケと通常の「明るいボケの強調」の両方から除外します。空以外の灯火、水面反射、雪、魔法効果などは従来どおり強調されます。
- 人物追従時の保護を、画面上の人物範囲と実深度を組み合わせる共通経路へ統合しました。首周りや画面端の背景を人物として誤保護しにくくなります。
- Community Shaders併用時の人物追従では、HDR設定にかかわらず手前ピント範囲の実効値を最低0.17 mにします。非人物の追従対象には適用しません。
- Community Shadersなしでも同じ人物保護を使用します。64-bit HDRが明示的に無効な場合の0.15 m補助も維持します。
- 会話限定DoFで、会話開始直後の描画遅延を減らすためレンダラー資源を事前準備します。
- 既存INIに「空をぼかさない」の保存値がない場合はONとして読み込みます。既存のプリセットID、Custom 1／2、その他の保存値は移動・上書きしません。

### 実機確認

- Skyrim AE 1.6.1170で、Community Shadersあり／なしの両方を確認しました。
- Community Shaders併用時、空と海面、遠景、船、氷山の境界が自然に保たれることを確認しました。
- 絞り形状ボケON／OFFの両方で、「明るいボケの強調」を上げても空境界の白色・水色の発光が抑えられることを確認しました。
- `bUse64bitsHDRRenderTarget=0`でも水面と空の除外が機能し、人物が鮮明に保たれることを確認しました。
- 空以外の水面反射と明るいボケが維持されることを確認しました。

### 既知の制限

- 月など深度を書き込む天体は空ではなく通常の遠景として扱われるため、ぼける場合があります。
- 非常に大きなボケを低い「ぼかし品質」で描画すると、サンプル密度不足によりボケが輪や点へ分裂する場合があります。

### 更新方法

既存環境ではINIを含まない`Update-Merge-MO2`版を既存MODへ`Merge`してください。`Replace`すると既存INIも削除されます。新規導入では配布時初期値のINIを含む`Full-MO2`版を使用してください。

## English

Version 1.0.1 unifies tracked-actor protection around measured depth and adds a natural sky-preserving DoF path. It removes unnatural lines and bright outlines at the sky boundary while retaining water, distant scenery, and non-sky bright bokeh.

### Main changes

- Added `Keep Sky Sharp` beside `Show Advanced Settings`. It defaults to on and is stored per preset.
- Clear sky is classified from unwritten main depth and excluded from the final DoF composite.
- Boundary feathering uses neighbouring depth to keep silhouettes of terrain, trees, and buildings natural against the sky.
- Sky classification now uses main depth independently of the low-setting fallback depth, removing the sea boundary seen with Community Shaders.
- With `Keep Sky Sharp` enabled, sky is excluded from both aperture-shaped highlight extraction and the general `Highlight Boost` path. Non-sky lamps, water reflections, snow, and magic effects retain their highlight emphasis.
- Tracked-actor protection now combines the projected actor area with measured depth in one common path, reducing false protection around neck gaps and screen-edge background.
- With Community Shaders, tracked actors receive an effective near-focus floor of 0.17 m regardless of HDR settings. Non-actor targets do not receive this floor.
- The same actor protection is used without Community Shaders, retaining the 0.15 m assist when 64-bit HDR is explicitly disabled.
- Dialogue-only DoF prewarms renderer resources to reduce the delay when a conversation begins.
- Existing INIs and presets without `Keep Sky Sharp` load it as enabled. Preset IDs, Custom 1/2, and all other saved values are preserved.

### In-game verification

- Tested on Skyrim AE 1.6.1170 both with and without Community Shaders.
- Confirmed natural boundaries between sky, sea, distant terrain, ships, and icebergs with Community Shaders.
- Confirmed that increasing `Highlight Boost`, with aperture-shaped bokeh both enabled and disabled, no longer produces white or cyan glow along the sky boundary.
- Confirmed sky and water exclusion with `bUse64bitsHDRRenderTarget=0`, while tracked actors remain sharp.
- Confirmed that non-sky water reflections and bright bokeh remain available.

### Known limitations

- Moons and other sky objects that write depth are treated as ordinary distant geometry and may still blur.
- Very large blur discs rendered at low `Blur Quality` can separate into visible rings or points because of insufficient sample density.

### Updating

For an existing installation, merge the INI-free `Update-Merge-MO2` package into the existing mod. Do not use `Replace`, because it deletes the existing INI. Use the `Full-MO2` package for a new installation; it includes the bundled default INI.
