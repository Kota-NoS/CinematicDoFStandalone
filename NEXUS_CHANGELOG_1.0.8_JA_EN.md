# CinematicDoFStandalone 1.0.8

## 日本語

### 改訂版の変更点

- Reverse Z が有効な環境で、通常のシルエット撮影モードの月が空に溶け込む問題を修正しました。

- 深度フレームで「被写体の境目を拡張」が人物の切り抜きに反映されるよう修正しました。上限は 0.300（約6 px）です。従来の 0.150 は約3 px に対応し、値を上げると背景色を太い縁として残せます。
- 「髪を馴染ませる」が有効なとき、半透明と推定された輪郭付近では、拡張で足した帯に枠色を戻して背景の縁を馴染ませます。
- 深度フレームからも「髪を馴染ませる」のオン・オフとモードを操作・保存できます。メイン画面と共通の設定です。

### 1.0.8 の主な変更点

- 深度フレームを追加。上下・四辺・個別の幅、枠の回転、色、不透明度を調整できます。メイン画面上部の二本線アイコンから専用ウィンドウを開けます。
- 被写体と装備を枠の手前に表示できます。輪郭の内側補正で色のにじみを調整できます。値を上げると細い毛先やレースが欠ける場合があります。
- Private Profile Redirector 導入環境で次回起動設定を保存できるよう、INIの書き込み方式を変更しました。
- シルエット撮影モードに CoC モノクロを追加。ピントやレンズ設定に応じて、人物や風景の濃淡が変化します。
- プリセット8（光芒）と10（カスタム2）の同梱初期値を調整しました。

既存の設定を引き継ぐ場合は、MO2 で Update を既存のフル版へマージしてください。新規導入には Full を使用してください。試作版や重複した旧版の上書き MOD は無効にしてください。

## English

### Revised build

- Fixed the moon blending into the sky in standard Silhouette Photo Mode when Reverse Z is enabled.

- Fixed Expand Subject Boundary so it affects the actor cutout in the depth-aware frame. The new limit is 0.300 (about 6 px). The previous limit of 0.150 still gives about 3 px. Higher values can leave a thicker rim of scene color around the subject.
- When Blend Hair is enabled, the frame color is blended back into the added margin near likely translucent edges to soften the background rim.
- Blend Hair and its two modes can now be adjusted and saved from the Depth Frame window. The controls share the main panel's settings.

### Highlights from 1.0.8

- Added a depth-aware frame with letterbox, four-sided, and independent edge controls, plus rotation, color, and opacity. Open its settings from the two-bar icon in the main panel.
- Subject Breakout keeps actors and their equipment in front of the frame. The inward contour control can reduce color fringes. Higher values can erode fine hair and lace.
- Changed INI writing to support saving startup settings with Private Profile Redirector installed.
- Added CoC Monochrome to Silhouette Photo Mode. Focus and lens settings change the tonal rendering of the subject and scenery.
- Retuned the bundled defaults for presets 8 (Starburst) and 10 (Custom 2).

For existing installations, merge the Update package into the existing mod in MO2 to keep your INI. Use Full for a fresh installation. Disable trial builds and duplicate older overlays when switching to this release.
