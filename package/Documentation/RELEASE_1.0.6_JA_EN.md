# CinematicDoFStandalone 1.0.6

## 日本語

1.0.6では、Bottled ShadersのReverse Z Depth Bufferとの互換性を追加しました。通常Z／Reverse Zのどちらでも、通常のDoFに加えて「髪を馴染ませる」モード1・2を使用できます。また、実ゲームで調整した組み込みプリセットの初期値を反映しました。

### 主な変更点

- Bottled Shadersの32-bit浮動小数点Reverse Z Depth Bufferへ対応しました。
- Reverse Z有効時も、髪の背景ぼかしに必要な元の深度を専用コピーから取得します。
- 通常ZとReverse Zの両方で、HDR0（`bUse64bitsHDRRenderTarget=0`）および「髪を馴染ませる」モード1・2を実ゲーム確認済みです。
- 「シネマ」の手前ピント範囲を`0.0740728 m`へ更新しました。
- 「人物撮影」の手前ピント範囲を`0.0574862 m`、背景境界の補正を`2.50`へ更新しました。
- 「広角撮影」の手前ピント範囲を`0.100821 m`、背景境界の補正を`2.50`へ更新しました。
- 「一人称撮影」の背景境界の補正を`2.50`へ更新しました。
- 既存INIは互換です。更新版にはINIを含めないため、保存済み設定は変わりません。新しい初期値はFull版の新規導入、または各プリセットの「初期値へ戻す」で利用できます。

### インストール

- 新規導入：`CinematicDoFStandalone-1.0.6-Full-MO2.zip`
- 更新：`CinematicDoFStandalone-1.0.6-Update-Merge-MO2.zip`

更新版にはINIを含めません。MO2では既存MODへ`Merge`してください。`Replace`すると既存INIが削除されます。

## English

Version 1.0.6 adds compatibility with the Reverse Z Depth Buffer in Bottled Shaders. Both Hair Blend modes now work with either conventional or reversed depth. It also incorporates refined built-in preset defaults from in-game testing.

### Highlights

- Adds support for Bottled Shaders' 32-bit floating-point Reverse Z Depth Buffer.
- Preserves the original depth needed by Hair Blend modes in a private copy, including when Reverse Z is enabled.
- Verified in game with conventional Z and Reverse Z, HDR0 (`bUse64bitsHDRRenderTarget=0`), and Hair Blend modes 1 and 2.
- Updates Cinematic near focus range to `0.0740728 m`.
- Updates Photo Portrait near focus range to `0.0574862 m` and edge compensation to `2.50`.
- Updates Photo Wide near focus range to `0.100821 m` and edge compensation to `2.50`.
- Updates First-Person Photo edge compensation to `2.50`.
- Existing INIs remain compatible. The update package contains no INI, so saved settings are preserved. The new defaults are available from a fresh Full installation or with each preset's Restore Defaults action.

### Installation

- New installation: `CinematicDoFStandalone-1.0.6-Full-MO2.zip`
- Update: `CinematicDoFStandalone-1.0.6-Update-Merge-MO2.zip`

The update package intentionally contains no INI. In MO2, choose `Merge` into the existing mod. Do not choose `Replace`, because that removes the existing INI.
