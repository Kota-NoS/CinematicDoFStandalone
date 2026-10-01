# CinematicDoFStandalone 1.0.3

## 日本語

1.0.3では、透過素材の境界とHDR0の手前ぼけ被覆率を安定化し、「空を鮮明に保つ」の境界処理を刷新しました。診断用の表示や切り替えは含まず、既存INIとプリセットをそのまま利用できます。

### 主な変更点

- DoFの距離判定をフル解像度のメイン深度へ統一し、Community Shadersの有無やSAO・SSR・64-bit HDR設定にかかわらず、透過衣装・ベール・透過アクセサリを同じ経路で扱います。
- HDR0の`R11G11B10_FLOAT`にはアルファがないため、手前ぼけ中間テクスチャだけを`R16G16B16A16_FLOAT`へ切り替え、被覆率を正しく保持します。人物周囲の黒い太線・点線状の膜を防ぎます。
- ピント境界をまたぐ色サンプルをCoC互換性で制限し、有効な手前ぼけ色がない画素は空のレイヤーとして扱います。
- 「空を鮮明に保つ」では、未描画の空と遠端深度の月を保護しながら、空色が粗い遠景色階層から山・建造物・樹木へ混入する経路を遮断します。
- 空と実ジオメトリが接する境界では、周囲3x3の深度から局所被覆率を求めます。空の内部と月は鮮明なまま、空を背景にした枝葉だけが不自然に鮮明になる差を和らげます。
- 広い保護帯や診断用マスクは使用せず、ぼけ半径に応じて月・山・建造物の周囲が発光状に拡張する現象を防ぎます。
- シルエット撮影モードへ、検証済みの40%・5x5対称境界スムージングを統合しました。小窓のチェックで従来のくっきりした輪郭へ戻せます。通常DoFには適用されません。

### 実ゲームで確認済み

- メイン環境と最小HDR0環境の両方
- HDR0／HDR1の透過衣装、ベール、透過アクセサリ（HDR0固有の制限は下記参照）
- 空、月、山、建造物、樹木、細い枝葉の境界
- 強い遠景ぼけと「空を鮮明に保つ」の組み合わせ
- Community Shadersあり／なしのメイン深度経路

### 既知の制限

半透明素材の背後に見える背景色と手前素材は、最終フレームでは同じ画素へ合成されています。特にHDR0（`bUse64bitsHDRRenderTarget=0`）ではメインカラー形式にアルファがないため、ベールなどの半透明素材越しの背景が十分にぼけず、鮮明に残る場合があります。1.0.3は透過素材の境界と人物周囲の膜を改善しますが、合成済みの背景だけを別の深度でぼかす処理は含みません。最良の透過表現には64-bit HDR（`bUse64bitsHDRRenderTarget=1`）を推奨します。

## English

Version 1.0.3 stabilizes transparent-material boundaries and HDR0 near-blur coverage, and revises `Keep Sky Sharp` boundary composition. It contains no diagnostic displays or experimental toggles, and existing INIs and presets remain compatible.

### Highlights

- DoF distance classification consistently uses the full-resolution main depth, handling transparent outfits, veils, and accessories through the same path with or without Community Shaders and regardless of SAO, SSR, or 64-bit HDR settings.
- Because HDR0's `R11G11B10_FLOAT` has no alpha channel, the near-blur intermediate alone uses `R16G16B16A16_FLOAT` so coverage survives correctly. This prevents thick black or dotted membranes around subjects.
- CoC compatibility limits color sampling across focus boundaries, and a near-blur pixel without valid color samples is treated as an empty layer.
- `Keep Sky Sharp` protects unwritten sky and far-depth moons while preventing bright sky already mixed into coarse far-color levels from contaminating mountains, buildings, and foliage.
- At sky/geometry boundaries, local coverage is estimated from a 3x3 depth neighborhood. Interior sky and moons remain sharp while thin foliage no longer stays unnaturally sharp only against the sky.
- No broad protection band or diagnostic mask is retained, avoiding blur-radius-sized glow around moons, mountains, and buildings.
- Silhouette Photo Mode now integrates the validated 40% symmetric 5x5 edge smoothing. A checkbox in its window restores the original crisp edge. It does not run during normal DoF.

### Validated in game

- Main setup and minimal HDR0 setup
- Transparent outfits, veils, and accessories under HDR0 and HDR1 (see the HDR0 limitation below)
- Sky, moon, mountain, building, tree, and thin-foliage boundaries
- Strong far blur together with `Keep Sky Sharp`
- Main-depth operation with and without Community Shaders

### Known limitation

A transparent foreground and the background visible through it are already combined into one final-frame pixel. In particular, HDR0 (`bUse64bitsHDRRenderTarget=0`) uses a main colour format without alpha, so scenery behind semi-transparent materials such as veils may remain sharper than the surrounding defocused background. Version 1.0.3 improves transparent boundaries and the membrane around subjects, but does not reprocess only the already-composited background at a separate depth. For the best transparency rendering, 64-bit HDR (`bUse64bitsHDRRenderTarget=1`) is recommended.
