# Cinematic DoF Standalone 1.0.10

## 日本語

- 深度フレームで、被写体を手前に出す辺を上・下・右・左から個別に選べます。既定では全辺が有効です。
- 「ロックオン時のみ」をオンにすると、TDM のロックオン時に枠が約0.22秒でせり出し、解除すると引っ込みます。
- 「通常時もDoFを使用」をオフにしても、有効にした会話・TDM ロックオン・スニークの DoF は発動します。DoF の主スイッチはオンにしてください。
- Community Shaders 系の HDR 出力で深度フレームが消える問題に対応しました。
- 回折光条の強さの上限を16.0、長さの上限を1.50に拡張しました。「光条の明部感度」を追加し、光条を生成する明部の拾いやすさを1.00〜4.00で調整できます。既定値1.00は従来の抽出条件です。
- プリセット8「光条」の初期値を更新しました。明部感度は1.75、長さは0.75、「内部を埋める」はオフです。

新規導入には Full を使用してください。設定を引き継ぐ場合は、MO2 で Update を既存のフル版にマージしてください。Update には INI を含めていません。保存済みのプリセット8に新しい初期値を適用するには、プリセット管理で8番の「初期値へ戻す」を2回押します。枠に保存するには「現在値を保存」を押してください。別途導入した試作版や診断版は無効にしてください。

## English

- Choose which edges let the subject appear in front of the Depth Frame. All four edges are enabled by default.
- Enable Lock-on Only to have the frame slide in when TDM locks onto a target and slide out when the lock is released. Each full transition takes about 0.22 seconds.
- Dialogue, TDM lock-on, and sneak DoF can activate with Use Normal Gameplay DoF turned off. Keep the DoF master switch enabled.
- Fixed the Depth Frame disappearing with Community Shaders HDR output.
- Increased the diffraction ray strength limit to 16.0 and the length limit to 1.50. Starburst Highlight Sensitivity ranges from 1.00 to 4.00 and controls how readily highlights produce rays. The default of 1.00 uses the previous extraction behavior.
- Updated the built-in defaults for preset 8, Starburst. Highlight sensitivity is 1.75, ray length is 0.75, and aperture fill is off.

Use Full for a new installation. To keep your settings, merge Update into the installed full version in MO2. Update omits the INI. To apply the new defaults over a saved preset 8, click Restore Defaults twice for slot 8 in preset management. Use Store Current to save the result to the slot. Disable any separate trial or diagnostic overlays.
