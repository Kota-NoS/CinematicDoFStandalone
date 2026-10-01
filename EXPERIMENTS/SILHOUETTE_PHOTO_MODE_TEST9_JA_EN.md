# Silhouette Photo Mode UI Test 9

## 日本語

Test 9 は、Test 8で確認済みの動作を維持し、起動アイコンの寸法と人物デザインを調整した版です。

- 起動ボタンの縦横をMenu Frameworkの標準フレーム高へ合わせる
- 横にあるDoFホットキーボタンと同じ高さで表示
- 人物を「頭・丸い肩・下端まで続く胴体」の一体型ピクトグラムへ変更
- 白い余白の下で胴体が途切れず、タイル下端まで接続
- OFF時は白黒、ON時はオレンジ枠と右上の点灯ランプを表示
- 同じ起動アイコンによる小窓の開閉を維持
- シルエット描画、色設定、ホットキー、INI形式には変更なし

### 導入

1. 正式版 v1.0.1 は有効のままにします。
2. Test 8を含む以前のシルエットTestパッケージを無効にします。
3. このZIPをMO2で導入し、正式版 v1.0.1 より後へ配置します。
4. 既存INIは削除しないでください。

### 確認項目

- `Depth of Field` を選択してもCTDしない
- 起動アイコンの高さが横のホットキーボタンと揃う
- 人物の頭・肩・胴体が一続きに見える
- 人物の胴体が白いタイルの下端まで続く
- OFF時は白黒、ON時はオレンジ枠と点灯ランプが出る
- 同じアイコンで小窓を開閉できる
- ホットキーとシルエット描画がTest 8と同じく動く

## English

Test 9 keeps the verified Test 8 behavior while refining the launcher dimensions and person design.

- Sizes the square launcher from Menu Framework's standard frame height
- Matches the height of the adjacent DoF hotkey button
- Replaces the person with one connected pictogram: head, rounded shoulders, and a torso continuing to the bottom edge
- Keeps OFF black/white and ON indicated by an orange border and upper-right lamp
- Keeps the same launcher-button open/close toggle
- No changes to silhouette rendering, colors, hotkeys, or the INI format

### Installation

1. Keep official v1.0.1 enabled.
2. Disable Test 8 and every earlier silhouette test package.
3. Install this ZIP with MO2 and place it after official v1.0.1.
4. Do not delete the existing INI.

### Checklist

- Selecting `Depth of Field` does not crash
- The launcher height matches the adjacent hotkey button
- The head, shoulders, and torso read as one connected person
- The torso continues to the bottom edge of the white tile
- OFF is black/white; ON adds the orange border and lit indicator
- The same icon opens and closes the floating window
- The hotkey and silhouette rendering behave exactly as in Test 8
