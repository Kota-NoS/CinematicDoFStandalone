# Cinematic DoF Standalone 1.0.13

## 日本語

- 深度フレームの表示条件に「キルムーブ時」「会話時」を追加しました。「ロックオン時」と併用でき、選択した条件のいずれかを満たすと表示します。
- 表示条件に関係なく枠を表示する「撮影モード」を追加しました。撮影用の枠設定は演出用と独立して保存され、有効な間は文字がオレンジ色になります。
- 「被写体を枠の手前に出す」がオフのとき、ロックオン状態の更新が止まる問題を修正しました。
- 既存のアップスケール・HDR対応を維持しています。

既存の設定を引き継ぐ場合は Update、新規導入には Full を使用してください。試作版・診断版は無効にしてください。演出用のはみ出しは初期オフ、撮影用は初期オンです。その他の撮影用設定は初回に演出用設定を引き継ぎ、「次回起動設定を保存」で保存できます。対象選択と髪の設定は共通です。

## English

- Added During Killmove and During Dialogue conditions for Depth Frame. These can be combined with During Target Lock; any selected condition can display the frame.
- Added Photography Mode, which displays the frame regardless of those conditions and keeps separate frame appearance settings. Its label turns orange while active.
- Fixed stale target-lock state when subject breakout was disabled.
- Retained existing upscaling and HDR compatibility fixes.

Use Update to preserve your existing INI, or Full for a new installation. Disable trial and diagnostic overlays. Subject breakout defaults to off for cinematic mode and on for photography. Other photography settings initially inherit the cinematic settings and can be saved with Save Startup Settings. Subject selection and hair settings remain shared.
