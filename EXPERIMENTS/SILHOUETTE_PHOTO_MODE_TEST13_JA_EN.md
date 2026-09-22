# Silhouette Photo Mode UI Test 13

## 日本語

Test 13 は、Test 12の中央円に灰色の縞と中心の白い点が見えた問題を修正した版です。

- 白い半円を多数の三角形ではなく、1枚の連続した凸ポリゴンとして描画
- 内部のアンチエイリアス境界をなくし、白い部分を均一化
- 中心を共有する三角形を廃止し、中央の白い点を除去
- 中央円の大きさ、斜め二分の向き、ボタン枠はTest 12と同じ
- ON時のオレンジ枠と点灯ランプ、小窓開閉、ホットキー、INI形式には変更なし

### 導入

1. 正式版 v1.0.1 は有効のままにします。
2. Test 12を含む以前のシルエットTestパッケージを無効にします。
3. このZIPをMO2で導入し、正式版 v1.0.1 より後へ配置します。
4. 既存INIは削除しないでください。

### 確認項目

- 円の左上半分が均一な白に見える
- 円の中心に白い点や放射状の縞が出ない
- 円が斜線を境に白黒で明確に二分される
- ON時はオレンジ枠と点灯ランプが出る
- 同じアイコンで専用小窓を開閉できる

## English

Test 13 fixes the gray radial seams and white center point visible in the Test 12 center circle.

- Draws the white semicircle as one continuous convex polygon instead of many triangles
- Removes internal anti-aliased edges so the white half is uniform
- Removes the shared fan center that produced the bright center pinhole
- Keeps the Test 12 circle size, diagonal direction, and button frame
- No changes to the orange active border/lamp, window toggling, hotkeys, or INI format

### Installation

1. Keep official v1.0.1 enabled.
2. Disable Test 12 and every earlier silhouette test package.
3. Install this ZIP with MO2 and place it after official v1.0.1.
4. Do not delete the existing INI.
