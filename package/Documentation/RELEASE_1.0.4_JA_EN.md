# CinematicDoFStandalone 1.0.4

## 日本語

1.0.4では、HDR0／HDR1の透過素材、人物境界、空・月・枝葉の境界処理を安定化し、任意で使える「髪を馴染ませる」を追加しました。通常描画と2つの髪ブレンド方式は完全に分離され、OFFでは追加方式を実行しません。

### 主な変更点

- HDR0の手前ぼけ中間テクスチャで被覆率を保持し、人物周囲の黒い太線や点線状の膜を防止。
- ピント境界のCoC互換性と空レイヤー判定を保ちながら、人物・髪・腕などの輪郭を自然に復元。
- 「空を鮮明に保つ」で、空色の漏れを抑えつつ月、山、建造物、樹木、細い枝葉の境界を安定化。
- 「髪を馴染ませる」をメイン画面へ追加。髪の後ろの背景をぼかし、切り抜き感を抑えます。
  - モード1：背景を強く柔らかく馴染ませます。最も滑らかですが、ベールなどの透過素材も柔らかくなります。
  - モード2：透過・形・内側の細部をより保ちながら馴染ませます。強いぼかし時の空・枝葉・月境界も保護します。
- 追加方式のシェーダー準備に失敗した場合は、通常描画へ自動的にフォールバックします。
- 既存INIとプリセットは互換です。髪の馴染ませは初期状態でOFFです。

### 実ゲームで確認済み

- HDR0／HDR1の人物、武器、透過衣装、ベール、透過アクセサリ
- 通常描画、髪ブレンドのモード1／2、ON／OFF切り替え
- 空、大小の月、山、建造物、樹木、細い枝葉
- 強い遠景ぼけと「空を鮮明に保つ」の併用
- 画面位置AF、対象追従、TDMロックオン、スニークモード、会話フォーカス
- シルエット撮影、絞り形状ボケ、9プリセット

### インストール

- 新規導入：`CinematicDoFStandalone-1.0.4-Full-MO2.zip`
- 更新：`CinematicDoFStandalone-1.0.4-Update-Merge-MO2.zip`

更新版にはINIを含めません。MO2では既存MODへ`Merge`してください。`Replace`すると既存INIが削除されます。

### 補足

「髪を馴染ませる」は、半透明素材の背後を別深度として完全に再構築する機能ではありません。画面上の深度と透過信号を使って境界と背景の見え方を調整するため、素材や構図により効果量は変わります。

## English

Version 1.0.4 stabilizes transparent materials, subject edges, and sky/moon/foliage boundaries under HDR0 and HDR1, and adds the optional `Blend Hair` feature. Standard rendering and the two hair-blending modes are fully separated; when disabled, the additional paths do not execute.

### Highlights

- Preserves near-blur coverage under HDR0, preventing thick black or dotted membranes around subjects.
- Restores natural subject, hair, arm, and hand boundaries while retaining CoC compatibility and empty-layer handling.
- Revises `Keep Sky Sharp` to prevent sky-color leakage and stabilize moon, mountain, building, tree, and fine-foliage boundaries.
- Adds `Blend Hair` to the main UI. It blurs scenery behind hair to reduce cut-out-looking edges.
  - Mode 1: stronger, softer blending. It gives the smoothest transition, but transparent materials such as veils also become softer.
  - Mode 2: preserves more transparency, shape, and interior detail while protecting sky, foliage, and moon boundaries during strong blur.
- If an optional shader set cannot be prepared, that mode automatically falls back to standard rendering.
- Existing INIs and presets remain compatible. Hair blending is disabled by default.

### Validated in game

- Actors, weapons, transparent outfits, veils, and accessories under HDR0 and HDR1
- Standard rendering, both hair-blending modes, and live ON/OFF switching
- Sky, both moons, mountains, buildings, trees, and fine foliage
- Strong far blur together with `Keep Sky Sharp`
- Screen AF, target tracking, TDM lock-on, Sneak Mode, and dialogue focus
- Silhouette Photo Mode, aperture-shaped bokeh, and all nine presets

### Installation

- New installation: `CinematicDoFStandalone-1.0.4-Full-MO2.zip`
- Update: `CinematicDoFStandalone-1.0.4-Update-Merge-MO2.zip`

The update package intentionally contains no INI. In MO2, choose `Merge` into the existing mod. Do not choose `Replace`, because that removes the existing INI.

### Note

`Blend Hair` does not reconstruct the scenery behind every transparent pixel as a fully separate depth layer. It adjusts boundaries and background appearance from the available depth and transparency signals, so the visible strength depends on the material and composition.
