# CinematicDoFStandalone 1.0.3

## 日本語

1.0.3では、HDR0／HDR1の透過素材対応を安定化し、「空を鮮明に保つ」の境界処理を刷新しました。診断用の表示や切り替えは含まず、既存INIとプリセットをそのまま利用できます。

### 主な変更点

- DoFの距離判定をフル解像度のメイン深度へ統一し、Community Shadersの有無やSAO・SSR・64-bit HDR設定にかかわらず、透過衣装・ベール・透過アクセサリを同じ経路で扱います。
- HDR0の`R11G11B10_FLOAT`にはアルファがないため、手前ぼけ中間テクスチャだけを`R16G16B16A16_FLOAT`へ切り替え、被覆率を正しく保持します。人物周囲の黒い太線・点線状の膜を防ぎます。
- ピント境界をまたぐ色サンプルをCoC互換性で制限し、有効な手前ぼけ色がない画素は空のレイヤーとして扱います。
- 「空を鮮明に保つ」では、未描画の空と遠端深度の月を保護しながら、空色が粗い遠景色階層から山・建造物・樹木へ混入する経路を遮断します。
- 空と実ジオメトリが接する境界では、周囲3x3の深度から局所被覆率を求めます。空の内部と月は鮮明なまま、空を背景にした枝葉だけが不自然に鮮明になる差を和らげます。
- 広い保護帯や診断用マスクは使用せず、ぼけ半径に応じて月・山・建造物の周囲が発光状に拡張する現象を防ぎます。

### 実ゲームで確認済み

- メイン環境と最小HDR0環境の両方
- HDR0／HDR1の透過衣装、ベール、透過アクセサリ
- 空、月、山、建造物、樹木、細い枝葉の境界
- 強い遠景ぼけと「空を鮮明に保つ」の組み合わせ
- Community Shadersあり／なしのメイン深度経路

### 既知の制限

半透明素材の背後に見える背景色と手前素材は、最終フレームでは同じ画素へ合成されています。1.0.3は透過素材そのものと通常のDoF境界を安定化しますが、透過素材越しの背景だけを別の深度でぼかす処理は含みません。

## English

Version 1.0.3 stabilizes transparent-material handling under HDR0 and HDR1 and revises `Keep Sky Sharp` boundary composition. It contains no diagnostic displays or experimental toggles, and existing INIs and presets remain compatible.

### Highlights

- DoF distance classification consistently uses the full-resolution main depth, handling transparent outfits, veils, and accessories through the same path with or without Community Shaders and regardless of SAO, SSR, or 64-bit HDR settings.
- Because HDR0's `R11G11B10_FLOAT` has no alpha channel, the near-blur intermediate alone uses `R16G16B16A16_FLOAT` so coverage survives correctly. This prevents thick black or dotted membranes around subjects.
- CoC compatibility limits color sampling across focus boundaries, and a near-blur pixel without valid color samples is treated as an empty layer.
- `Keep Sky Sharp` protects unwritten sky and far-depth moons while preventing bright sky already mixed into coarse far-color levels from contaminating mountains, buildings, and foliage.
- At sky/geometry boundaries, local coverage is estimated from a 3x3 depth neighborhood. Interior sky and moons remain sharp while thin foliage no longer stays unnaturally sharp only against the sky.
- No broad protection band or diagnostic mask is retained, avoiding blur-radius-sized glow around moons, mountains, and buildings.

### Validated in game

- Main setup and minimal HDR0 setup
- Transparent outfits, veils, and accessories under HDR0 and HDR1
- Sky, moon, mountain, building, tree, and thin-foliage boundaries
- Strong far blur together with `Keep Sky Sharp`
- Main-depth operation with and without Community Shaders

### Known limitation

A transparent foreground and the background visible through it are already combined into one final-frame pixel. Version 1.0.3 stabilizes the transparent material and ordinary DoF boundaries, but does not yet blur only the background behind transparency at a separate depth.
