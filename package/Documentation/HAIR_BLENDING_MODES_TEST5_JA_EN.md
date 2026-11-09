# Hair Blending Modes Test 5

## 日本語

完成済みの通常描画を保ったまま、髪やベール越しの背景ぼかしを選べる検証用オーバーレイです。

- 「空を鮮明に保つ」と「髪を馴染ませる」を、会話外DoF・言語選択の一段下へ配置しました。
- 「髪を馴染ませる」をOFFにすると、完成済みの通常シェーダーだけを使用します。
- ONでは右側の「1」「2」で方式を選択します。
- モード1はTest6の境界ぼかしを基礎に、最終の細部復元を意図的に行わない柔らか処理です。Test2統合版で偶然得られた自然な全体ぼかしを、未接続テクスチャに依存せず再現します。
- モード2は元のSkyFoliage-HybridTransparency-Test8です。
- Test 5では、モード2の最終復元パスへ正しい焦点・二重深度入力を渡したまま、モード1だけ細部復元を明示的に省きます。
- モード2では、復元対象の月を周辺の空境界判定でも保護された空として扱い、Test8の均一遷移を維持しながら月外周の固定幅リングを抑えます。
- 通常版、柔らかモード、Test8は別々のシェーダー集合として読み込み、1フレームでは選択中の1集合だけを実行します。
- 追加の中間テクスチャや追加描画はありません。増えるのは起動時のシェーダー準備時間、シェーダーオブジェクト用メモリ、パッケージ容量だけです。
- 柔らかモードまたはTest8の準備に失敗した場合、その方式は通常描画へ自動的にフォールバックします。
- ポップアップ説明は、主な見た目の効果である「髪の後ろをぼかして背景になじませる」ことと、ベールなどの透過素材が少しぼやける点を案内します。

更新用ZIPには既存設定を保護するためINIを含めていません。

## English

This test overlay keeps the completed stable renderer intact while offering optional background blur behind hair and veils.

- “Keep Sky Sharp” and “Blend Hair” now appear on a prominent row directly below the outside-dialogue and language controls.
- With “Blend Hair” OFF, only the completed stable shader set is used.
- When enabled, buttons “1” and “2” select the treatment.
- Mode 1 is a soft treatment based on the Test6 boundary blur with final fine/medium detail recovery intentionally omitted. It deliberately reproduces the attractive broad softness that occurred accidentally in the combined Test2 build, without relying on unbound textures.
- Mode 2 is the original SkyFoliage-HybridTransparency-Test8 shader.
- Test 5 keeps the correct focus and dual-depth inputs for Mode 2 while explicitly omitting final detail recovery only in Mode 1.
- Mode 2 now treats restored moon pixels as protected sky during the surrounding boundary test, suppressing the fixed-width moon ring while retaining Test8's uniform sky/foliage transition.
- Stable, soft, and Test8 treatments are loaded as separate shader sets; only the selected set executes in a frame.
- No extra intermediate textures or extra rendering passes are added. The trade-off is limited to startup shader preparation time, shader-object memory, and package size.
- If either optional set cannot be prepared, that unavailable mode automatically falls back to stable rendering.
- The tooltip describes the main visual benefit—softening the scenery behind hair so the edge blends naturally—and notes that transparent materials such as veils may become slightly softer.

The update ZIP intentionally contains no INI so existing settings are preserved.
