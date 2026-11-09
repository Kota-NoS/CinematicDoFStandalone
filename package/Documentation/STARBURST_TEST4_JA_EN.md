# Starburst Test 4 / 回折光条 Test 4

## 日本語

Test 4では次の2点を変更しました。

- 「明部抽出しきい値」を絞り形状ボケ内の「形状の回転」直下へ移動しました。
- 回折光条の本数に5本を追加しました。

5本光条に対応するため、光条生成を従来の「1軸から正反対の2本を生成」する方式から、指定した
本数ぶんの方向を1本ずつ等間隔に生成する方式へ変更しました。4・6・8・10・12本では従来と同じ
方向配置と総合的な明るさになるよう正規化しています。

絞り形状ボケがOFFで回折光条だけがONの場合は、明部抽出しきい値を操作できるよう回折光条内に
表示します。絞り形状ボケがONの場合は重複表示せず、形状の回転直下だけに表示します。

このオーバーレイにはINIを同梱していません。既存設定は上書きされません。

## English

Test 4 moves Highlight Extraction Threshold directly below Shape Rotation in the aperture-bokeh block and
adds a five-ray starburst option.

To support odd ray counts, diffraction rays are now emitted as individually spaced directions instead of
paired opposite directions. Even counts retain the previous angular layout and normalized total intensity.
When Aperture Bokeh is disabled but Starburst is enabled, the threshold remains available in the Starburst
block; it is never duplicated while both features are enabled.

No INI is included in this overlay, so existing settings are preserved.
