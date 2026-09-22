# Silhouette Photo Mode UI Test 10

## 日本語

Test 10 は、Test 9で確認済みの枠サイズと動作を維持し、人物シルエットをより丸く柔らかい形へ変更した版です。

- 起動ボタンは引き続きMenu Frameworkの標準フレーム高を使用
- 頭を少し大きな円に変更
- 肩と上半身を大きな円で構成し、画像サンプルに近い丸い輪郭へ変更
- 円の下半分からタイル下端まで胴体を接続
- OFF時は白黒、ON時はオレンジ枠と右上の点灯ランプを表示
- 小窓の開閉、シルエット描画、色設定、ホットキー、INI形式には変更なし

### 導入

1. 正式版 v1.0.1 は有効のままにします。
2. Test 9を含む以前のシルエットTestパッケージを無効にします。
3. このZIPをMO2で導入し、正式版 v1.0.1 より後へ配置します。
4. 既存INIは削除しないでください。

### 確認項目

- 起動アイコンの高さが横のホットキーボタンと揃う
- 人物が画像サンプルのような丸い輪郭に見える
- 頭と胴体が離れて見えない
- 胴体が白いタイルの下端まで続く
- OFF時は白黒、ON時はオレンジ枠と点灯ランプが出る
- 同じアイコンで小窓を開閉できる
- ホットキーとシルエット描画がTest 9と同じく動く

## English

Test 10 keeps the verified Test 9 frame size and behavior while giving the person a softer, rounder silhouette.

- Continues to use Menu Framework's standard frame height for the launcher
- Uses a slightly larger circular head
- Builds the shoulders and upper torso from a large circle for a rounder sample-like outline
- Connects the lower half of that circle to the bottom edge of the tile
- Keeps OFF black/white and ON indicated by an orange border and upper-right lamp
- No changes to window toggling, silhouette rendering, colors, hotkeys, or the INI format

### Installation

1. Keep official v1.0.1 enabled.
2. Disable Test 9 and every earlier silhouette test package.
3. Install this ZIP with MO2 and place it after official v1.0.1.
4. Do not delete the existing INI.
