# Silhouette Photo Mode UI Test 7

## 日本語

Test 7 は、Test 6 の機能とCTD修正を維持したまま、メイン画面のシルエット撮影モード起動アイコンだけを見直した版です。

- ボタンを 28 x 28 の正方形に変更
- 白地に黒い、頭と肩がつながったポップな人物シルエット
- OFF時は白黒のみ
- ON時だけオレンジの外枠と右上の点灯ランプを表示
- シルエット描画、色設定、ホットキー、INI形式には変更なし
- Test 6 の `igGetItemRectMin/Max` ABI修正を維持

### 導入

1. 正式版 v1.0.1 は有効のままにします。
2. Test 6を含む以前のシルエットTestパッケージを無効にします。
3. このZIPをMO2で導入し、正式版 v1.0.1 より後へ配置します。
4. 既存INIは削除しないでください。

### 確認項目

- `Depth of Field` を選択してもCTDしない
- 起動アイコンが正方形に見える
- 人物の頭と肩が離れて見えない
- OFF時は白黒、ON時はオレンジ枠と点灯ランプが出る
- アイコンから小窓を開ける
- ホットキーとシルエット描画がTest 6と同じく動く

## English

Test 7 keeps the Test 6 functionality and crash fix, while redesigning only the Silhouette Photo Mode launcher icon on the main page.

- Uses a square 28 x 28 button
- Shows a bold black person silhouette on a white tile, with the head overlapping the shoulders
- OFF uses black and white only
- ON adds an orange border and a small lit indicator in the upper-right corner
- No changes to silhouette rendering, colors, hotkeys, or the INI format
- Retains the Test 6 `igGetItemRectMin/Max` ABI correction

### Installation

1. Keep official v1.0.1 enabled.
2. Disable Test 6 and every earlier silhouette test package.
3. Install this ZIP with MO2 and place it after official v1.0.1.
4. Do not delete the existing INI.

### Checklist

- Selecting `Depth of Field` does not crash
- The launcher icon is square
- The head and shoulders read as one connected figure
- OFF is black/white; ON adds an orange border and lit indicator
- The icon opens the floating window
- The hotkey and silhouette rendering behave exactly as in Test 6
