# Starburst Test 6 / 回折光条 Test 6

## 日本語

Test 6では、2026年10月21日23:33更新のユーザー調整済みINIを基準に、プリセット7と8の内蔵初期値を更新します。

- 7番「絞り形状」：`内部を滑らかに埋める`をON
- 8番「光条」：短い表示名へ変更し、6枚羽根・6本光条・強さ6.00・長さ0.90・明部抽出しきい値0.54・中心補光0.50を採用
- Custom 1／2：設定と内部IDを保ったまま、表示位置だけ9／10番へ移動

`Preset.ApertureBokeh`、`Preset.ShapeStarburst`、`Preset.Custom1`、`Preset.Custom2`のセクション名と内部IDは変更しません。
このテスト用オーバーレイにはINIを含めないため、既存の保存値も上書きしません。各枠の「初期値へ戻す」を使用すると、更新した内蔵初期値を確認できます。

## English

Test 6 updates the bundled defaults for slots 7 and 8 from the user-tuned INI last modified at 23:33 on
October 21, 2026. Slot 7 now enables filled aperture sampling. Slot 8 is renamed `Starburst` and uses six
aperture blades, six diffraction rays, strength 6.00, length 0.90, threshold 0.54, and core fill 0.50.
Custom 1 and Custom 2 retain their settings, stable storage IDs, and named INI sections while moving only
visually to slots 9 and 10. The test overlay contains no INI and therefore does not overwrite saved values.
