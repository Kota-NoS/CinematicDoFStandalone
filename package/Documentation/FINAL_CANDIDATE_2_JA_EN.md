# CinematicDoFStandalone 1.0.1 Final Candidate 2

## 日本語

この候補版は、前回のFinal Candidateへ実ゲーム確認済みの空・水面対応を統合したものです。

- 空の内部をDoFから除外し、地形・樹木・建築との境界は段階的にぼかして馴染ませます。
- Community Shadersの低設定用深度を通常のDoFへ使う環境でも、空判定だけはメイン深度から行います。これにより水面の境界を維持します。
- 「空をぼかさない」を「詳細設定を表示」の右側へ追加しました。画面の縦幅は増えません。
- 初期値はONです。この値は各プリセットと次回起動設定へ保存されます。
- 既存INI／既存プリセットに`KeepSkySharp`が無い場合はONとして読み込みます。OFFで保存したプリセットは、以後OFFを保持します。
- 月など深度を書き込む天体は空マスクに含まれないため、DoFでぼける場合があります。今回は仕様とします。
- 前回Final Candidateの人物追従・会話フォーカス・人物保護はそのまま含まれます。

### 確認項目

1. 「空をぼかさない」が「詳細設定を表示」の右側にあり、初期状態でONになっていること。
2. 既存プリセットを適用するとONになること。
3. OFFにしてプリセットへ「現在値を保存」し、別プリセットを経由して戻した場合にOFFが復元されること。
4. Community Shaders ON/OFFの両方で、水面と空・地形の境界が自然であること。
5. ON/OFFを切り替えた際、人物・通常の遠景DoF・会話フォーカスに変化がないこと。

---

## English

This candidate integrates the in-game-validated sky and water handling into the previous Final Candidate.

- Sky interiors are excluded from DoF, while boundaries against terrain, trees, and buildings are progressively feathered.
- When a Community Shaders low-settings depth source is used for ordinary DoF, sky classification still reads the main depth. This preserves the water boundary.
- `Keep Sky Sharp` is placed to the right of `Show Advanced Settings`, without adding vertical UI height.
- It defaults to on and is saved in each preset and in the startup settings.
- Existing INIs and presets without `KeepSkySharp` load it as enabled. A preset explicitly stored while it is off keeps the off value.
- Moons and other celestial objects that write depth are not part of the sky mask and can still be blurred. This remains expected behavior for this candidate.
- The tracked-actor, dialogue-focus, and subject-protection behavior from the previous Final Candidate is retained.

### Verification

1. Confirm that `Keep Sky Sharp` is beside `Show Advanced Settings` and defaults to on.
2. Apply an existing preset and confirm that the option is on.
3. Turn it off, use `Store Current`, switch to another preset, return, and confirm that off is restored.
4. With Community Shaders both enabled and disabled, verify natural water, sky, and geometry boundaries.
5. Toggle it on and off and confirm that actor rendering, ordinary distant DoF, and dialogue focus do not otherwise change.
