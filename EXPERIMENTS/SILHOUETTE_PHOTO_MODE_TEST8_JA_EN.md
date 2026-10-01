# Silhouette Photo Mode UI Test 8

## 日本語

Test 8 は、Test 7で確認済みの機能とCTD修正を維持し、起動アイコンの視認性と操作を改善した版です。

- 起動ボタンを 28 x 28 から 36 x 36 へ拡大
- 内部余白を減らし、白黒の人物シルエットも約1.4倍へ拡大
- OFF時は白黒のみ、ON時はオレンジ枠と右上の点灯ランプを表示
- 起動アイコンを再度クリックすると専用ウインドウを閉じる
- ツールチップも小窓の現在状態に応じて「開く／閉じる」を切り替える
- シルエット描画、色設定、ホットキー、INI形式には変更なし
- Test 6以降の `igGetItemRectMin/Max` ABI修正を維持

### 導入

1. 正式版 v1.0.1 は有効のままにします。
2. Test 7を含む以前のシルエットTestパッケージを無効にします。
3. このZIPをMO2で導入し、正式版 v1.0.1 より後へ配置します。
4. 既存INIは削除しないでください。

### 確認項目

- `Depth of Field` を選択してもCTDしない
- 起動アイコンと人物がTest 7より大きく見える
- OFF時は白黒、ON時はオレンジ枠と点灯ランプが出る
- 1回目のクリックで小窓が開く
- 2回目のクリックで同じ小窓が閉じる
- 小窓右上の閉じるボタンも従来どおり動く
- ホットキーとシルエット描画がTest 7と同じく動く

## English

Test 8 keeps the Test 7 functionality and crash fix while improving launcher visibility and interaction.

- Enlarges the launcher button from 28 x 28 to 36 x 36
- Reduces internal padding and makes the black-and-white person approximately 1.4 times larger
- OFF remains black/white; ON adds the orange border and upper-right indicator
- Clicking the launcher again closes the dedicated window
- The tooltip now changes between Open and Close according to the window state
- No changes to silhouette rendering, colors, hotkeys, or the INI format
- Retains the `igGetItemRectMin/Max` ABI correction from Test 6 onward

### Installation

1. Keep official v1.0.1 enabled.
2. Disable Test 7 and every earlier silhouette test package.
3. Install this ZIP with MO2 and place it after official v1.0.1.
4. Do not delete the existing INI.

### Checklist

- Selecting `Depth of Field` does not crash
- The launcher and person are visibly larger than in Test 7
- OFF is black/white; ON adds the orange border and lit indicator
- The first click opens the floating window
- The second click closes the same window
- The window's own close button still works
- The hotkey and silhouette rendering behave exactly as in Test 7
