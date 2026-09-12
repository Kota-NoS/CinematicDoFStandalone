# Dialogue-Only Mode Test / 会話限定DoFテスト

## 日本語

この小規模テスト版は、実ゲーム確認済みのFar Density Mip Test 2をそのまま基礎にしています。描画シェーダー、Mip選択、通常ぼかし、絞り形状、Highlight Boost、FSR有効描画領域、ピント処理、9プリセットの値は変更していません。

### 会話中だけDoFを使う

1. `DoFを有効にする`をONにします。
2. `会話外でもDoFを使用`をOFFにします。
3. `会話中の被写体にピントを合わせる`をONにします。

主スイッチまたはホットキーでDoFをOFFにすると、会話中も含めて完全に停止します。`会話外でもDoFを使用`はプリセットに含まれないため、プリセットを適用・保存・初期化しても変化しません。

### 更新互換性

- 旧INIに`[Modes]`がない場合は`NormalGameplayEnabled=true`として読み込み、従来どおり通常時にもDoFを使用します。
- 保存済みの`[Interface] Japanese=true`が残っている場合は日本語を維持します。
- 新規導入用INIの初期言語は英語です。更新時にユーザーINIを上書きする配布方法では保存済み言語を判定できないため、MOD管理ツール側でも既存INIを維持してください。

### 確認項目

- 通常時OFFから会話開始でDoFが有効になり、会話終了で完全に戻ること
- 会話中に主スイッチ／ホットキーをOFFにすると即時停止すること
- `会話外でもDoFを使用`をONへ戻すと通常DoFが復帰すること
- 9プリセットの適用・保存・初期化で会話外DoF設定が変わらないこと
- `[English]`／`[日本語]`の選択状態が常に分かり、即時に切り替わって保存・再読込できること

## English

This small test build is based directly on the in-game-validated Far Density Mip Test 2. It does not change the rendering shader, Mip selection, ordinary blur, aperture shaping, Highlight Boost, FSR active-area handling, focus calculations, or any of the nine preset values.

### Dialogue-only DoF

1. Enable `Enable DoF`.
2. Disable `Use DoF outside dialogue`.
3. Keep dialogue focus enabled.

The master switch and its hotkey still stop every DoF path, including dialogue. `Use DoF outside dialogue` is deliberately excluded from presets, so applying, storing, or restoring a preset cannot change it.

### Update compatibility

- An older INI without `[Modes]` loads `NormalGameplayEnabled=true`, preserving the previous normal-gameplay behavior.
- An existing `[Interface] Japanese=true` value remains Japanese when that INI is retained.
- Fresh installations default to English. A package installer cannot recover a saved language after physically overwriting the user's INI, so updates should preserve the existing INI.

### Test checklist

- With normal gameplay disabled, starting dialogue enables DoF and ending dialogue restores a completely clear normal view.
- The master switch or hotkey immediately stops DoF even during dialogue.
- Re-enabling `Use DoF outside dialogue` restores normal DoF.
- Applying, storing, or restoring any of the nine presets does not change the dialogue-only mode.
- Both language buttons are always visible; language changes immediately and survives save/reload.
