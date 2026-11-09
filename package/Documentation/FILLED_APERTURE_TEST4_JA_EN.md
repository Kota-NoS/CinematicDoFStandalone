# Filled Aperture Test 4 / 絞り内部補完 Test 4

## 日本語

Test 3の外径と内部の均一さを維持しながら、`最高`の形状ボケ境界を少し読み取りやすくする比較版です。

- `高`はTest 3から変更していません。
- `最高`では、外周約20％を滑らかな帯として抽出します。
- 内部全体の明点平均へ、外周帯の明点平均を22％だけ混ぜます。
- 新しい発光や硬い輪郭線は加えず、サンプル済みの明るさの範囲内で合成します。
- 中心点の抑制と外周サイズの維持はTest 3と同じです。
- OFF時の従来処理には影響しません。

確認項目：

1. `最高`の輪郭が`高`より少し読み取りやすいか。
2. 中央まで埋まった状態を維持し、輪や空洞へ戻っていないか。
3. 明るいボケだけが不自然に縁取られていないか。
4. 手前ボケと奥ボケの両方で効果が自然か。

## English

This comparison build keeps the Test 3 size and filled interior while making the `Highest` aperture boundary slightly easier to read.

- `High` is unchanged from Test 3.
- `Highest` extracts the outer approximately 20 percent as a soft band.
- It mixes 22 percent of that rim highlight mean into the full-aperture highlight mean.
- It adds neither new light nor a hard outline; blending remains within the sampled highlight range.
- Test 3's centre-dot suppression and full outer size are retained.
- The legacy path remains unchanged when the option is off.

Please compare:

1. Whether the `Highest` outline is slightly clearer than `High`.
2. Whether the interior remains filled instead of returning to a ring or hollow shape.
3. Whether bright bokeh avoids an unnatural hard border.
4. Both foreground and background bokeh.
