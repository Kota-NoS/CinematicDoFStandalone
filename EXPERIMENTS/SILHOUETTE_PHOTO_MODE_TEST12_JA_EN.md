# Silhouette Photo Mode UI Test 12

## 日本語

Test 12 は、Test 11で中央円の反転方向が背景と重なり、円がほとんど見えなかった問題を修正した版です。

- 画面座標のY軸方向を考慮し、白く塗る半円を正しい左上側へ反転
- 中央円を少し大きくして、小さいボタンでも形を認識しやすく調整
- 円周の分割数を増やして輪郭を滑らかに調整
- 枠サイズ、クリック範囲、ON時のオレンジ枠と点灯ランプはTest 11と同じ
- 同じアイコンによる専用小窓の開閉、ホットキー、シルエット描画、INI形式には変更なし

### 導入

1. 正式版 v1.0.1 は有効のままにします。
2. Test 11を含む以前のシルエットTestパッケージを無効にします。
3. このZIPをMO2で導入し、正式版 v1.0.1 より後へ配置します。
4. 既存INIは削除しないでください。

### 確認項目

- 左上の黒地では中央円が白く見える
- 右下の白地では中央円が黒く見える
- 中央円が斜線を境に明確に反転して見える
- ON時はオレンジ枠と点灯ランプが出る
- 同じアイコンで専用小窓を開閉できる

## English

Test 12 fixes the Test 11 issue where the inverted half of the center circle overlapped matching background colors and made the circle nearly disappear.

- Corrects the semicircle direction for screen-space coordinates so the upper-left half is white
- Slightly enlarges the center circle for better readability at the small button size
- Uses more arc segments for a smoother circle edge
- Keeps the Test 11 frame size, click target, orange active border, and active lamp
- No changes to window toggling, hotkeys, silhouette rendering, or the INI format

### Installation

1. Keep official v1.0.1 enabled.
2. Disable Test 11 and every earlier silhouette test package.
3. Install this ZIP with MO2 and place it after official v1.0.1.
4. Do not delete the existing INI.
