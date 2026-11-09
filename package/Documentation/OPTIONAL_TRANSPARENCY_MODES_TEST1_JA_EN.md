# Optional Transparency Modes Test 1

## 日本語

このオーバーレイは、完成版の通常描画と2種類の透過試作処理を完全に分離して切り替える個人向け検証版です。

- 設定は「詳細設定」の一番下にあります。
- 「透過処理を有効にする」がOFFの場合、モード1・2の処理はどちらも実行されず、完成済みの通常描画を使用します。
- ONの場合のみ、二重深度から透過候補を推定し、右側の「1」「2」で方式を選択できます。
- モード1はTest6です。強い境界補正を透過候補側の外周・急変部へ限定し、広いベール内部では背景ぼかしを維持します。
- モード2はTest8です。モード1へ透過内部の控えめな補正を加え、強い遠景ぼかし時にはTest7方式の空・枝葉の均一遷移を使用します。月は引き続き常に除外・最終復元されます。
- 透過候補の判定は材質フラグではなく深度差を使う推定のため、非常に細い素材は領域全体が境界として扱われる場合があります。

通常描画の透過物の鮮明さ、月の白膜対策、HDR0境界対策、人物境界、シルエット、画面ピント照準、ロックオンフォーカス、スニークモードには変更を加えていません。

更新用ZIPには既存設定を保護するためINIを含めていません。以前のTest6版から更新した場合、既存のON/OFF値を引き継ぎ、モードは1から開始します。

## English

This personal test overlay keeps the completed normal renderer fully separated from two optional transparency experiments.

- The controls are located at the bottom of Advanced Settings.
- With “Enable Transparency Processing” OFF, neither mode runs and the completed normal renderer remains in use.
- Only ON enables the dual-depth transparency-candidate heuristic; the adjacent “1” and “2” buttons select the treatment.
- Mode 1 is Test6. It limits strong boundary compensation to the candidate-side outer edge and abrupt mask transitions, retaining background blur across broad veil interiors.
- Mode 2 is Test8. It adds restrained interior compensation and uses Test7's frame-uniform sky/foliage transition during strong far blur. Moon pixels remain excluded and restored at the final composite.
- Detection is a depth-difference heuristic rather than a material flag, so extremely thin transparent materials may be classified entirely as boundary.

The normal renderer's transparent-material sharpness, moon white-film protection, HDR0 boundary handling, subject boundaries, silhouette mode, screen focus reticle, lock-on focus, and sneak mode are unchanged.

The update ZIP intentionally contains no INI so existing settings are preserved. Updating from the previous Test6 build keeps its ON/OFF value and defaults the new selection to mode 1.
