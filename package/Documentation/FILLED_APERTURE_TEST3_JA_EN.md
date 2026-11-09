# Filled Aperture Test 3 / 絞り内部補完 Test 3

## 日本語

Test 2で確認された「形状ボケ全体がやや小さくなる」「中心に点ができる」現象を修正する比較版です。

- `高` は前回と同じです。
- `最高` の各リングを本来の半径へ戻し、形状ボケの外周サイズを維持します。
- `最高` の追加リングは角度だけをずらし、同じ方向へサンプルが並ぶことによる隙間を減らします。
- `最高` では中央サンプルのハイライト強調を無効化し、中心点を抑えます。
- OFF時の従来処理には影響しません。

確認項目：

1. `高` と `最高` で形状ボケの外径が不自然に変わらないか。
2. `最高` でTest 2の中心点が消えているか。
3. `最高` の内部が`高`より均一に見えるか。
4. 遠景ボケと手前ボケの両方で破綻がないか。

## English

This comparison build addresses the two Test 2 regressions: slightly smaller shaped bokeh and a visible centre dot.

- `High` is unchanged.
- `Highest` restores every ring to its original radius, preserving the full aperture outline and size.
- The additional `Highest` rings are angle-shifted only, reducing aligned gaps without pulling samples inward.
- The emphasized centre highlight sample is disabled in `Highest` to suppress the centre dot.
- The legacy path remains unchanged when the option is off.

Please compare:

1. Whether `High` and `Highest` retain a consistent outer bokeh size.
2. Whether the Test 2 centre dot is gone in `Highest`.
3. Whether the `Highest` interior looks more even than `High`.
4. Both far-background and foreground bokeh for artifacts.
