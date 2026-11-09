# CinematicDoFStandalone 1.0.5

## 日本語

1.0.5では、画面位置AFを一人称／三人称で個別設定できるようにし、プレイヤー自動追従の一人称時の扱いを拡張しました。また、Community Shaders併用時の隠れた手前ピント範囲制限を廃止しました。

### 主な変更点

- Community Shaders併用時に人物追従へ自動適用していた手前ピント範囲`0.17 m`の下限を廃止。`0.00 m`を含む保存値をそのまま使用します。
- 画面位置AFのX／Y座標を一人称と三人称で個別保存します。
- 視点に合わせて、UI表示を「一人称画面AF」／「三人称画面AF」へ自動で切り替えます。
- プレイヤー自動追従が一人称で停止している間、任意で一人称画面AFへ切り替える「一人称時は一人称画面AFを使用」を追加しました。
- 三人称へ戻るとプレイヤー自動追従を再開します。コンソール選択対象の追従には影響しません。
- 「一人称の手前ぼかし」を関連する一人称設定の直下へ移動しました。
- 既存INIとプリセットは互換です。新しい設定は安全な初期値で読み込まれます。

### インストール

- 新規導入：`CinematicDoFStandalone-1.0.5-Full-MO2.zip`
- 更新：`CinematicDoFStandalone-1.0.5-Update-Merge-MO2.zip`

更新版にはINIを含めません。MO2では既存MODへ`Merge`してください。`Replace`すると既存INIが削除されます。

## English

Version 1.0.5 adds separate Screen AF positions for first- and third-person views, expands first-person behavior for automatic player tracking, and removes the hidden near-focus-range restriction used with Community Shaders.

### Highlights

- Removes the automatic `0.17 m` near-focus-range floor formerly applied to tracked actors with Community Shaders. Saved values, including `0.00 m`, are now used unchanged.
- Stores Screen AF X/Y coordinates independently for first- and third-person views.
- Changes the UI label automatically between `1st-Person AF` and `3rd-Person AF` according to the current view.
- Adds an optional `Use 1st-Person AF` setting while automatic player tracking is paused in first person.
- Returning to third person resumes automatic player tracking. Console-selected target tracking is unaffected.
- Moves `First-Person Near Blur` directly below the related first-person setting.
- Existing INIs and presets remain compatible. New settings load with safe defaults.

### Installation

- New installation: `CinematicDoFStandalone-1.0.5-Full-MO2.zip`
- Update: `CinematicDoFStandalone-1.0.5-Update-Merge-MO2.zip`

The update package intentionally contains no INI. In MO2, choose `Merge` into the existing mod. Do not choose `Replace`, because that removes the existing INI.
