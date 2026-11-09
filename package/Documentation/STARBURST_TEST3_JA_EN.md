# Starburst Test 3 / 回折光条 Test 3

## 日本語

Test 3では設定の所属を整理し、光条の強調範囲を拡張しました。

### UIの整理

「画質・ボケの詳細」の共通設定を先にまとめました。

- ぼかし品質
- 背景境界の補正
- ボケの密度
- 周辺ボケの強さ
- 明るいボケの強調
- 仕上げの滑らかさ
- 明部抽出しきい値

「ボケ中心の補光」は絞り形状ボケ専用の項目として「形状の回転」の直後へ移動しました。
回折光条をOFFにしても、絞り形状ボケがONなら使用できます。

「明部抽出しきい値」は回折光条とボケ中心の補光で共用します。旧Test 2の
`StarburstThreshold` / `StarburstCoreFill` は読み込み時に自動移行されます。

### 光条の強さ

光条の強さの上限を4.0から8.0へ拡張しました。計算倍率はTest 2から変更していないため、
0.0～4.0の既存設定は同じ見え方です。4.0を超える範囲は撮影向けの強い強調用です。

このオーバーレイにはINIを同梱していません。既存設定は上書きされません。

## English

Test 3 reorganizes controls by ownership and extends the starburst intensity range.

Common quality and bokeh controls now appear before the feature-specific blocks. Bokeh Core Fill is placed
directly below Shape Rotation and works whenever Aperture Bokeh is enabled, independently of Diffraction
Starburst. Highlight Extraction Threshold is shared by Core Fill and Starburst.

Legacy Test 2 keys `StarburstThreshold` and `StarburstCoreFill` are migrated when read. Starburst Strength
now extends from 0.0 to 8.0. Its multiplier is unchanged from Test 2, so existing values through 4.0 retain
the same appearance; the additional range is intended for strongly emphasized screenshots.

No INI is included in this overlay, so existing settings are preserved.
