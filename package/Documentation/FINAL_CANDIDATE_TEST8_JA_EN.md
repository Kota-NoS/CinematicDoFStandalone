# CinematicDoFStandalone 1.0.1 Final Candidate — Test 8

## 日本語

この最終候補は、比較で好まれたTest 7Aの映像挙動を明示的な仕様として採用します。

- Community Shaders併用時はHDR設定にかかわらず、有効な人物追従中だけ手前ピント範囲の実効値を最低`0.17 m`にします。
- 保存済みINI、プリセット、MCM表示値は変更しません。
- 人物保護はTest 7Aと同じ、画面上の人物範囲と実測深度を組み合わせた共通処理です。
- 深度を無視する旧式の接写頭部保護は復活させていません。
- シェーダー、人物保護形状、通常のDoF計算はTest 7Aから変更していません。
- Test 7B～7Dで使用したHDR判定とライフサイクル診断ログは削除しました。
- パッケージにINIは含まれないため、現在の設定を維持します。

### 最終確認

1. Skyrimを終了し、公式版1.0.0より後にこの最終候補だけを有効にします。Test 7A～7Dは無効にしてください。
2. Community Shaders ON・HDR=1で、通常設定（70.3 mm、f/3.4、奥2.10 m、手前0.00 m）を確認します。
3. 同じ環境で深いDoF（300.0 mm、f/1.0）を確認します。
4. Community Shaders ON・HDR=0へ変更し、通常設定を確認します。
5. 接写と画面端で、首周りに帯や円形の保護痕がなく、顔・胴体・手が自然に保たれることを確認します。
6. `CinematicDoFStandalone.log`を保存してください。

Community Shaders OFF・HDR=1の通常設定は任意の回帰確認です。

---

## English

This final candidate makes the visually preferred Test 7A behaviour explicit.

- With Community Shaders, only a valid tracked actor receives an effective near-focus floor of `0.17 m`, regardless of the HDR setting.
- Saved INI values, presets, and displayed MCM values are unchanged.
- Actor protection remains the Test 7A path that combines the projected actor area with measured depth.
- The former close-up head override that ignored depth has not been restored.
- The shader, actor-guard geometry, and ordinary DoF calculation are unchanged from Test 7A.
- The HDR classification and lifecycle diagnostics used by Test 7B through Test 7D have been removed.
- No INI is included, so the current settings are preserved.

### Final verification

1. Exit Skyrim and enable only this final candidate after the official 1.0.0 package. Disable Test 7A through Test 7D.
2. With Community Shaders enabled and HDR=1, check the normal setup (70.3 mm, f/3.4, far 2.10 m, near 0.00 m).
3. In the same environment, check a deep-DoF setup (300.0 mm, f/1.0).
4. Switch to Community Shaders enabled and HDR=0, then check the normal setup.
5. In close portraits and near the screen edges, verify that the neck has no band- or circle-shaped protection mark and that the face, torso, and hands remain natural.
6. Save `CinematicDoFStandalone.log`.

The normal setup with Community Shaders disabled and HDR=1 is an optional regression check.
