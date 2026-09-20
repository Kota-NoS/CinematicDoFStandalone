# CinematicDoFStandalone 1.0.1 Test 7C — HDR Lifecycle Diagnostic

## 日本語

これはTest 7BのHDR判定位置を調査するための診断版です。映像処理、統合人物保護、シェーダー、0.17 m補正条件はTest 7Bから変更していません。INIも含まれないため、保存済み設定は維持されます。

Test 7Bでは、選択中のMO2プロフィールが`bUse64bitsHDRRenderTarget=1`でも、`kPostLoad`時点では`off`、初回描画時には`on`と記録されました。Test 7Cは次の各段階でSAO、SSR、64-bit HDR、Community Shaders DLLの状態をログへ記録します。

- `SKSEPlugin_Load`
- `kPostLoad`
- `kPostPostLoad`
- `kInputLoaded`
- `kDataLoaded`
- 初回描画（既存ログ）

### 確認手順

1. Skyrimを終了してから、公式版1.0.0より後にTest 7Cだけを有効にします。Test 7A／7Bなど過去のTest版は無効にしてください。
2. MO2プロフィールの`bUse64bitsHDRRenderTarget=1`を維持します。
3. Community Shadersを有効にしてSkyrimを完全起動し、人物追従でDoFが一度描画されるところまで進めて終了します。
4. `CinematicDoFStandalone.log`を`Test7C-CS-ON.log`など別名で保存します。
5. Test 7Cと同じ公式版を有効にしたまま、Community Shadersだけを無効にして同じ手順を繰り返します。
6. 2回目のログを`Test7C-CS-OFF.log`など別名で保存します。

この診断では画質比較やF値の比較は不要です。ログ中の`HDR lifecycle checkpoint`行と`Display depth settings at first frame`行が判断材料です。

---

## English

This is a diagnostic build for locating the correct HDR classification point after Test 7B. Rendering, the unified actor guard, the shader, and the 0.17 m assist condition are unchanged from Test 7B. No INI is included, so saved settings are preserved.

Test 7B recorded 64-bit HDR as `off` at `kPostLoad` but `on` at the first render frame even though the selected MO2 profile explicitly used `bUse64bitsHDRRenderTarget=1`. Test 7C records SAO, SSR, 64-bit HDR, and the Community Shaders DLL state at these stages:

- `SKSEPlugin_Load`
- `kPostLoad`
- `kPostPostLoad`
- `kInputLoaded`
- `kDataLoaded`
- first render frame (existing log)

### Test procedure

1. Exit Skyrim, then enable only Test 7C after the official 1.0.0 package. Disable Test 7A, Test 7B, and every earlier test build.
2. Keep `bUse64bitsHDRRenderTarget=1` in the selected MO2 profile.
3. Enable Community Shaders, fully launch Skyrim, reach one tracked-actor DoF frame, and exit.
4. Save `CinematicDoFStandalone.log` under a distinct name such as `Test7C-CS-ON.log`.
5. Keep Test 7C and the same official package enabled, disable only Community Shaders, and repeat the launch.
6. Save the second log under a distinct name such as `Test7C-CS-OFF.log`.

No visual or F-number comparison is needed for this diagnostic. The relevant evidence is in the `HDR lifecycle checkpoint` lines and the existing `Display depth settings at first frame` line.
