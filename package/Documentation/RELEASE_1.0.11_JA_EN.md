# Cinematic DoF Standalone 1.0.11

## 日本語

- 公式 Community Shaders 1.9.1 の FSR・DLSS で、縮小解像度を選ぶと DoF が画面左上の一部だけにかかる問題を修正しました。
- アップスケール前に取得した深度の座標を補正し、縮小解像度でも通常 DoF の「髪をなじませる」が働くようにしました。公式 Community Shaders と Bottled Shaders（Reverse Z Depth 有効）で確認済みです。

新規導入には Full を使用してください。既存設定を引き継ぐ場合は、MO2 で Update を既存版にマージしてください。別途導入した試作版・診断版は無効にしてください。

## English

- Fixed DoF affecting only the upper-left portion of the screen when using sub-native FSR or DLSS in Community Shaders 1.9.1.
- Hair blending in regular DoF now maps pre-upscale depth to the output resolution. Tested with Community Shaders and with Bottled Shaders with Reverse Z Depth enabled.

Use Full for a new installation, or merge Update into the installed version in MO2 to keep your INI settings. Disable separate trial and diagnostic overlays.
