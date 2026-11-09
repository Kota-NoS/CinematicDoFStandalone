# Optional Test4 Transparency Test 2

## 日本語

このオーバーレイは、完成版の通常描画と Test4 透過処理を完全に分離して切り替える検証版です。

- 「Test4透過処理を有効にする」が OFF の場合、完成版と同じ main depth のみを使用します。
- ON の場合だけ、背景ぼかしに post-Z-prepass depth、透過面の保護判定に現在の main depth を使用します。
- Test 1 では両方へ同じ main depth が渡されていたため、深度差が常にゼロとなり、チェックしても効果が出ませんでした。
- post-Z-prepass depth が利用できない環境では、完成版の通常描画へ安全に戻ります。

INI は含みません。前回の CurrentBase-SubjectEdgeRestoration-Test1 の上へ置き換えてください。

## English

This overlay keeps the completed normal renderer and the experimental Test4 transparency path fully separated.

- With “Enable Test4 transparency processing” OFF, rendering continues to use only the completed main-depth path.
- Only when ON, post-Z-prepass depth drives background DoF while current main depth protects detected transparent surfaces.
- Test 1 accidentally supplied the same main depth to both inputs, making the depth difference always zero and the checkbox visually ineffective.
- If post-Z-prepass depth is unavailable, the renderer safely falls back to the completed normal path.

No INI is included. Replace the previous CurrentBase-SubjectEdgeRestoration-Test1 overlay with this package.
