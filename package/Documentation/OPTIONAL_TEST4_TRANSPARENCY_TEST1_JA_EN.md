# Optional Test4 Transparency — Test1

## 日本語

このパッケージは、完成版の安定描画を既定値のまま維持しながら、Test4で試作した「透過物の後ろをぼかす」処理を任意に切り替える個人向け検証版です。

- 既定値はOFFです。OFFでは完成版1.0.3の描画計算を使用します。
- ONでは、2種類の深度の差からピント付近の髪・ベールなどを推定します。
- 推定領域では、透過物越しの背景ぼかしを維持しながら、細部と限定的な中間ディテールを復元します。
- 材質フラグを直接読んでいるわけではないため、結果はヒューリスティックです。素材や構図によっては透過物が柔らかく残る、または近接した背景へ影響する場合があります。

切り替え場所：

`Depth of Field` → `透過越し背景ぼかし（実験）` → `Test4透過処理を有効にする`

チェックの変更は直ちに反映されます。次回起動にも保持する場合は、画面下部の「現在設定をINIへ保存」を押してください。

このMO2用オーバーレイにはINIを含めていません。採用済み完成版の後に置き、既存設定を維持したままDLLとシェーダーだけを上書きしてください。

## English

This package is a personal experimental build that keeps the completed stable renderer as the default while exposing the Test4 “blur scenery behind transparent subjects” path as an optional switch.

- The default is OFF. OFF uses the completed 1.0.3 rendering calculations.
- ON estimates focused hair and veils from the difference between two depth sources.
- Inside that estimated region, it preserves background blur while restoring fine and limited medium-scale subject detail.
- This is a heuristic rather than direct material classification. Some materials may remain soft or may influence nearby background detail.

Toggle location:

`Depth of Field` → `Experimental Transparency` → `Enable Test4 Transparency Path`

The checkbox applies immediately. Use “Save current settings to INI” at the bottom of the panel if the choice should persist across launches.

The MO2 overlay intentionally contains no INI. Place it after the accepted stable package so it replaces only the DLL and shaders while preserving the existing configuration.
