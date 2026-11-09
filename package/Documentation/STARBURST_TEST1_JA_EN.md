# Diffraction Starburst Test 1 / 回折光条 Test 1

## 日本語

形状ボケとは独立した、撮影向けの回折光条テストです。

- 「回折光条」をONにすると、遠景側で大きくピンぼけした明るい点から放射状の光条を生成します。
- 光条の本数は4・6・8・10・12本から選べます。初期値は6本です。
- 「光条の強さ」「光条の長さ」「光条の回転」「光条の発生しきい値」を調整できます。
- F値へ自動連動し、F2.8付近では短く弱く、F16付近では長く強くなります。
- 明るい面まで光条になる場合は「光条の発生しきい値」を上げてください。
- 全画面で多数の追加サンプルを取得するため、ON時のGPU負荷は大きく増加します。

初回確認は、光条6本、強さ0.75、長さ0.65、しきい値0.65を基準にしてください。効果が見えない場合は強さを1.25、しきい値を0.45へ変更すると確認しやすくなります。

このオーバーレイにはINIを含めません。既存の1.0.6 Full版および古い形状ボケ試験版より後へ配置し、DLLとシェーダーだけを上書きしてください。既存INIでは「回折光条」は初期状態でOFFになります。

## English

This is a screenshot-oriented diffraction-starburst test, independent from the shaped-bokeh kernel.

- Enabling `Diffraction Starburst` creates radial rays from bright points that are strongly defocused on the far plane.
- Ray counts of 4, 6, 8, 10, and 12 are available. The default is 6.
- Strength, length, rotation, and highlight threshold can be adjusted.
- The effect follows the F-number automatically: it is shorter and weaker near F2.8, and longer and stronger near F16.
- Raise the threshold if broad bright surfaces emit unwanted streaks.
- GPU cost rises substantially while enabled because the pass takes many additional full-screen samples.

For the first comparison, use 6 rays, strength 0.75, length 0.65, and threshold 0.65. If the effect is not visible, try strength 1.25 and threshold 0.45.

This overlay intentionally contains no INI. Place it after the existing 1.0.6 Full mod and older shaped-bokeh test overlays so only the DLL and shaders win conflicts. Existing INIs default the new effect to OFF.
