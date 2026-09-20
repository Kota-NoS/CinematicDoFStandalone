# CinematicDoFStandalone 1.0.1 Test 7D — Input-loaded HDR Classification

## 日本語

Test 7Cの実測では、Community ShadersのON/OFFにかかわらず、MO2プロフィールの`bUse64bitsHDRRenderTarget=1`は`kPostPostLoad`までは`off`、`kInputLoaded`以降は`on`でした。Test 7DはHDR判定の取得位置を`kPostLoad`から`kInputLoaded`へ移します。

これにより、Community Shaders使用時の0.17 m人物補正は次の条件になります。

- `HDR=1`: 統合人物保護のみ。0.17 m補正なし。
- `HDR=0`: 統合人物保護＋0.17 m補正。

シェーダー、人物保護の形状、プリセット、保存済みINIはTest 7B／7Cから変更していません。パッケージにもINIは含まれません。

### 確認手順

1. Skyrimを終了し、公式版1.0.0より後にTest 7Dだけを有効にします。Test 7A／7B／7Cは無効にしてください。
2. Community Shadersを有効にし、`bUse64bitsHDRRenderTarget=1`で通常設定（70.3 mm、f/3.4、奥2.10 m、手前0.00 m）を確認します。
3. 同じ環境で深いDoF（300.0 mm、f/1.0など）を確認します。
4. `CinematicDoFStandalone.log`を保存してください。

通常設定と深いDoFの画像があれば、補正ありのTest 7Aとの最終比較ができます。

---

## English

Test 7C measured the selected MO2 profile's `bUse64bitsHDRRenderTarget=1` as `off` through `kPostPostLoad` and `on` from `kInputLoaded` onward, identically with Community Shaders enabled and disabled. Test 7D moves the HDR classification snapshot from `kPostLoad` to `kInputLoaded`.

The 0.17 m actor assist with Community Shaders now follows these conditions:

- `HDR=1`: unified actor guard only; no 0.17 m assist.
- `HDR=0`: unified actor guard plus the 0.17 m assist.

The shader, actor-guard geometry, presets, and saved INI are unchanged from Test 7B/Test 7C. No INI is included in this package.

### Test procedure

1. Exit Skyrim and enable only Test 7D after the official 1.0.0 package. Disable Test 7A, Test 7B, and Test 7C.
2. Enable Community Shaders and check the normal settings (70.3 mm, f/3.4, far 2.10 m, near 0.00 m) with `bUse64bitsHDRRenderTarget=1`.
3. In the same environment, check a deep-DoF setup such as 300.0 mm and f/1.0.
4. Save `CinematicDoFStandalone.log`.

Images from the normal and deep-DoF checks allow the final comparison against the assisted Test 7A behaviour.
