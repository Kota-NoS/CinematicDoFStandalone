# Subject Edge Restoration for Current Base — Test1

## 日本語

現在の完成版を維持しながら、Sky / Foliage Uniform Transition Test7で見られた自然な人物境界を復元するA/Bテストです。

変更は人物境界に関係する色収集だけに限定しています。

- 遠景側の1ピクセルCoC境界補正を外しました。
- 遠景ぼけ収集時のCoC差によるサンプル排除を外しました。
- 近景ぼけ収集時の背景側サンプル排除を外しました。
- 空・月・HDR0・透過素材・ターゲット保護・各フォーカスモードは現在版のままです。
- オプションのTest6透過処理もそのまま使用できます。

狙いは、髪・肩・腕・手の輪郭を切り抜き状にせず、Test7や公開版1.0.2に近い自然なぼけへ戻すことです。

強いぼけでは人物色が背景側へ以前より混ざりやすくなるため、手や髪の外周に長い色の筋・二重線が出ないかも確認してください。

このMO2オーバーレイにはINIを含めていません。現在使用中の完成版またはOptional Test6 Transparency Test1の後に置いてください。

## English

This A/B build preserves the current completed base while restoring the softer subject boundary seen in Sky / Foliage Uniform Transition Test7.

Only subject-boundary colour gathering changes:

- Removes the one-pixel far-CoC boundary refinement.
- Removes far-gather sample rejection based on CoC difference.
- Removes near-gather rejection of background-side samples.
- Keeps the current sky, moon, HDR0, transparency, target protection, and focus-mode work.
- The optional Test6 transparency switch remains available.

The goal is to remove the cut-out appearance around hair, shoulders, arms, and hands and recover a transition closer to Test7 and public 1.0.2.

At strong blur settings, subject colour can again mix farther into the background. Please also check for long colour streaks or double lines around hands and hair.

The MO2 overlay intentionally contains no INI. Place it after the current completed build or Optional Test6 Transparency Test1.
